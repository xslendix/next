#pragma once

#include <raylib.h>

#include "common.h"

struct GameState {
	i32 width;
	i32 height;
	f32 widthf;
	f32 heightf;

	RenderTexture2D target {};
};

extern GameState g_gs;
