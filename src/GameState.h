#pragma once

#include <raylib.h>

#include "common.h"

#include "Color.h"
#include "Level.h"
#include "Player.h"

struct GameState {
	// Game logic
	std::vector<Level> levels;
	Player             player;
	f64                time_spent;
	f64                completion_time;

	int collected_files, total_files;

	int total_collected_files = 0;

	ColorPalette palette;

	std::optional<usize> current_level;

	// Rendering
	i32 width;
	i32 height;
	f32 widthf;
	f32 heightf;

	f32 menu_scroll = 0;

	std::vector<Vector2> menu_particles;
	std::vector<float>   menu_particle_speeds;

	RenderTexture2D target {};
	Camera2D        camera {};
	Font            font;

	Level *level()
	{
		if (!current_level)
			return nullptr;
		return &this->levels.at(*current_level);
	}
};

extern GameState g_gs;

static char const *format_time(f64 time)
{
	return TextFormat("%02d:%02d:%02d", static_cast<int>(time / 60) % 60,
	    static_cast<int>(time) % 60, static_cast<int>(time * 100) % 100);
}
