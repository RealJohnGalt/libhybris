/*
 * Internal backend interface header
 */

#ifndef __VIBRATOR_BACKEND_H
#define __VIBRATOR_BACKEND_H

#include <stdint.h>

struct vibrator_backend {
    int (*exists)(void);
    int (*on)(int timeout_ms);
    int (*off)(void);
    void (*close)(void);
    const char *name;
};

/* Backend initializers (return 0 on success, non-zero on not available) */
int vibrator_aidl_init(struct vibrator_backend **out);
int vibrator_hidl_init(struct vibrator_backend **out);
int vibrator_legacy_init(struct vibrator_backend **out);
int vibrator_sysfs_init(struct vibrator_backend **out);

#endif
