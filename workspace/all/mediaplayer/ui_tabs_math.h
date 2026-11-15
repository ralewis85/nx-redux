#ifndef __UI_TABS_MATH_H__
#define __UI_TABS_MATH_H__

// Pure layout & Minimal typography helpers for Media Player's tabbed UI
// (ui_tabs.c). No SDL headers; host-tested by tests/test_ui_tabs_math.c.

static inline float MediaTabs_clamp01f(float v) {
	return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// Asymmetric distance-based opacity curve matching nextui's HomeMin_alpha:
// rows below the active selection stay slightly brighter than scrolled-past
// rows above it.
static inline float MediaTabs_distanceAlpha(float d) {
	if (d >= 0.0f) {
		if (d <= 1.0f)
			return 1.0f + (0.80f - 1.0f) * d;
		if (d <= 2.0f)
			return 0.80f + (0.60f - 0.80f) * (d - 1.0f);
		if (d <= 3.0f)
			return 0.60f + (0.50f - 0.60f) * (d - 2.0f);
		if (d <= 4.0f)
			return 0.50f + (0.42f - 0.50f) * (d - 3.0f);
		return 0.42f;
	}
	float a = -d;
	if (a <= 1.0f)
		return 1.0f + (0.60f - 1.0f) * a;
	if (a <= 2.0f)
		return 0.60f + (0.48f - 0.60f) * (a - 1.0f);
	if (a <= 3.0f)
		return 0.48f + (0.40f - 0.48f) * (a - 2.0f);
	return 0.40f;
}

// Returns how many right-hand button-hint pairs (0..pair_count) fit to the
// right of `tabs_end_x + gap` without exceeding `right_edge`, updating
// `*out_total_w` to their combined pixel width.
static inline int MediaTabs_fitHintPairs(int tabs_end_x, int gap, int right_edge,
										 const int* pair_widths, int pair_count,
										 int hint_gap, int* out_total_w) {
	while (pair_count > 0) {
		int total = 0;
		for (int i = 0; i < pair_count; i++)
			total += pair_widths[i] + (i > 0 ? hint_gap : 0);
		if (tabs_end_x + gap + total <= right_edge) {
			if (out_total_w)
				*out_total_w = total;
			return pair_count;
		}
		pair_count--;
	}
	if (out_total_w)
		*out_total_w = 0;
	return 0;
}

#endif
