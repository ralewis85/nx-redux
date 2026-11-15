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

	printf("test_led_model: OK\n");
	return 0;
}
