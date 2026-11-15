// Host test for led_model.h: ambient zone routing across Brick (4-zone),
// Brick Pro (5-zone & 6-zone split sticks), and Smart Pro / Smart Pro S (3-zone).
#include <assert.h>
#include <stdio.h>
#include "../led_model.h"

int main(void) {
	// Brick (4-zone, has_rear = 0): "lr" is L/R triggers
	assert(LEDModel_isAmbientZoneActive(1, 0, "f1") == 1);
	assert(LEDModel_isAmbientZoneActive(2, 0, "m") == 1 && LEDModel_isAmbientZoneActive(2, 0, "lr") == 0);
	assert(LEDModel_isAmbientZoneActive(3, 0, "f1") == 1 && LEDModel_isAmbientZoneActive(3, 0, "f2") == 1 && LEDModel_isAmbientZoneActive(3, 0, "m") == 0);
	assert(LEDModel_isAmbientZoneActive(4, 0, "lr") == 1 && LEDModel_isAmbientZoneActive(4, 0, "m") == 0);
	assert(LEDModel_isAmbientZoneActive(5, 0, "m") == 1 && LEDModel_isAmbientZoneActive(5, 0, "lr") == 1 && LEDModel_isAmbientZoneActive(5, 0, "f1") == 0);
	assert(LEDModel_isAmbientZoneActive(6, 0, "lr") == 0); // Brick has no sticks

	// Brick Pro 5-zone (has_rear = 1): "lr" is joysticks, "rear" is triggers
	assert(LEDModel_isAmbientZoneActive(4, 1, "rear") == 1 && LEDModel_isAmbientZoneActive(4, 1, "lr") == 0);
	assert(LEDModel_isAmbientZoneActive(5, 1, "m") == 1 && LEDModel_isAmbientZoneActive(5, 1, "rear") == 1 && LEDModel_isAmbientZoneActive(5, 1, "lr") == 0);
	assert(LEDModel_isAmbientZoneActive(6, 1, "lr") == 1 && LEDModel_isAmbientZoneActive(6, 1, "rear") == 0);
	assert(LEDModel_isAmbientZoneActive(7, 1, "lr") == 1 && LEDModel_isAmbientZoneActive(7, 1, "rear") == 1 && LEDModel_isAmbientZoneActive(7, 1, "m") == 0);

	// Brick Pro 6-zone split sticks (has_rear = 1): "l" and "r" are joysticks, "rear" is triggers
	assert(LEDModel_isAmbientZoneActive(6, 1, "l") == 1 && LEDModel_isAmbientZoneActive(6, 1, "r") == 1 && LEDModel_isAmbientZoneActive(6, 1, "rear") == 0);
	assert(LEDModel_isAmbientZoneActive(7, 1, "l") == 1 && LEDModel_isAmbientZoneActive(7, 1, "r") == 1 && LEDModel_isAmbientZoneActive(7, 1, "rear") == 1);

	// Smart Pro / Smart Pro S (3-zone, has_rear = 0): "l", "r", "m"
	assert(LEDModel_isAmbientZoneActive(6, 0, "l") == 1 && LEDModel_isAmbientZoneActive(6, 0, "r") == 1 && LEDModel_isAmbientZoneActive(6, 0, "m") == 0);
	assert(LEDModel_isAmbientZoneActive(0, 1, "m") == 0);
	assert(LEDModel_isAmbientZoneActive(1, 1, NULL) == 0);

	// Battery bar segment quantization & color thresholds
	assert(LEDModel_batteryBarSegments(0, 80) == 0);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 0) == 1);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, -5) == 1);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 12) == 1);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 13) == 2);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 50) == 4);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 88) == 8);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 100) == 8);
	assert(LEDModel_batteryBarSegments(LED_MODEL_BAR_TOP, 120) == 8);

	assert(LEDModel_batteryBarColor(10, true) == 0x00FF88u);
	assert(LEDModel_batteryBarColor(75, false) == 0x00E640u);
	assert(LEDModel_batteryBarColor(50, false) == 0x00E640u);
	assert(LEDModel_batteryBarColor(49, false) == 0xFFAA00u);
	assert(LEDModel_batteryBarColor(20, false) == 0xFFAA00u);
	assert(LEDModel_batteryBarColor(19, false) == 0xFF2000u);

	// 23-slot sunxi_ledc DMA frame buffer: preserves F2/F1 [8..9] & rear [18..21],
	// lights top [0..7] and/or sticks [10..17], and clears released zones cleanly.
	uint32_t raw[LED_MODEL_FRAME_SLOTS];
	for (int i = 0; i < LED_MODEL_FRAME_SLOTS; i++)
		raw[i] = 0x123456u;

	// Both top + sticks at 50% (4 segments)
	LEDModel_applyBatteryBarSlots(raw, LED_MODEL_BAR_TOP | LED_MODEL_BAR_STICKS, 0, 4, 0x00E640u);
	for (int k = 0; k < 8; k++) {
		assert(raw[k] == (k < 4 ? 0x00E640u : 0u));
		assert(raw[10 + k] == (k < 4 ? 0x00E640u : 0u));
	}
	assert(raw[8] == 0x123456u && raw[9] == 0x123456u);
	assert(raw[18] == 0x123456u && raw[21] == 0x123456u);
	assert(raw[22] == 0u);

	// Switch from Top+Sticks -> Top only (released_bits = STICKS): sticks cleared to 0
	LEDModel_applyBatteryBarSlots(raw, LED_MODEL_BAR_TOP, LED_MODEL_BAR_STICKS, 2, 0xFFAA00u);
	assert(raw[0] == 0xFFAA00u && raw[1] == 0xFFAA00u && raw[2] == 0u);
	for (int k = 0; k < 8; k++)
		assert(raw[10 + k] == 0u);
	assert(raw[8] == 0x123456u && raw[18] == 0x123456u);

	printf("test_led_model: OK\n");
	return 0;
}
