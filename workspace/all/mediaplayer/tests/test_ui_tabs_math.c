// Host unit test for ui_tabs_math.h (Minimal distance-opacity curve, clamping,
// and bottom tab-bar right-hint overflow fitting).
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "../ui_tabs_math.h"

static int nearf(float a, float b) {
	return fabsf(a - b) < 1e-5f;
}

int main(void) {
	assert(nearf(MediaTabs_clamp01f(-0.5f), 0.0f));
	assert(nearf(MediaTabs_clamp01f(0.35f), 0.35f));
	assert(nearf(MediaTabs_clamp01f(1.5f), 1.0f));

	// Selected row (d == 0) has full opacity 1.0; items below stay brighter than above
	assert(nearf(MediaTabs_distanceAlpha(0.0f), 1.0f));
	assert(nearf(MediaTabs_distanceAlpha(1.0f), 0.80f));
	assert(nearf(MediaTabs_distanceAlpha(-1.0f), 0.60f));
	assert(nearf(MediaTabs_distanceAlpha(2.0f), 0.60f));
	assert(nearf(MediaTabs_distanceAlpha(-2.0f), 0.48f));
	assert(nearf(MediaTabs_distanceAlpha(10.0f), 0.42f));
	assert(nearf(MediaTabs_distanceAlpha(-10.0f), 0.40f));

	// Hint fitting on 1024px Brick Pro vs cramped bar
	int widths[2] = {120, 140};
	int total = -1;
	// Plenty of room: both pairs fit (120 + 20 + 140 = 280)
	assert(MediaTabs_fitHintPairs(380, 40, 960, widths, 2, 20, &total) == 2 && total == 280);
	// Tight bar: drops trailing pair so remaining pair fits cleanly without overlapping tabs
	assert(MediaTabs_fitHintPairs(380, 40, 620, widths, 2, 20, &total) == 1 && total == 120);
	// Very narrow: drops all hints gracefully instead of colliding with tab text
	assert(MediaTabs_fitHintPairs(380, 40, 500, widths, 2, 20, &total) == 0 && total == 0);

	printf("test_ui_tabs_math: OK\n");
	return 0;
}
