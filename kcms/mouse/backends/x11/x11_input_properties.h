#pragma once

#include <config-X11.h>

#if HAVE_XORGLIBINPUT
#include <libinput-properties.h>
#else
#define LIBINPUT_PROP_ACCEL "libinput Accel Speed"
#define LIBINPUT_PROP_ACCEL_PROFILE_ENABLED "libinput Accel Profile Enabled"
#define LIBINPUT_PROP_NATURAL_SCROLL "libinput Natural Scrolling Enabled"
#define LIBINPUT_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define LIBINPUT_PROP_SCROLL_METHOD_ENABLED "libinput Scroll Method Enabled"
#define LIBINPUT_PROP_MIDDLE_EMULATION_ENABLED "libinput Middle Emulation Enabled"
#endif
