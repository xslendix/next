#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <raylib.h>

#include "common.h"

constexpr auto WALL_THICKNESS = 8;
constexpr auto PICKUP_RADIUS = 10;

struct Level {
	struct Wall {
		enum class Kind {
			Wall,
			Door,
		};

		Kind                 kind;
		std::vector<Vector2> points;
		u8                   key_id;
	};

	struct Zone {
		enum class Kind {
			End,
			DialogTrigger,
			OneWay,
			Danger,
		};

		Kind                 kind;
		std::vector<Vector2> points;

		union {
			i32 dialog_index;
			f32 one_way_angle;
		} value;
	};

	struct Pickup {
		enum class Kind {
			Key,
			File,
		};

		Kind    kind;
		Vector2 position;
		i32     id = 0;

		f64 time_since_pickup = -1; // If >=0, it's picked up.
	};

	Level() = delete;
	Level(std::string name, u16 files_required);

	nlohmann::json serialize(void);
	static Level   deserialize(nlohmann::json &data);

	void export_to_file(std::filesystem::path path)
	{
		std::ofstream f(path);
		if (!f)
			throw std::runtime_error("Failed to open file for writing.");
		f << serialize().dump();
	}
	static Level read_from_file(std::filesystem::path path)
	{
		std::ifstream f(path);
		if (!f)
			throw std::runtime_error("Failed to open file for reading.");
		nlohmann::json j;
		f >> j;
		return deserialize(j);
	}

	void render(Camera2D *camera, bool origin = false, bool render_player = true);

	std::string name;
	u16         files_required;
	Vector2     start_position;
	float       start_angle;

	std::vector<Wall>   walls;
	std::vector<Zone>   zones;
	std::vector<Pickup> pickups;
};
