#include "Level.h"

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

	for (auto const &pickup : pickups) {
		json pickupj;
		pickupj["kind"] = pickup.kind;
		pickupj["points"] = json::array();
		pickupj["x"] = pickup.position.x;
		pickupj["y"] = pickup.position.y;
		pickupj["id"] = pickup.id;
		j["pickups"].push_back(pickupj);
	}

	return j;
}

Level Level::deserialize(nlohmann::json &data)
{
	Level level(data["name"], data["files_required"].get<u16>());

	for (auto &wallj : data["walls"]) {
		Wall wall;
		wall.kind = wallj["kind"];
		for (auto &pointj : wallj["points"]) {
			Vector2 point;
			point.x = pointj[0];
			point.y = pointj[1];
			wall.points.push_back(point);
		}
		level.walls.push_back(wall);
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

	return level;
}
