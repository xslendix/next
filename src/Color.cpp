#include "Color.h"

#include <tuple>

Color ColorLerp(Color a, Color b, float amnt)
{
	Color color = { 0 };

	if (amnt < 0.0f)
		amnt = 0.0f;
	else if (amnt > 1.0f)
		amnt = 1.0f;

	color.r = (unsigned char)((1.0f - amnt) * a.r + amnt * b.r);
	color.g = (unsigned char)((1.0f - amnt) * a.g + amnt * b.g);
	color.b = (unsigned char)((1.0f - amnt) * a.b + amnt * b.b);
	color.a = (unsigned char)((1.0f - amnt) * a.a + amnt * b.a);

	return color;
}

static float get_random_hue(void) { return static_cast<float>(GetRandomValue(220, 250)); }

static constexpr float fixed_saturation = 0.8f;

static std::pair<Color, Color> get_colors(float hue, float v1 = .7f, float v2 = .4f)
{
	auto c1 = ColorFromHSV(hue, fixed_saturation, v1);
	auto c2 = ColorFromHSV(hue, fixed_saturation, v2);
	c1.a = 0xff;
	c2.a = 0xff;
	return { c1, c2 };
}

static float get_contrasting_hue(float base_hue, float min_difference)
{
	float hue_offset = GetRandomValue(static_cast<int>(min_difference), static_cast<int>(180.0f));
	float sign = GetRandomValue(0, 1) == 0 ? -1.0f : 1.0f;
	float new_hue = fmod(base_hue + sign * hue_offset + 360.0f, 360.0f);
	return new_hue;
}

ColorPalette ColorPalette::generate(void)
{
	ColorPalette p;

	float bg_hue = get_random_hue();
	std::tie(p.wall, p.game_background) = get_colors(bg_hue);

	float primary_hue = fmod(bg_hue + 180.0f, 360.0f);

	if (primary_hue >= 270.0f && primary_hue <= 300.0f) {
		primary_hue = fmod(primary_hue + 60.0f, 360.0f);
	}

	p.primary = ColorFromHSV(primary_hue, fixed_saturation, 0.75f);
	p.primary.a = 0xff;

	p.menu_background = ColorBrightness(p.game_background, -0.2f);

	float key_door_hue = fmod(bg_hue + 90.0f, 360.0f);
	p.key_door = ColorFromHSV(key_door_hue, fixed_saturation, 0.85f);
	p.key_door.a = 0xff;

	float file_hue = get_contrasting_hue(bg_hue, 60.0f);
	p.file = ColorFromHSV(file_hue, fixed_saturation, 0.7f);
	p.file.a = 0xff;

	p.one_way_zone_background = ColorBrightness(p.game_background, -0.25f);

	p.danger_zone_background
	    = ColorLerp(ColorFromHSV(0.0f, fixed_saturation, 0.5f), p.game_background, 0.2);

	return p;
}
