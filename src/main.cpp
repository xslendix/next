#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

#include <raylib.h>

#include "common.h"

#include "GameState.h"
#include "Player.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS
#include <emscripten/emscripten.h>
#endif

static constexpr auto INITIAL_SCREEN_WIDTH = 800;
static constexpr auto INITIAL_SCREEN_HEIGHT = 450;

static void produce_frame(void);

constexpr TextureFilter TEXTURE_FILTER = TEXTURE_FILTER_POINT;

int main(void)
{
	try {
		g_gs.levels.push_back(Level::read_from_file("Level.json"));
		g_gs.current_level = 0;
		g_gs.player.position = g_gs.level()->start_position;
		g_gs.player.angle = g_gs.level()->start_angle;
		g_gs.camera.target = { 0, 0 };
		g_gs.camera.zoom = 2;
		// g_gs.camera.rotation = -90;

#if !defined(_DEBUG)
		SetTraceLogLevel(LOG_NONE);
#endif

#if !defined(PLATFORM_WEB)
		SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
		InitWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "ByteRacer");

		g_gs.target = LoadRenderTexture(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);
		SetTextureFilter(g_gs.target.texture, TEXTURE_FILTER);

#if defined(PLATFORM_WEB)
		emscripten_set_main_loop(produce_frame, 60, 1);
#else
		SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
		while (!WindowShouldClose())
			produce_frame();
#endif

		UnloadRenderTexture(g_gs.target);

		CloseWindow();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}

float lerp(float a, float b, float f) { return a + f * (b - a); }

void produce_frame(void)
{
	if (IsWindowResized()) {
		UnloadRenderTexture(g_gs.target);
		g_gs.target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		SetTextureFilter(g_gs.target.texture, TEXTURE_FILTER);
	}

	double dt = GetFrameTime();

	g_gs.width = GetScreenWidth();
	g_gs.height = GetScreenHeight();
	g_gs.widthf = static_cast<float>(g_gs.width);
	g_gs.heightf = static_cast<float>(g_gs.height);

	g_gs.player.update(dt);

	for (auto &pickup : g_gs.level()->pickups) {
		if (pickup.time_since_pickup != -1) {
			pickup.time_since_pickup += dt;
		} else {
			if (CheckCollisionCircles(
			        g_gs.player.position, PLAYER_RADIUS, pickup.position, PICKUP_RADIUS)) {
				pickup.time_since_pickup = 0;
				g_gs.player.trail.push_back(
				    Player::TrailPickup { &pickup, g_gs.player.get_next_trail_position() });
			}
		}
	}

	g_gs.camera.offset.x = g_gs.widthf / 2.;
	g_gs.camera.offset.y = g_gs.heightf / 2.;
	g_gs.camera.target.x = lerp(g_gs.camera.target.x, g_gs.player.position.x, dt * 4);
	g_gs.camera.target.y = lerp(g_gs.camera.target.y, g_gs.player.position.y, dt * 4);
	g_gs.camera.rotation
	    = lerp(g_gs.camera.rotation, -(g_gs.player.angle * RAD2DEG) - 90.0, dt * 2);

	BeginTextureMode(g_gs.target);
	{
		ClearBackground(BLACK);

		if (g_gs.level()) {
			g_gs.level()->render(&g_gs.camera);
		}

		DrawFPS(20, 20);
	}
	EndTextureMode();

	BeginDrawing();
	DrawTexturePro(g_gs.target.texture, { 0, 0, g_gs.widthf, -g_gs.heightf },
	    { 0, 0, g_gs.widthf, g_gs.heightf }, { 0, 0 }, 0.0f, WHITE);
	EndDrawing();
}
