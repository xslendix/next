#pragma once

#include <raylib.h>

struct ColorPalette {
	Color primary;
	Color game_background;
	Color menu_background;
	Color wall;
	Color key_door;
	Color file;
	Color one_way_zone_background;
	Color danger_zone_background;

	static ColorPalette generate(void);
};
