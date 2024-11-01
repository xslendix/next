#pragma once

#include "common.h"

#include <raylib.h>
#include <nlohmann/json.hpp>

#include "Color.h"
#include "Level.h"
#include "Player.h"

struct GameState {
	struct Dialog {
		std::string name;
		std::string message;
	};

	// Game logic
	std::vector<Level> levels;
	Player             player;
	f64                time_spent;
	f64                completion_time;

	std::vector<std::vector<Dialog>> *current_dialog = nullptr;
	// I'm sorry if you're reading this...
	usize current_dialog_idx, current_dialog_dialog_idx;
	f32 dialog_box_y;

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
	f32 target_menu_scroll = 0;

	std::map<std::string, std::vector<std::vector<Dialog>>> dialogs;

	std::vector<Vector2> menu_particles;
	std::vector<f32>   menu_particle_speeds;

	RenderTexture2D target {};
	Camera2D        camera {};
	Font            font;

	Texture2D spritesheet;

	Level *level()
	{
		if (!current_level)
			return nullptr;
		return &this->levels.at(*current_level);
	}

	void render_texture(Vector2 position, int id, float angle, float size, Color tint);
	void deserialize_dialogs(nlohmann::json j);

	void read_dialogs_from_file(std::filesystem::path path)
	{
		std::ifstream f(path);
		if (!f)
			throw std::runtime_error("Failed to open file for reading.");
		nlohmann::json j;
		f >> j;
		return deserialize_dialogs(j);
	}

	void show_dialog(std::string name, int idx) {
		auto &dialog = this->dialogs[name];
		this->current_dialog = &dialog;
		this->dialog_box_y = this->heightf;
		this->current_dialog_idx = idx;
		this->current_dialog_dialog_idx = 0;
	}
};

extern GameState g_gs;

static char const *format_time(f64 time)
{
	return TextFormat("%02d:%02d.%02d", static_cast<int>(time / 60) % 60,
	    static_cast<int>(time) % 60, static_cast<int>(time * 100) % 100);
}
