#include "GameState.h"

#include <raylib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

GameState g_gs {};

void GameState::render_texture(Vector2 position, int id, float angle, float size, Color tint) {
	DrawTexturePro(
		this->spritesheet,
		{
			.x = static_cast<float>(id * this->spritesheet.height),
			.y = 0,
			.width = static_cast<float>(this->spritesheet.height),
			.height = static_cast<float>(this->spritesheet.height),
		},
		{
			.x = position.x,
			.y = position.y,
			.width = size * 2,
			.height = size * 2,
		},
		{ size, size },
		angle,
		tint
	);
}

#include <iostream>

void GameState::deserialize_dialogs(json j) { 
	for (auto &[k, topLevel] : j.items()) {
		for (auto& secondLevel : topLevel.items()) {
			std::vector<GameState::Dialog> dias;
			auto dialogs_array = secondLevel.value().get<std::vector<json>>();
			for (auto& dialog_item : dialogs_array) {
				GameState::Dialog dia;
				dia.name = dialog_item["name"];
				dia.message = dialog_item["msg"];
				dias.push_back(dia);
			}
			this->dialogs[k].push_back(dias);
		}
	}
}
