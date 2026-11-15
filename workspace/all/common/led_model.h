#pragma once

// Pure LED zone-matching and addressable-bar helpers shared by common/api.c and
// tg5040/platform/platform.c. No SDL or OS headers; host-tested by
// common/tests/test_led_model.c.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Zone suffix semantics across TrimUI models:
//   "m":       Top bar (Brick / Brick Pro) or Logo (Smart Pro / Smart Pro S)
//   "f1"/"f2": FN 1 / FN 2 keys (Brick / Brick Pro)
//   "rear":    L/R triggers on Brick Pro (5-zone / 6-zone)
//   "lr":      L/R triggers on Brick (4-zone), or unified Joysticks on Brick Pro (5-zone)
//   "l"/"r":   Left / Right joysticks on Smart Pro / S and split-stick Brick Pro (6-zone)
//
// Ambient modes:
//   1 = All, 2 = Top, 3 = FN, 4 = LR (Triggers), 5 = Top/LR, 6 = Sticks, 7 = Sticks/LR
static inline int LEDModel_isAmbientZoneActive(int mode, int has_rear, const char* z) {
	if (!z)
		return 0;
	if (mode == 1)
		return 1;
	if (mode == 2)
		return strcmp(z, "m") == 0;
	if (mode == 3)
		return strcmp(z, "f1") == 0 || strcmp(z, "f2") == 0;
	if (mode == 4)
		return has_rear ? (strcmp(z, "rear") == 0)
						: (strcmp(z, "lr") == 0 || strcmp(z, "l") == 0 || strcmp(z, "r") == 0);
	if (mode == 5)
		return (strcmp(z, "m") == 0) ||
			   (has_rear ? (strcmp(z, "rear") == 0)
						 : (strcmp(z, "lr") == 0 || strcmp(z, "l") == 0 || strcmp(z, "r") == 0));
	if (mode == 6)
		return strcmp(z, "l") == 0 || strcmp(z, "r") == 0 || (has_rear && strcmp(z, "lr") == 0);
	if (mode == 7)
		return strcmp(z, "lr") == 0 || strcmp(z, "rear") == 0 || strcmp(z, "l") == 0 || strcmp(z, "r") == 0;
	return 0;
}
