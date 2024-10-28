#pragma once

#include <raylib.h>

#include "common.h"

#include "Level.h"
#include "LevelEditor.h"

struct GameState {
	// Game logic
	std::vector<Level> levels;
	LevelEditor        level_editor;

	usize current_level;

	// Rendering
	i32 width;
	i32 height;
	f32 widthf;
	f32 heightf;

	RenderTexture2D target {};
	Camera          camera {};

	Level &level() { return this->levels.at(current_level); }
};

extern GameState g_gs;
