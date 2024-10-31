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

	ColorPalette palette;

	std::optional<usize> current_level;

	// Rendering
	i32 width;
	i32 height;
	f32 widthf;
	f32 heightf;

	RenderTexture2D target {};
	Camera2D        camera {};

	Level *level()
	{
		if (!current_level)
			return nullptr;
		return &this->levels.at(*current_level);
	}
};

extern GameState g_gs;
