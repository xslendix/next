#include "raylib.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS
#include <emscripten/emscripten.h>
#endif

#include <cstdlib>
#include <cstring>

static int const screenWidth = 800;
static int const screenHeight = 450;

static RenderTexture2D target {};

static void UpdateDrawFrame(void);

constexpr TextureFilter TEXTURE_FILTER = TEXTURE_FILTER_POINT;

int main(void)
{
#if !defined(_DEBUG)
	SetTraceLogLevel(LOG_NONE);
#endif

	InitWindow(screenWidth, screenHeight, "ByteRacer");

	target = LoadRenderTexture(screenWidth, screenHeight);
	SetTextureFilter(target.texture, TEXTURE_FILTER);

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	while (!WindowShouldClose())
		UpdateDrawFrame();
#endif

	UnloadRenderTexture(target);

	CloseWindow();

	return 0;
}

void UpdateDrawFrame(void)
{
	if (IsWindowResized()) {
		UnloadRenderTexture(target);
		target = LoadRenderTexture(screenWidth, screenHeight);
		SetTextureFilter(target.texture, TEXTURE_FILTER);
	}

	BeginTextureMode(target);
	ClearBackground(RAYWHITE);

	DrawText("Welcome to raylib NEXT gamejam!", 150, 140, 30, BLACK);
	DrawRectangleLinesEx((Rectangle) { 0, 0, screenWidth, screenHeight }, 16, BLACK);

	EndTextureMode();

	BeginDrawing();
	ClearBackground(RAYWHITE);

	DrawTexturePro(target.texture,
	    (Rectangle) { 0, 0, (float)target.texture.width, -(float)target.texture.height },
	    (Rectangle) { 0, 0, (float)target.texture.width, (float)target.texture.height },
	    (Vector2) { 0, 0 }, 0.0f, WHITE);

	EndDrawing();
}
