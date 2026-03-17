/*
 * AIDL Backend for android.hardware.vibrator.IVibrator (Android 11+)
 */

#include <android/binder_manager.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory>
#include <aidl/android/hardware/vibrator/IVibrator.h>

#include "vibrator_backend.h"

using aidl::android::hardware::vibrator::IVibrator;

static std::shared_ptr<IVibrator> g_vibrator_service = nullptr;

static int aidl_exists(void)
{
    return g_vibrator_service != nullptr ? 1 : 0;
}

static int aidl_on(int timeout_ms)
{
    if (!g_vibrator_service) return -1;
    if (timeout_ms < 0) return -1;

    try {
        // Simple on pattern: single effect with given duration
        auto status = g_vibrator_service->on(
            static_cast<int32_t>(timeout_ms),
            nullptr /* callback */
        );
        return status.isOk() ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

static int aidl_off(void)
{
    if (!g_vibrator_service) return -1;

    try {
        auto status = g_vibrator_service->off();
        return status.isOk() ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

static void aidl_close(void)
{
    g_vibrator_service = nullptr;
}

static struct vibrator_backend aidl_backend = {
    .exists = aidl_exists,
    .on = aidl_on,
    .off = aidl_off,
    .close = aidl_close,
    .name = "aidl"
};

extern "C" int vibrator_aidl_init(struct vibrator_backend **out)
{
    const char *instance_name = "android.hardware.vibrator.IVibrator/default";

    try {
        ndk::SpAIBinder binder(
            AServiceManager_waitForService(instance_name)
        );

        if (!binder.get()) {
            fprintf(stderr, "[vibrator_aidl] Service %s not found\n", instance_name);
            return -1;
        }

        g_vibrator_service = IVibrator::fromBinder(binder);
        if (!g_vibrator_service) {
            fprintf(stderr, "[vibrator_aidl] Failed to bind IVibrator service\n");
            return -1;
        }

        fprintf(stderr, "[vibrator_aidl] Connected to %s\n", instance_name);
        *out = &aidl_backend;
        return 0;
    } catch (const std::exception &e) {
        fprintf(stderr, "[vibrator_aidl] Exception: %s\n", e.what());
        return -1;
    } catch (...) {
        fprintf(stderr, "[vibrator_aidl] Unknown exception\n");
        return -1;
    }
}
