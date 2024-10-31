#include "Level.h"

#include "GameState.h"

using json = nlohmann::json;

Level::Level(std::string name, u16 files_required)
{
	this->name = name;
	this->files_required = files_required;
}

json Level::serialize(void)
{
	json j;
	j["name"] = this->name;
	j["files_required"] = this->files_required;
	j["start_position"] = json::array();
	j["start_position"].push_back(this->start_position.x);
	j["start_position"].push_back(this->start_position.y);
	j["start_angle"] = this->start_angle;
	j["walls"] = json::array();
	for (auto const &wall : walls) {
		json wallj;
		wallj["kind"] = wall.kind;
		wallj["points"] = json::array();
		for (auto const &point : wall.points) {
			json pointj;
			pointj.push_back(point.x);
			pointj.push_back(point.y);
			wallj["points"].push_back(pointj);
		}
		j["walls"].push_back(wallj);
	}

	j["zones"] = json::array();
	for (auto const &zone : zones) {
		json zonej;
		zonej["kind"] = zone.kind;
		zonej["points"] = json::array();
		for (auto const &point : zone.points) {
			json pointj;
			pointj.push_back(point.x);
			pointj.push_back(point.y);
			zonej["points"].push_back(pointj);
		}
		switch (zone.kind) {
		case Zone::Kind::End:
			break;
		case Zone::Kind::DialogTrigger:
			zonej["value"] = zone.value.dialog_index;
			break;
		case Zone::Kind::OneWay:
			zonej["value"] = zone.value.one_way_angle;
			break;
		case Zone::Kind::Danger:
			break;
		default:
			unreachable();
		}
		j["zones"].push_back(zonej);
	}

	for (auto const &pickup : this->pickups) {
		json pickupj;
		pickupj["kind"] = pickup.kind;
		pickupj["x"] = pickup.position.x;
		pickupj["y"] = pickup.position.y;
		pickupj["id"] = pickup.id;
		j["pickups"].push_back(pickupj);
	}

	for (auto const &dialog : this->dialogs) {
		json dialog_arr = json::array();
		for (auto const &dialog : dialog) {
			json dialogj;
			dialogj["name"] = dialog.name;
			dialogj["message"] = dialog.message;
			dialog_arr.push_back(dialogj);
		}
		j["dialogs"].push_back(dialog_arr);
	}

	return j;
}

Level Level::deserialize(nlohmann::json &data)
{
	Level level(data["name"], data["files_required"].get<u16>());
	level.start_position.x = data["start_position"][0];
	level.start_position.y = data["start_position"][1];
	level.start_angle = data["start_angle"];

	for (auto &wallj : data["walls"]) {
		Wall wall;
		wall.kind = wallj["kind"];
		for (auto &pointj : wallj["points"]) {
			Vector2 point;
			point.x = pointj[0];
			point.y = pointj[1];
			wall.points.push_back(point);
		}
		if (wall.kind == Wall::Kind::Door) {
			wall.key_id = wallj["key_id"];
			level.walls.emplace(level.walls.begin(), wall);
		} else {
			level.walls.push_back(wall);
		}
	}

	for (auto &zonej : data["zones"]) {
		Zone zone;
		zone.kind = static_cast<Zone::Kind>(zonej["kind"].get<int>());
		for (auto &pointj : zonej["points"]) {
			Vector2 point;
			point.x = pointj[0];
			point.y = pointj[1];
			zone.points.push_back(point);
		}
		switch (zone.kind) {
		case Zone::Kind::End:
			break;
		case Zone::Kind::DialogTrigger:
			zone.value.dialog_index = zonej["value"];
			break;
		case Zone::Kind::OneWay:
			zone.value.one_way_angle = zonej["value"];
			zone.power = zonej["power"];
			break;
		case Zone::Kind::Danger:
			break;
		default:
			unreachable();
		}
		level.zones.push_back(zone);
	}

	for (auto &pickupj : data["pickups"]) {
		Pickup pickup;
		pickup.kind = pickupj["kind"];
		pickup.position.x = pickupj["x"];
		pickup.position.y = pickupj["y"];
		pickup.id = pickupj["id"];
		level.pickups.push_back(pickup);
	}

	for (auto &dialogj : data["dialogs"]) {
		std::vector<Dialog> dialogs;
		for (auto &dialog_entryj : dialogj) {
			Dialog dialog;
			dialog.name = dialog_entryj["name"];
			dialog.message = dialog_entryj["message"];
			dialogs.push_back(dialog);
		}
		level.dialogs.push_back(dialogs);
	}

	return level;
}

void Level::Pickup::render(Vector2 position, float radius) const {
	if (!radius)
		return;
	Color pickup_color = this->kind == Level::Pickup::Kind::Key ? g_gs.palette.key_door
																 : g_gs.palette.file;
	DrawCircleV(position, radius, pickup_color);
}

void Level::render(Camera2D *camera, bool origin, bool render_player)
{
	if (origin)
		DrawCircle(0, 0, 2, GREEN);

	BeginMode2D(*camera);
	{
		for (auto const &zone : this->zones) {
			Color col;
			switch (zone.kind) {
			case Zone::Kind::End:
			case Zone::Kind::DialogTrigger:
				col = { 0, 0, 0, 0 };
#ifdef _DEBUG
				col = DARKGREEN;
#endif
				break;
			case Zone::Kind::OneWay:
				col = g_gs.palette.one_way_zone_background;
				break;
			case Zone::Kind::Danger:
				col = g_gs.palette.danger_zone_background;
				break;
			}
			DrawTriangleFan(zone.points.data(), zone.points.size(), col);
		}

		for (auto const &wall : this->walls) {
			if (wall.time_since_trigger != -1)
				continue;

			Color wall_color
			    = wall.kind == Level::Wall::Kind::Wall ? g_gs.palette.wall : g_gs.palette.key_door;
			for (usize i = 0; i < wall.points.size() - 1; i++) {
				auto first = wall.points.at(i);
				auto second = wall.points.at(i + 1);
				DrawLineEx(first, second, WALL_THICKNESS, wall_color);
				DrawCircleV(first, WALL_THICKNESS / 2, wall_color);
				DrawCircleV(second, WALL_THICKNESS / 2, wall_color);
			}
		}

		for (auto const &pickup : this->pickups) {
			auto  radius = PICKUP_RADIUS;
			if (pickup.time_since_pickup != -1) {
				if (pickup.time_since_pickup <= .3) {
					radius *= (.3 - pickup.time_since_pickup) / .3;
				} else {
					radius = 0;
				}
			}

			pickup.render(pickup.position, radius);
		}

		if (render_player)
			g_gs.player.render();
	}
	EndMode2D();
}
