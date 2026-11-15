#pragma once

// Pure trigger-axis normalization and hysteresis helpers shared by common/api.c
// (SDL2 [-32768..32767] trigger axes) and screenshot.c (raw evdev [0..maximum]
// ABS_Z / ABS_RZ axes). No SDL or OS headers; host-tested by
// common/tests/test_trigger_axis.c.

#include <stdint.h>

#define TRIGGER_NORM_MAX 32767
#define TRIGGER_RAW_DEADZONE 1024 // ~3.1% of [0..32767] resting Hall sensor floor
#define TRIGGER_PRESS_NORM 6553	  // ~20% normalized travel to actuate digital L2/R2
#define TRIGGER_RELEASE_NORM 4587 // ~14% normalized travel to release (hysteresis)

// Maps an SDL joystick trigger axis value in [-32768..32767] (where -32768 is
// unpressed and +32767 is fully depressed) onto [0..32767] with a small resting
// deadzone so Hall-effect sensor noise sits cleanly at 0.
static inline int TriggerAxis_normalizeSdl(int sdl_val) {
	int raw = (sdl_val + 32768) >> 1;
	if (raw < 0)
		raw = 0;
	if (raw > TRIGGER_NORM_MAX)
		raw = TRIGGER_NORM_MAX;
	if (raw <= TRIGGER_RAW_DEADZONE)
		return 0;
	return (int)(((int64_t)(raw - TRIGGER_RAW_DEADZONE) * TRIGGER_NORM_MAX) /
				 (TRIGGER_NORM_MAX - TRIGGER_RAW_DEADZONE));
}

// Digital press state from a normalized [0..32767] trigger value, with ~20%
// press / ~14% release Schmitt-trigger hysteresis to prevent midpoint chatter.
static inline int TriggerAxis_isDigitalPressed(int norm, int was_down) {
	return was_down ? (norm >= TRIGGER_RELEASE_NORM) : (norm >= TRIGGER_PRESS_NORM);
}

// Raw evdev ABS_Z / ABS_RZ hysteresis for screenshot.elf (25% press, 15% release
// of EVIOCGABS maximum, defaulting to 255 when maximum <= 0).
static inline int TriggerAxis_isEvdevPressed(int ev_value, int ev_max, int was_down) {
	int max_v = ev_max > 0 ? ev_max : 255;
	int hi = max_v / 4;
	int lo = (max_v * 3) / 20;
	return was_down ? (ev_value > lo) : (ev_value > hi);
}
