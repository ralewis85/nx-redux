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

#define LED_MODEL_BAR_TOP 1
#define LED_MODEL_BAR_STICKS 2
#define LED_MODEL_FRAME_SLOTS 23

// Number of lit segments (1..8 when active, 0 when mode == 0) for charge_pct in [0..100].
static inline int LEDModel_batteryBarSegments(int mode, int charge_pct) {
	if (mode == 0)
		return 0;
	if (charge_pct <= 0)
		return 1;
	if (charge_pct >= 100)
		return 8;
	int segs = (charge_pct * 8 + 99) / 100;
	return segs > 8 ? 8 : segs;
}

// 24-bit RGB battery-bar color: cyan-mint while charging, green >= 50%, amber 20-49%, red < 20%.
static inline uint32_t LEDModel_batteryBarColor(int charge_pct, bool charging) {
	if (charging)
		return 0x00FF88u;
	if (charge_pct >= 50)
		return 0x00E640u;
	if (charge_pct >= 20)
		return 0xFFAA00u;
	return 0xFF2000u;
}

// Updates a 23-slot Brick Pro sunxi_ledc DMA frame buffer in place:
//   [0..7] = top bar, [8..9] = F2/F1 (preserved), [10..17] = left+right stick rings,
//   [18..21] = rear triggers (preserved), [22] = padding (0).
static inline void LEDModel_applyBatteryBarSlots(uint32_t raw[LED_MODEL_FRAME_SLOTS],
												 int mode, int released_bits,
												 int segments, uint32_t color) {
	if (released_bits & LED_MODEL_BAR_TOP) {
		for (int k = 0; k < 8; k++)
			raw[k] = 0;
	}
	if (released_bits & LED_MODEL_BAR_STICKS) {
		for (int k = 0; k < 8; k++)
			raw[10 + k] = 0;
	}
	if (mode & LED_MODEL_BAR_TOP) {
		for (int k = 0; k < 8; k++)
			raw[k] = (k < segments) ? color : 0;
	}
	if (mode & LED_MODEL_BAR_STICKS) {
		for (int k = 0; k < 8; k++)
			raw[10 + k] = (k < segments) ? color : 0;
	}
	raw[22] = 0;
}
