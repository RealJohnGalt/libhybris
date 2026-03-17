/*
 * Copyright (C) 2014 Jolla Ltd.
 * Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>
 * Copyright (C) 2026 Lindroid Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vibrator_backend.h"

static struct vibrator_backend *active_backend = NULL;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

/* Null backend (no-op fallback) */
static int null_exists(void) { return 0; }
static int null_on(int ms) { (void)ms; return 0; }
static int null_off(void) { return 0; }
static void null_close(void) { }

static struct vibrator_backend null_backend = {
    .exists = null_exists,
    .on = null_on,
    .off = null_off,
    .close = null_close,
    .name = "null"
};

/**
 * Probes available backends in priority order.
 * First match wins and is cached for entire process lifetime.
 */
static void vibrator_init_backend(void)
{
    /* Try AIDL first (Android 11+) */
    if (vibrator_aidl_init(&active_backend) == 0) {
        fprintf(stderr, "[libhybris-vibrator] Using AIDL backend\n");
        return;
    }

    /* Try HIDL second (Android 8-12) */
    if (vibrator_hidl_init(&active_backend) == 0) {
        fprintf(stderr, "[libhybris-vibrator] Using HIDL backend\n");
        return;
    }

    /* Try legacy libhardware_legacy.so (pre-Treble) */
    if (vibrator_legacy_init(&active_backend) == 0) {
        fprintf(stderr, "[libhybris-vibrator] Using legacy libhardware_legacy backend\n");
        return;
    }

    /* Try kernel sysfs/evdev (last resort) */
    if (vibrator_sysfs_init(&active_backend) == 0) {
        fprintf(stderr, "[libhybris-vibrator] Using sysfs/evdev backend\n");
        return;
    }

    /* Fallback to null backend */
    fprintf(stderr, "[libhybris-vibrator] No vibrator backend available, using null\n");
    active_backend = &null_backend;
}

int vibrator_exists(void)
{
    pthread_once(&init_once, vibrator_init_backend);
    if (!active_backend) return 0;
    return active_backend->exists();
}

int vibrator_on(int timeout_ms)
{
    pthread_once(&init_once, vibrator_init_backend);
    if (!active_backend) return -1;
    return active_backend->on(timeout_ms);
}

int vibrator_off(int timeout_ms)
{
    (void)timeout_ms; /* Some backends ignore timeout on off */
    pthread_once(&init_once, vibrator_init_backend);
    if (!active_backend) return -1;
    return active_backend->off();
}
