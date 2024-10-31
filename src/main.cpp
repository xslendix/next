#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

#include <raylib.h>

#include "common.h"

#include "GameMath.h"
#include "GameState.h"
#include "Player.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS
#include <emscripten/emscripten.h>
#endif

static constexpr auto INITIAL_SCREEN_WIDTH = 800;
static constexpr auto INITIAL_SCREEN_HEIGHT = INITIAL_SCREEN_WIDTH;

static void produce_frame(void);

constexpr TextureFilter TEXTURE_FILTER = TEXTURE_FILTER_POINT;

void set_level(usize i, bool reset_dialog = false);

int main(void)
{
	try {
		if (!std::filesystem::exists("resources")) {
			std::filesystem::path currentPath = std::filesystem::current_path();
			std::filesystem::path parentPath = currentPath.parent_path();
			ChangeDirectory(parentPath.string().c_str());
		}

		g_gs.palette = ColorPalette::generate();
		g_gs.levels.push_back(Level::read_from_file(RESOURCES_PATH "Level.json"));
		g_gs.current_level = 0;
		set_level(0);

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

void set_level(usize i, bool reset_dialog)
{
	g_gs.current_level = i;
	auto &lvl = *g_gs.level();
	for (auto &wall : lvl.walls) {
		wall.time_since_trigger = -1;
	}
	for (auto &zone : lvl.zones) {
		if (!reset_dialog && zone.kind != Level::Zone::Kind::DialogTrigger)
			zone.time_since_trigger = -1;
	}
	for (auto &pickup : lvl.pickups) {
		pickup.time_since_pickup = -1;
	}

	g_gs.player.position = g_gs.level()->start_position;
	g_gs.player.velocity = { 0, 0 };
	g_gs.player.angle = g_gs.level()->start_angle;
	g_gs.player.trail.clear();
	g_gs.player.health = PLAYER_MAX_HP;

	g_gs.camera.target = g_gs.player.position;
	g_gs.camera.zoom = 2;
	g_gs.camera.rotation = -g_gs.player.angle * RAD2DEG - 90;

	g_gs.time_spent = 0;
}

void produce_frame(void)
{
	if (IsWindowResized()) {
		UnloadRenderTexture(g_gs.target);
		g_gs.target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		SetTextureFilter(g_gs.target.texture, TEXTURE_FILTER);
	}

	double dt = GetFrameTime();

	g_gs.time_spent += dt;

	if (IsKeyPressed(KEY_R)) {
		set_level(*g_gs.current_level);
	}
	if (IsKeyPressed(KEY_P)) {
		g_gs.palette = ColorPalette::generate();
	}

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

	bool in_danger = false;
	for (auto &zone : g_gs.level()->zones) {
		if (CheckCollisionCirclePoly(g_gs.player.position, PLAYER_RADIUS, zone.points)) {
			// TraceLog(LOG_INFO, "Colliding with zone");
			if (!in_danger && zone.kind == Level::Zone::Kind::Danger) {
				in_danger = true;
			}
		}
	}

	if (in_danger)
		g_gs.player.health -= dt;
	else
		g_gs.player.health += dt;

	if (g_gs.player.health > PLAYER_MAX_HP)
		g_gs.player.health = PLAYER_MAX_HP;

	g_gs.camera.offset.x = g_gs.widthf / 2.;
	g_gs.camera.offset.y = g_gs.heightf / 2.;
	g_gs.camera.target.x = lerp(g_gs.camera.target.x, g_gs.player.position.x, dt * 4);
	g_gs.camera.target.y = lerp(g_gs.camera.target.y, g_gs.player.position.y, dt * 4);
	g_gs.camera.rotation
	    = lerp(g_gs.camera.rotation, -(g_gs.player.angle * RAD2DEG) - 90.0, dt * 2);

	BeginTextureMode(g_gs.target);
	{
		ClearBackground(g_gs.palette.game_background);

		if (g_gs.level()) {
			g_gs.level()->render(&g_gs.camera);
		}

		if (g_gs.player.health != PLAYER_MAX_HP) {
			constexpr auto BAR_WIDTH = 30.f;
			Vector2 hp_position = {
				g_gs.widthf / 2,
				g_gs.heightf * 0.8,
			};
			Vector2 left = { hp_position.x - BAR_WIDTH * (g_gs.player.health / PLAYER_MAX_HP), hp_position.y };
			Vector2 right = { hp_position.x + BAR_WIDTH * (g_gs.player.health / PLAYER_MAX_HP), hp_position.y };
			DrawLineEx(left, right, 3, g_gs.palette.primary);
		}

		DrawFPS(20, 60);
		DrawText(TextFormat("%02d:%02d:%02d", static_cast<int>(g_gs.time_spent / 60) % 60,
		             static_cast<int>(g_gs.time_spent) % 60,
		             static_cast<int>(g_gs.time_spent * 100) % 100),
		    20, 20, 20, g_gs.palette.primary);
	}
	EndTextureMode();

	BeginDrawing();
	DrawTexturePro(g_gs.target.texture, { 0, 0, g_gs.widthf, -g_gs.heightf },
	    { 0, 0, g_gs.widthf, g_gs.heightf }, { 0, 0 }, 0.0f, WHITE);
	EndDrawing();
}
