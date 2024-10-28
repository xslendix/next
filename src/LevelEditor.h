#pragma once

#include <raylib.h>

#include "Level.h"

struct LevelEditor {
	enum class Tool {
		Selection,
		Move,
		Creation,
	};

	enum class Mode {
		Wall,
		Zone,
		Pickup,
	};

	void init(i32 id, Level *level);

	void set_mode(Mode mode) { this->mode = mode; }
	void set_tool(Tool tool) { this->tool = tool; }

	void update(void);
	void render(Camera2D cam)
	{
		this->render_ui();
		BeginMode2D(cam);
		m_level->render();
		this->render_in_camera();
		EndMode2D();
	}

	bool const is_initialised() const { return m_level != nullptr; }

	Tool tool;
	Mode mode;

	Level::Wall   *selected_wall = nullptr;
	Level::Zone   *selected_zone = nullptr;
	Level::Pickup *selected_pickup = nullptr;

private:
	void render_ui(void);
	void render_in_camera(void);

	void save(void);

	std::string m_path;
	Level      *m_level;
};
