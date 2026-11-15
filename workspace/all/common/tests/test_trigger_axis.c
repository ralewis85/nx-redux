// Host test for trigger_axis.h: SDL [-32768..32767] Hall-effect trigger
// normalization, resting deadzone, Schmitt-trigger digital hysteresis, and
// screenshot.c raw evdev EVIOCGABS hysteresis.
#include <assert.h>
#include <stdio.h>
#include "../trigger_axis.h"

int main(void) {
	// Brick digital extremes: -32768 -> 0 (released), +32767 -> 32767 (pressed)
	assert(TriggerAxis_normalizeSdl(-32768) == 0);
	assert(TriggerAxis_normalizeSdl(32767) == 32767);
	assert(TriggerAxis_isDigitalPressed(0, 0) == 0);
	assert(TriggerAxis_isDigitalPressed(32767, 0) == 1);

	// Out-of-range clamping
	assert(TriggerAxis_normalizeSdl(-40000) == 0);
	assert(TriggerAxis_normalizeSdl(40000) == 32767);

	// Resting Hall sensor noise within ~3.1% deadzone stays strictly 0
	assert(TriggerAxis_normalizeSdl(-32768 + 2048) == 0); // raw == 1024
	assert(TriggerAxis_normalizeSdl(-32768 + 2050) > 0);  // raw == 1025

	// Monotonic across full SDL range
	int prev = 0;
	for (int v = -32768; v <= 32767; v += 64) {
		int n = TriggerAxis_normalizeSdl(v);
		assert(n >= prev && n >= 0 && n <= 32767);
		prev = n;
	}

	// Digital Schmitt-trigger hysteresis: presses at >= 6553, holds down until < 4587
	int down = 0;
	down = TriggerAxis_isDigitalPressed(5000, down);
	assert(down == 0); // above release threshold, below press threshold -> still up
	down = TriggerAxis_isDigitalPressed(6553, down);
	assert(down == 1); // crosses press threshold -> latches down
	down = TriggerAxis_isDigitalPressed(5000, down);
	assert(down == 1); // stays down inside hysteresis band (no chatter)
	down = TriggerAxis_isDigitalPressed(4587, down);
	assert(down == 1); // exact release floor still held
	down = TriggerAxis_isDigitalPressed(4586, down);
	assert(down == 0); // drops below release floor -> cleanly released

	// Raw evdev EVIOCGABS hysteresis (screenshot.elf): 25% press, 15% release
	assert(TriggerAxis_isEvdevPressed(1, 255, 0) == 0);	 // 1 ADC count noise ignored!
	assert(TriggerAxis_isEvdevPressed(63, 255, 0) == 0); // 63 <= 63
	assert(TriggerAxis_isEvdevPressed(64, 255, 0) == 1); // 64 > 63 -> pressed
	assert(TriggerAxis_isEvdevPressed(40, 255, 1) == 1); // 40 > 38 -> held
	assert(TriggerAxis_isEvdevPressed(38, 255, 1) == 0); // 38 <= 38 -> released
	assert(TriggerAxis_isEvdevPressed(1025, 4095, 0) == 1);
	assert(TriggerAxis_isEvdevPressed(70, 0, 0) == 1); // fallback max=255

	printf("test_trigger_axis: OK\n");
	return 0;
}
