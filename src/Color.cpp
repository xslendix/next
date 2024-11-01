#include "Color.h"

#include <tuple>

static float get_random_hue(void) {
    int hue = GetRandomValue(0, 360);
    // Avoid the orange, yellow, and green ranges
    while ((hue >= 15 && hue <= 70) || (hue >= 85 && hue <= 150)) {
        hue = GetRandomValue(0, 360);
    }
    return static_cast<float>(hue);
}

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

    p.danger_zone_background = ColorFromHSV(0.0f, fixed_saturation, 0.5f);

    return p;
}
