/*
 * Legacy Backend for libhardware_legacy.so (Pre-Treble)
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "vibrator_backend.h"

typedef int (*vibrator_exists_t)(void);
typedef int (*vibrator_on_t)(int timeout_ms);
typedef int (*vibrator_off_t)(int timeout_ms);

static vibrator_exists_t g_vibrator_exists = NULL;
static vibrator_on_t g_vibrator_on = NULL;
static vibrator_off_t g_vibrator_off = NULL;
static void *g_vibrator_handle = NULL;

static int legacy_exists(void)
{
    if (!g_vibrator_exists) return 0;
    return g_vibrator_exists();
}

static int legacy_on(int timeout_ms)
{
    if (!g_vibrator_on) return -1;
    return g_vibrator_on(timeout_ms);
}

static int legacy_off(void)
{
    if (!g_vibrator_off) return -1;
    return g_vibrator_off(0);
}

static void legacy_close(void)
{
    if (g_vibrator_handle) {
        dlclose(g_vibrator_handle);
        g_vibrator_handle = NULL;
    }
    g_vibrator_exists = NULL;
    g_vibrator_on = NULL;
    g_vibrator_off = NULL;
}

static struct vibrator_backend legacy_backend = {
    .exists = legacy_exists,
    .on = legacy_on,
    .off = legacy_off,
    .close = legacy_close,
    .name = "legacy"
};

extern int vibrator_legacy_init(struct vibrator_backend **out)
{
    const char *lib_path = "libhardware_legacy.so";

    g_vibrator_handle = dlopen(lib_path, RTLD_LAZY);
    if (!g_vibrator_handle) {
        fprintf(stderr, "[vibrator_legacy] dlopen(%s) failed: %s\n",
                lib_path, dlerror());
        return -1;
    }

    /* Clear any previous errors */
    dlerror();

    g_vibrator_exists = (vibrator_exists_t) dlsym(
        g_vibrator_handle, "vibrator_exists"
    );
    if (!g_vibrator_exists) {
        fprintf(stderr, "[vibrator_legacy] dlsym(vibrator_exists) failed: %s\n",
                dlerror());
        dlclose(g_vibrator_handle);
        g_vibrator_handle = NULL;
        return -1;
    }

    g_vibrator_on = (vibrator_on_t) dlsym(
        g_vibrator_handle, "vibrator_on"
    );
    if (!g_vibrator_on) {
        fprintf(stderr, "[vibrator_legacy] dlsym(vibrator_on) failed: %s\n",
                dlerror());
        dlclose(g_vibrator_handle);
        g_vibrator_handle = NULL;
        return -1;
    }

    g_vibrator_off = (vibrator_off_t) dlsym(
        g_vibrator_handle, "vibrator_off"
    );
    if (!g_vibrator_off) {
        fprintf(stderr, "[vibrator_legacy] dlsym(vibrator_off) failed: %s\n",
                dlerror());
        dlclose(g_vibrator_handle);
        g_vibrator_handle = NULL;
        return -1;
    }

    fprintf(stderr, "[vibrator_legacy] Loaded %s successfully\n", lib_path);
    *out = &legacy_backend;
    return 0;
}
