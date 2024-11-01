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

static std::pair<Color, Color> get_random_colors(float hue, float v1 = .7, float v2 = .3)
{
	auto c1 = ColorFromHSV(hue, get_random_saturation(), v1);
	auto c2 = ColorFromHSV(hue, get_random_saturation(), v2);
	c1.a = 0xff;
	c2.a = 0xff;
	return { c1, c2 };
}

ColorPalette ColorPalette::generate(void)
{
	ColorPalette p;
	float bg_hue = get_random_hue();
	std::tie(p.wall, p.game_background) = get_random_colors(bg_hue, .7, .4);

	float primary_hue = fmod(bg_hue + 180.0f, 360.0f); // opposite hue
	p.primary = ColorFromHSV(primary_hue, get_random_saturation(), 0.75f);
	p.primary.a = 0xff;

	p.menu_background = ColorBrightness(p.game_background, -.6);

	p.key_door
	    = ColorFromHSV(GetRandomValue(35, 57), get_random_saturation(), get_random_value(.8, .9));
	p.file
	    = ColorFromHSV(GetRandomValue(120, 150), get_random_saturation(), get_random_value(.6, .9));
	p.one_way_zone_background = ColorBrightness(p.game_background, -.25);
	p.danger_zone_background
	    = ColorFromHSV(GetRandomValue(0, 16), get_random_saturation(), get_random_value(.3, .5));

	return p;
}
