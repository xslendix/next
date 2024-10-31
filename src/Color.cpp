#include "Color.h"

#include <tuple>

static float get_random_hue(void) { return static_cast<float>(GetRandomValue(0, 360)); }
static float get_random_saturation(void)
{
	return static_cast<float>(GetRandomValue(70, 100)) / 100.0f;
}
static float get_random_value(float min, float max)
{
	return static_cast<float>(
	           GetRandomValue(static_cast<int>(min * 100), static_cast<int>(max * 100)))
	    / 100.0f;
}

static std::pair<Color, Color> get_random_colors(float v1 = .7, float v2 = .3)
{
	float hue = get_random_hue();
	auto  c1 = ColorFromHSV(hue, get_random_saturation(), v1);
	auto  c2 = ColorFromHSV(hue, get_random_saturation(), v2);
	c1.a = 0xff;
	c2.a = 0xff;
	return { c1, c2 };
}

ColorPalette ColorPalette::generate(void)
{
	ColorPalette p;
	std::tie(p.primary, p.menu_background)
	    = get_random_colors(.8, .3); // More contrast in brightness
	std::tie(p.wall, p.game_background) = get_random_colors(.7, .4); // Different contrast level

	// Specific colors with contrasting hues
	p.key_door
	    = ColorFromHSV(GetRandomValue(35, 57), get_random_saturation(), get_random_value(.8, .9));
	p.file
	    = ColorFromHSV(GetRandomValue(120, 150), get_random_saturation(), get_random_value(.6, .9));
	p.one_way_zone_background = ColorBrightness(p.game_background, -.25);
	p.danger_zone_background
	    = ColorFromHSV(GetRandomValue(0, 16), get_random_saturation(), get_random_value(.3, .5));

	return p;
}
