#include <filesystem>
#include <string>

#include <raylib.h>

#include "common.h"

#include "GameState.h"

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
	Level new_level("238.244.3.56", 0);
	new_level.walls.push_back({ Level::Wall::Kind::Wall });
	new_level.walls.push_back({ Level::Wall::Kind::Door });
	new_level.zones.push_back({ Level::Zone::Kind::End });
	new_level.zones.push_back({ Level::Zone::Kind::DialogTrigger });
	new_level.zones.push_back({ Level::Zone::Kind::OneWay });
	new_level.zones.push_back({ Level::Zone::Kind::Danger });
	new_level.pickups.push_back({});
	new_level.export_to_file("Level.json");

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
	emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	while (!WindowShouldClose())
		produce_frame();
#endif

	UnloadRenderTexture(g_gs.target);

	CloseWindow();

	return 0;
}

void produce_frame(void)
{
	if (IsWindowResized()) {
		UnloadRenderTexture(g_gs.target);
		g_gs.target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		SetTextureFilter(g_gs.target.texture, TEXTURE_FILTER);
	}

	g_gs.width = GetScreenWidth();
	g_gs.height = GetScreenHeight();
	g_gs.widthf = static_cast<float>(g_gs.width);
	g_gs.heightf = static_cast<float>(g_gs.height);

	BeginTextureMode(g_gs.target);
	ClearBackground(RAYWHITE);

	DrawFPS(20, 20);

	EndTextureMode();

	BeginDrawing();

	DrawTexturePro(g_gs.target.texture, { 0, 0, g_gs.widthf, -g_gs.heightf },
	    { 0, 0, g_gs.widthf, g_gs.heightf }, { 0, 0 }, 0.0f, WHITE);

	EndDrawing();
}