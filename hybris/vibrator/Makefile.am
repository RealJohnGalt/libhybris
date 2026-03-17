lib_LTLIBRARIES = \
	libvibrator.la

libvibrator_la_SOURCES = \
	vibrator.c \
	vibrator_legacy.c \
	vibrator_sysfs.c

libvibrator_la_SOURCES += \
	vibrator_aidl.cpp \
	vibrator_hidl.cpp

libvibrator_la_CFLAGS = \
	-I$(top_srcdir)/include \
	$(ANDROID_HEADERS_CFLAGS) \
	-Wall -Wextra -Werror \
	-fvisibility=hidden

libvibrator_la_CXXFLAGS = \
	-I$(top_srcdir)/include \
	$(ANDROID_HEADERS_CFLAGS) \
	-std=c++17 \
	-Wall -Wextra -Werror \
	-fvisibility=hidden

if WANT_TRACE
libvibrator_la_CFLAGS += -DDEBUG
libvibrator_la_CXXFLAGS += -DDEBUG
endif

if WANT_DEBUG
libvibrator_la_CFLAGS += -ggdb -O0
libvibrator_la_CXXFLAGS += -ggdb -O0
endif

libvibrator_la_LDFLAGS = \
	$(top_builddir)/common/libhybris-common.la \
	-version-info "1":"0":"0"

libvibrator_la_LDFLAGS += \
	-lbinder_ndk \
	-landroid.hardware.vibrator@1.0

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvibrator.pc

include_HEADERS = $(top_srcdir)/include/hybris/vibrator.h
