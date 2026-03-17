/*
 * Sysfs/Evdev Backend for kernel force-feedback or LED vibrator
 * Last resort when all HALs are unavailable
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include "vibrator_backend.h"

static int g_ff_device_fd = -1;
static int g_led_enable_fd = -1;
static pthread_mutex_t g_sysfs_lock = PTHREAD_MUTEX_INITIALIZER;

/* Try to detect and open vibrator via force-feedback (evdev) */
static int open_ff_device(void)
{
    const char *device_paths[] = {
        "/dev/input/event0",
        "/dev/input/event1",
        "/dev/input/event2",
        "/dev/input/event3",
        "/dev/input/event4",
        NULL
    };

    for (int i = 0; device_paths[i]; i++) {
        int fd = open(device_paths[i], O_WRONLY);
        if (fd < 0) continue;

        /* Test if device supports force feedback */
        unsigned long features[4] = {0};
        if (ioctl(fd, EVIOCGBIT(EV_FF, sizeof(features)), features) < 0) {
            close(fd);
            continue;
        }

        /* Check for FF_RUMBLE support */
        if (features[FF_RUMBLE / 32] & (1 << (FF_RUMBLE % 32))) {
            fprintf(stderr, "[vibrator_sysfs] Found FF device at %s\n", device_paths[i]);
            return fd;
        }

        close(fd);
    }

    return -1;
}

/* Try to detect vibrator LED (common on some platforms) */
static int open_led_device(void)
{
    const char *led_paths[] = {
        "/sys/class/leds/vibrator/brightness",
        "/sys/class/led/vibrator/brightness",
        "/sys/devices/virtual/leds/vibrator/brightness",
        NULL
    };

    for (int i = 0; led_paths[i]; i++) {
        int fd = open(led_paths[i], O_WRONLY);
        if (fd >= 0) {
            fprintf(stderr, "[vibrator_sysfs] Found LED vibrator at %s\n", led_paths[i]);
            return fd;
        }
    }

    return -1;
}

static int sysfs_exists(void)
{
    return (g_ff_device_fd >= 0 || g_led_enable_fd >= 0) ? 1 : 0;
}

static int sysfs_on(int timeout_ms)
{
    if (timeout_ms < 0) return -1;

    pthread_mutex_lock(&g_sysfs_lock);

    if (g_ff_device_fd >= 0) {
        /* Use force feedback */
        struct ff_effect effect = {
            .type = FF_RUMBLE,
            .id = -1,
            .u.rumble = {
                .strong_magnitude = 0xffff,
                .weak_magnitude = 0x0000
            }
        };

        if (ioctl(g_ff_device_fd, EVIOCSFF, &effect) < 0) {
            pthread_mutex_unlock(&g_sysfs_lock);
            return -1;
        }

        struct input_event event = {
            .type = EV_FF,
            .code = effect.id,
            .value = 1
        };

        if (write(g_ff_device_fd, &event, sizeof(event)) != sizeof(event)) {
            pthread_mutex_unlock(&g_sysfs_lock);
            return -1;
        }

        /* Let hardware handle the timeout; this is a crude approximation */
        pthread_mutex_unlock(&g_sysfs_lock);
        return 0;
    } else if (g_led_enable_fd >= 0) {
        /* Use simple LED on/off */
        if (write(g_led_enable_fd, "255", 3) != 3) {
            pthread_mutex_unlock(&g_sysfs_lock);
            return -1;
        }
        pthread_mutex_unlock(&g_sysfs_lock);
        return 0;
    }

    pthread_mutex_unlock(&g_sysfs_lock);
    return -1;
}

static int sysfs_off(void)
{
    ssize_t ret;

    pthread_mutex_lock(&g_sysfs_lock);

    if (g_ff_device_fd >= 0) {
        struct input_event event = {
            .type = EV_FF,
            .code = 0,
            .value = 0
        };

        ret = write(g_ff_device_fd, &event, sizeof(event));
        if (ret != (ssize_t)sizeof(event)) {
            pthread_mutex_unlock(&g_sysfs_lock);
            return -1;
        }
    } else if (g_led_enable_fd >= 0) {
        ret = write(g_led_enable_fd, "0", 1);
        if (ret != 1) {
            pthread_mutex_unlock(&g_sysfs_lock);
            return -1;
        }
    }

    pthread_mutex_unlock(&g_sysfs_lock);
    return 0;
}

static void sysfs_close(void)
{
    pthread_mutex_lock(&g_sysfs_lock);
    if (g_ff_device_fd >= 0) {
        close(g_ff_device_fd);
        g_ff_device_fd = -1;
    }
    if (g_led_enable_fd >= 0) {
        close(g_led_enable_fd);
        g_led_enable_fd = -1;
    }
    pthread_mutex_unlock(&g_sysfs_lock);
}

static struct vibrator_backend sysfs_backend = {
    .exists = sysfs_exists,
    .on = sysfs_on,
    .off = sysfs_off,
    .close = sysfs_close,
    .name = "sysfs"
};

extern int vibrator_sysfs_init(struct vibrator_backend **out)
{
    g_ff_device_fd = open_ff_device();
    if (g_ff_device_fd >= 0) {
        *out = &sysfs_backend;
        return 0;
    }

    g_led_enable_fd = open_led_device();
    if (g_led_enable_fd >= 0) {
        *out = &sysfs_backend;
        return 0;
    }

    fprintf(stderr, "[vibrator_sysfs] No sysfs/evdev vibrator found\n");
    return -1;
}
