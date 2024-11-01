#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>

#include <raylib.h>
#include <raymath.h>

#include "common.h"

#include "GameMath.h"
#include "GameState.h"
#include "Gui.h"
#include "Player.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS
#include <emscripten/emscripten.h>
#endif

static constexpr auto INITIAL_SCREEN_WIDTH = 800;
static constexpr auto INITIAL_SCREEN_HEIGHT = INITIAL_SCREEN_WIDTH;

static void produce_frame(void);

constexpr TextureFilter TEXTURE_FILTER = TEXTURE_FILTER_BILINEAR;

void set_level(usize i, bool reset_dialog = false);
static void DrawTextBoxed(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);

int main(void)
{
	for (usize i = 0; i < 800; i++) {
		g_gs.menu_particles.push_back({
		    static_cast<float>(GetRandomValue(0, INITIAL_SCREEN_WIDTH)),
		    static_cast<float>(GetRandomValue(0, INITIAL_SCREEN_HEIGHT)),
		});
		g_gs.menu_particle_speeds.push_back(GetRandomValue(500, 700));
	}

	try {
		if (!std::filesystem::exists("resources")) {
			std::filesystem::path currentPath = std::filesystem::current_path();
			std::filesystem::path parentPath = currentPath.parent_path();
			ChangeDirectory(parentPath.string().c_str());
		}

		SetRandomSeed(time(nullptr));

		g_gs.palette = ColorPalette::generate();
		for (int i = 0; i < 1; i++) {
			g_gs.levels.push_back(Level::read_from_file(TextFormat(RESOURCES_PATH "Level%d.json", i)));
		}
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

#if !defined(_DEBUG)
	SetTraceLogLevel(LOG_NONE);
#endif

#if !defined(PLATFORM_WEB)
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
#endif
	InitWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "ByteRacer");

	g_gs.spritesheet = LoadTexture("resources/spritesheet.png");
	g_gs.read_dialogs_from_file("resources/Dialog.json");
	g_gs.font = LoadFontEx("resources/SpaceMono-Regular.ttf", 60, nullptr, 0);

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(produce_frame, 0, 1);
#else
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	while (!WindowShouldClose())
		produce_frame();
#endif

	CloseWindow();

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
	g_gs.completion_time = 0;
}

void produce_frame(void)
{
	double dt = GetFrameTime();

	if (IsKeyPressed(KEY_R)) {
		set_level(*g_gs.current_level);
	}
#ifdef _DEBUG
	if (IsKeyPressed(KEY_P)) {
		g_gs.palette = ColorPalette::generate();
	}
#endif

	g_gs.width = GetScreenWidth();
	g_gs.height = GetScreenHeight();
	g_gs.widthf = static_cast<float>(g_gs.width);
	g_gs.heightf = static_cast<float>(g_gs.height);

	if (g_gs.level() && !g_gs.current_dialog) {
		g_gs.time_spent += dt;

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
				if (!in_danger && zone.kind == Level::Zone::Kind::Danger) {
					in_danger = true;
				} else if (zone.kind == Level::Zone::Kind::End) {
					if (!g_gs.completion_time) {
						g_gs.completion_time = g_gs.time_spent;
						g_gs.collected_files = 0;
						g_gs.total_files = 0;
						for (auto &pickup : g_gs.level()->pickups) {
							if (pickup.kind != Level::Pickup::Kind::File)
								continue;
							g_gs.total_files++;
							g_gs.collected_files += pickup.time_since_pickup != -1;
						}
					}
				}
			}
		}

		if (in_danger)
			g_gs.player.health -= dt;
		else
			g_gs.player.health += dt;

		if (g_gs.player.health > PLAYER_MAX_HP)
			g_gs.player.health = PLAYER_MAX_HP;
		else if (g_gs.player.health < 0)
			set_level(*g_gs.current_level);

		g_gs.camera.offset.x = g_gs.widthf / 2.;
		g_gs.camera.offset.y = g_gs.heightf / 2.;
		g_gs.camera.target.x = lerp(g_gs.camera.target.x, g_gs.player.position.x, dt * 4);
		g_gs.camera.target.y = lerp(g_gs.camera.target.y, g_gs.player.position.y, dt * 4);
		g_gs.camera.rotation
		    = lerp(g_gs.camera.rotation, -(g_gs.player.angle * RAD2DEG) - 90.0, dt * 2);
	} else {
		g_gs.target_menu_scroll += GetMouseWheelMove() * 30;
		g_gs.menu_scroll = lerp(g_gs.menu_scroll, g_gs.target_menu_scroll, dt * 3);
		if (g_gs.target_menu_scroll > 0)
			g_gs.target_menu_scroll = 0;

		for (auto &level : g_gs.levels) {
			if (level.files_required < g_gs.total_collected_files)
				continue;

			if (!level.did_initial_dialog) {
				g_gs.show_dialog(level.name, 0);
				level.did_initial_dialog = true;
			}
		}
	}

	BeginDrawing();
	{
		if (g_gs.level()) {
			ClearBackground(g_gs.palette.game_background);

			g_gs.level()->render(&g_gs.camera);
			if (g_gs.player.health != PLAYER_MAX_HP) {
				constexpr auto BAR_WIDTH = 30.f;
				Vector2        hp_position = {
                    static_cast<float>(g_gs.widthf / 2),
                    static_cast<float>(g_gs.heightf * 0.8),
				};
				Vector2 left = { hp_position.x - BAR_WIDTH * (g_gs.player.health / PLAYER_MAX_HP),
					hp_position.y };
				Vector2 right = { hp_position.x + BAR_WIDTH * (g_gs.player.health / PLAYER_MAX_HP),
					hp_position.y };
				DrawLineEx(left, right, 3, g_gs.palette.primary);
			}

			if (g_gs.completion_time)
				g_gs.level()->render_hud(g_gs.time_spent - g_gs.completion_time);
			else {
				auto text = format_time(g_gs.time_spent);
				int  w = MeasureTextEx(g_gs.font, text, 40, 3).x;
				DrawTextEx(
				    g_gs.font, text, { g_gs.widthf / 2 - w / 2, 20 }, 40, 3, g_gs.palette.primary);
			}
		} else {
			ClearBackground(g_gs.palette.menu_background);

			float t = 0;
			int   i = 1;

			constexpr auto HEIGHT = 60;
			constexpr auto BUTTON_SIZE = 50;
			constexpr auto PADDING = 20;
			constexpr auto FONT_SIZE = BUTTON_SIZE * .95;

			usize i2 = 0;
			for (auto &particle : g_gs.menu_particles) {
				particle.x -= dt * g_gs.menu_particle_speeds[i2];
				if (particle.x < 0) {
					particle.x = g_gs.widthf + particle.x;
					particle.y = static_cast<float>(GetRandomValue(0, g_gs.heightf));
					g_gs.menu_particle_speeds[i2] = GetRandomValue(500, 700);
				}

				DrawRectangle(particle.x, particle.y, 20, 2, g_gs.palette.game_background);
				i2++;
			}

			Vector2 prev;
			for (auto const &level : g_gs.levels) {
				auto    offy = std::sin(t) * HEIGHT;
				auto    x = PADDING + g_gs.menu_scroll + i * BUTTON_SIZE * 3;
				auto    y = g_gs.heightf / 2 + offy;
				Vector2 pos { x, y };

				if (i != 1) {
					auto    diff = Vector2Subtract(pos, prev);
					auto    offset = Vector2Scale(Vector2Normalize(diff), BUTTON_SIZE);
					Vector2 line_start = Vector2Add(prev, offset);
					DrawLineEx(line_start, pos, 3, g_gs.palette.primary);
				}
				prev = pos;

				bool in = CheckCollisionPointCircle(GetMousePosition(), pos, BUTTON_SIZE);

				if (level.files_required) {
					constexpr auto FILE_ICON_SIZE = 15;
					auto txt = TextFormat("%d/%d", 0, level.files_required);
					auto sz = MeasureTextEx(g_gs.font, txt, FILE_ICON_SIZE * 2.5, 2);
					auto file_pos = pos;
					file_pos.y -= BUTTON_SIZE + FILE_ICON_SIZE * 2;
					file_pos.x -= sz.x / 2 - FILE_ICON_SIZE * 0.5;
					g_gs.render_texture({ file_pos.x - FILE_ICON_SIZE, file_pos.y }, 2, 0, FILE_ICON_SIZE, g_gs.palette.file);
					file_pos.y -= sz.y / 2;
					file_pos.x += BUTTON_SIZE / 4;
					DrawTextEx(g_gs.font, txt, file_pos, FILE_ICON_SIZE * 2.5, 2, g_gs.palette.file);
				}

				if (!level.name.empty()) {
					constexpr auto TEXT_SIZE = 10;
					auto txt = level.name.c_str();
					auto sz = MeasureTextEx(g_gs.font, txt, TEXT_SIZE * 2.5, 2);
					auto file_pos = pos;
					file_pos.y += BUTTON_SIZE + TEXT_SIZE;
					file_pos.x -= sz.x / 2;
					DrawTextEx(g_gs.font, txt, file_pos, TEXT_SIZE * 2.5, 2, g_gs.palette.file);
				}

				DrawCircleV(pos, BUTTON_SIZE, g_gs.palette.primary);
				DrawCircleV(pos, BUTTON_SIZE - BORDER_WIDTH,
				    in ? g_gs.palette.game_background : g_gs.palette.menu_background);

				auto txt = TextFormat("%d", i);
				auto w = MeasureTextEx(g_gs.font, txt, FONT_SIZE, 2).x;

				DrawTextEx(g_gs.font, txt, { x - w / 2, static_cast<float>(y - FONT_SIZE / 2) },
				    FONT_SIZE, 2, g_gs.palette.primary);

				if (IsMouseButtonPressed(0) && in) {
					set_level(i - 1);
				}

				t += PI / 2;
				i++;
			}
		}

		if (g_gs.current_dialog) {
			auto const height = g_gs.heightf * .3;
			auto &y = g_gs.dialog_box_y;

			if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(0))
				g_gs.current_dialog_dialog_idx++;

			auto &dia = g_gs.current_dialog->at(g_gs.current_dialog_idx);
			
			y -= dt * 1000 * (g_gs.current_dialog_dialog_idx >= dia.size() ? -1 : 1);

			if (y < g_gs.heightf - height) {
				y = g_gs.heightf - height;
			}

			if (y > g_gs.heightf) {
				g_gs.current_dialog = nullptr;
			}

			DrawRectangle(0, y, g_gs.widthf, height, g_gs.palette.menu_background);
			DrawLineEx({0, y}, {g_gs.widthf, y}, BORDER_WIDTH, g_gs.palette.primary);

			constexpr auto NAME_SIZE = 50;
			constexpr auto NAME_SPACING = 4;
			constexpr auto DIALOG_SIZE = NAME_SIZE * .8;
			constexpr auto DIALOG_SPACING = NAME_SPACING;
			constexpr auto PADDING = 16;

			auto idx = g_gs.current_dialog_dialog_idx;
			if (idx >= dia.size())
				idx = dia.size() - 1;

			auto &dialog = dia.at(idx);
			auto sz_name = MeasureTextEx(g_gs.font, dialog.name.c_str(), NAME_SIZE, NAME_SPACING);

			DrawTextEx(g_gs.font, dialog.name.c_str(), { PADDING, y - sz_name.y }, NAME_SIZE, NAME_SPACING, g_gs.palette.primary);
			DrawTextBoxed(g_gs.font, dialog.message.c_str(), { PADDING, y+PADDING / 2, g_gs.widthf - PADDING * 2, static_cast<float>(height)}, DIALOG_SIZE,
			    DIALOG_SPACING, true, g_gs.palette.primary);
		}

#ifdef _DEBUG
		DrawFPS(20, 20);
#endif
	}
	EndDrawing();
}

static void DrawTextBoxed(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    DrawTextBoxedSelectable(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}

// Draw text using font inside rectangle limits with support for text selection
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/(float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize)*scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width))
                {
                    textOffsetY += (font.baseSize)*scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec({ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, { rec.x + textOffsetX, rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}
