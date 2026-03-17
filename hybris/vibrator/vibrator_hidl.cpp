/*
 * HIDL Backend for android.hardware.vibrator@1.0 (Android 8-12)
 */

#include <android/hardware/vibrator/1.0/IVibrator.h>
#include <hidl/ServiceManagement.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory>

#include "vibrator_backend.h"

using android::hardware::vibrator::V1_0::IVibrator;
using android::hardware::Return;
using android::sp;

static sp<IVibrator> g_vibrator_service = nullptr;

static int hidl_exists(void)
{
    return g_vibrator_service != nullptr ? 1 : 0;
}

static int hidl_on(int timeout_ms)
{
    if (!g_vibrator_service) return -1;
    if (timeout_ms < 0) return -1;

    try {
        Return<void> ret = g_vibrator_service->on(
            static_cast<uint32_t>(timeout_ms)
        );
        return ret.isOk() ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

static int hidl_off(void)
{
    if (!g_vibrator_service) return -1;

    try {
        Return<void> ret = g_vibrator_service->off();
        return ret.isOk() ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

static void hidl_close(void)
{
    g_vibrator_service = nullptr;
}

static struct vibrator_backend hidl_backend = {
    .exists = hidl_exists,
    .on = hidl_on,
    .off = hidl_off,
    .close = hidl_close,
    .name = "hidl"
};

extern "C" int vibrator_hidl_init(struct vibrator_backend **out)
{
    try {
        /* getService returns nullptr if not available */
        g_vibrator_service = IVibrator::getService();

        if (g_vibrator_service == nullptr) {
            fprintf(stderr, "[vibrator_hidl] IVibrator service not found\n");
            return -1;
        }

        fprintf(stderr, "[vibrator_hidl] Connected to IVibrator@1.0\n");
        *out = &hidl_backend;
        return 0;
    } catch (const std::exception &e) {
        fprintf(stderr, "[vibrator_hidl] Exception: %s\n", e.what());
        return -1;
    } catch (...) {
        fprintf(stderr, "[vibrator_hidl] Unknown exception\n");
        return -1;
    }
}
