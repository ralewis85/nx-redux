// Host ABI & migration parity test between common/msettings_shm.h (the linkless
// shared-memory mirror used by osdctl and poweroff_next) and each platform's
// libmsettings/msettings.c Settings struct (extracted by test_msettings_shm.sh).
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../msettings_shm.h"
#include "msettings_extracted.h"

#define ASSERT_FIELD(field)                                             \
	assert(offsetof(SettingsShm, field) == offsetof(Settings, field) && \
		   sizeof(((SettingsShm*)0)->field) == sizeof(((Settings*)0)->field))

int main(void) {
	assert(MSETTINGS_SHM_VERSION == SETTINGS_VERSION);
	assert(sizeof(SettingsShm) == sizeof(Settings));

	ASSERT_FIELD(version);
	ASSERT_FIELD(brightness);
	ASSERT_FIELD(colortemperature);
	ASSERT_FIELD(headphones);
	ASSERT_FIELD(speaker);
	ASSERT_FIELD(fn_mode);
	ASSERT_FIELD(contrast);
	ASSERT_FIELD(saturation);
	ASSERT_FIELD(exposure);
	ASSERT_FIELD(fn_brightness);
	ASSERT_FIELD(fn_colortemperature);
	ASSERT_FIELD(fn_contrast);
	ASSERT_FIELD(fn_saturation);
	ASSERT_FIELD(fn_exposure);
	ASSERT_FIELD(fn_volume);
#ifndef HAS_FAN
	ASSERT_FIELD(fn_dpad_disabled);
	ASSERT_FIELD(fn_dpad_joystick);
#endif
	ASSERT_FIELD(fn_prevent_wake);
	ASSERT_FIELD(turbo_a);
	ASSERT_FIELD(turbo_b);
	ASSERT_FIELD(turbo_x);
	ASSERT_FIELD(turbo_y);
	ASSERT_FIELD(turbo_l1);
	ASSERT_FIELD(turbo_l2);
	ASSERT_FIELD(turbo_r1);
	ASSERT_FIELD(turbo_r2);
	ASSERT_FIELD(rumble_off);
	ASSERT_FIELD(rumble_strength);
	ASSERT_FIELD(game_volume);
	ASSERT_FIELD(music_volume);
	ASSERT_FIELD(speaker_mute);
	ASSERT_FIELD(jack);
	ASSERT_FIELD(audiosink);
#ifdef HAS_FAN
	ASSERT_FIELD(fanSpeed);
#endif

	// Verify old -> current struct size delta is exactly one int (fn_prevent_wake)
	assert(sizeof(Settings) == sizeof(PrevSettings) + sizeof(int));
	return 0;
}
