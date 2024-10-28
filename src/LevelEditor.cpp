#include "LevelEditor.h"

#include <format>

#include "GameState.h"

void LevelEditor::init(i32 id, Level *level)
{
	m_path = std::format("level_{}.json", id);
	m_level = level;
}

void LevelEditor::update(void) { }

void LevelEditor::render_ui(void) { }

void LevelEditor::render_in_camera(void) { }

void LevelEditor::save(void) { m_level->export_to_file(m_path); }
