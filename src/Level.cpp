#include "Level.h"

#include "GameState.h"
#include "Gui.h"

#include <raylib.h>

#include <polypartition.h>

#include <polypartition.h>

float CalculateSignedArea(std::vector<Vector2> const &points)
{
	float  area = 0.0f;
	size_t n = points.size();
	for (size_t i = 0; i < n; ++i) {
		Vector2 const &p1 = points[i];
		Vector2 const &p2 = points[(i + 1) % n];
		area += (p1.x * p2.y - p2.x * p1.y);
	}
	return area * 0.5f;
}

void EnsureCounterClockwise(std::vector<Vector2> &points)
{
	if (CalculateSignedArea(points) < 0) {
		std::reverse(points.begin(), points.end());
	}
}

std::vector<TPPLPoint> ConvertToTPPLPoints(std::vector<Vector2> const &points)
{
	std::vector<TPPLPoint> polyPoints(points.size());
	for (size_t i = 0; i < points.size(); ++i) {
		polyPoints[i].x = points[i].x;
		polyPoints[i].y = points[i].y;
	}
	return polyPoints;
}

float CalculateTriangleArea(Vector2 const &p0, Vector2 const &p1, Vector2 const &p2)
{
	return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

void DrawTriangleCCW(Vector2 p0, Vector2 p1, Vector2 p2, Color col)
{
	if (CalculateTriangleArea(p0, p1, p2) > 0) {
		std::swap(p1, p2);
	}

	DrawTriangle(p0, p1, p2, col);
}

void DrawPolygon(std::vector<Vector2> &points, Color col)
{
	EnsureCounterClockwise(points);

	if (points.size() < 3)
		return;

	std::vector<TPPLPoint> polyPoints = ConvertToTPPLPoints(points);

	TPPLPoly polygon;
	polygon.Init(polyPoints.size());
	for (size_t i = 0; i < polyPoints.size(); ++i) {
		polygon[i] = polyPoints[i];
	}

	std::list<TPPLPoly> triangles;
	TPPLPartition       partitioner;
	partitioner.Triangulate_EC(&polygon, &triangles);

	for (TPPLPoly const &triangle : triangles) {
		if (triangle.GetNumPoints() == 3) {
			Vector2 p0 = { (float)triangle[0].x, (float)triangle[0].y };
			Vector2 p1 = { (float)triangle[1].x, (float)triangle[1].y };
			Vector2 p2 = { (float)triangle[2].x, (float)triangle[2].y };

			DrawTriangleCCW(p0, p1, p2, col);
		}
	}
}

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

	return j;
}

Level Level::deserialize(nlohmann::json &data)
{
	Level level(data["name"], data["files_required"].get<u16>());
	level.author_time = data["author_time"];
	level.start_position.x = data["start_position"][0];
	level.start_position.y = data["start_position"][1];
	level.start_angle = data["start_angle"];
	if (!data["on_unlock_dialog"].is_null())
		level.on_unlock_dialog = data["on_unlock_dialog"];

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

	return level;
}

void Level::Pickup::render(Vector2 position, float radius, float theta) const
{
	if (!radius)
		return;
	Color pickup_color
	    = this->kind == Level::Pickup::Kind::Key ? g_gs.palette.key_door : g_gs.palette.file;
	int id;
	switch (this->kind) {
	case Kind::Key:
		id = 1;
		break;
	case Kind::File:
		id = 2;
		break;
	}
	g_gs.render_texture(position, id, theta, radius, pickup_color);
}

void Level::render(Camera2D *camera, bool origin, bool render_player)
{
	if (origin)
		DrawCircle(0, 0, 2, GREEN);

	BeginMode2D(*camera);
	{
		for (auto &zone : this->zones) {
			Color col;
			switch (zone.kind) {
			case Zone::Kind::End:
			case Zone::Kind::DialogTrigger:
				col = { 0, 0, 0, 0 };
				break;
			case Zone::Kind::OneWay:
				col = g_gs.palette.one_way_zone_background;
				break;
			case Zone::Kind::Danger:
				col = g_gs.palette.danger_zone_background;
				break;
			}
			DrawPolygon(zone.points, col);
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
			auto radius = PICKUP_RADIUS;
			if (pickup.time_since_pickup != -1) {
				if (pickup.time_since_pickup <= .3) {
					radius *= (.3 - pickup.time_since_pickup) / .3;
				} else {
					radius = 0;
				}
			}

			pickup.render(pickup.position, radius, 0);
		}

		if (render_player)
			g_gs.player.render();
	}
	EndMode2D();
}

float ease_out_lerp(float start, float end, float t)
{
	t = 1 - (1 - t) * (1 - t);
	return start + t * (end - start);
}

float limit(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

void Level::render_hud(f64 t)
{
	constexpr float WIDTH = 500;
	constexpr float HEIGHT = 320;

	float x = g_gs.widthf / 2 - WIDTH / 2;
	float y = ease_out_lerp(g_gs.heightf, g_gs.heightf / 2 - HEIGHT / 2, limit(t, 0, 1));

	DrawRectangle(x - BORDER_WIDTH, y - BORDER_WIDTH, WIDTH + BORDER_WIDTH * 2,
	    HEIGHT + BORDER_WIDTH * 2, g_gs.palette.primary);
	DrawRectangle(x, y, WIDTH, HEIGHT, g_gs.palette.menu_background);
	constexpr auto text = "DATA SENT";
	constexpr auto text_size = 40;

	float start_x = x + WIDTH / 2
	    - MeasureTextEx(g_gs.font, " ", text_size * 2, 0).x * 1.5 * (strlen(text) - 1) * .5 - 15;
	for (size_t i = 0; i < strlen(text); ++i) {
		float char_x = start_x + i * MeasureTextEx(g_gs.font, " ", text_size * 2, 0).x * 1.5;
		float bounce_offset = sin(t * 3.0f + i * 0.3f) * 10.0f - 15;
		DrawTextEx(g_gs.font, TextSubtext(text, i, 1), { char_x, y + text_size + bounce_offset },
		    text_size * 2, 0, g_gs.palette.primary);
	}

	constexpr auto PADDING = 20;
	constexpr auto FONT_SIZE = 30;
	constexpr auto FONT_SPACING = 2;
	float          off = y + text_size * 2 + PADDING * 2;
	if (t > .75) {
		auto time = std::string(format_time(g_gs.completion_time));
		DrawTextEx(g_gs.font, TextFormat("Completion time: %s", time.c_str()), { x + PADDING, off },
		    FONT_SIZE, FONT_SPACING, g_gs.palette.primary);
		off += FONT_SIZE * .75 + PADDING / 2;
	}
	if (t > 1) {
		DrawTextEx(g_gs.font,
		    TextFormat("Files collected: %d/%d", this->collected_files, this->total_files),
		    { x + PADDING, off }, FONT_SIZE, FONT_SPACING, g_gs.palette.primary);
		off += FONT_SIZE * .75 + PADDING / 2;
	}
	if (t > 1.25) {
		auto time = std::string(format_time(this->author_time));
		DrawTextEx(g_gs.font, TextFormat("Author completion time: %s", time.c_str()),
		    { x + PADDING, off }, FONT_SIZE, FONT_SPACING, g_gs.palette.primary);
		off += FONT_SIZE * .75 + PADDING / 2;
	}
	off += FONT_SIZE * .75 + PADDING / 2;
	if (t > 1.5) {
		if (GuiButton(
		        { x + PADDING, static_cast<float>(off), WIDTH - PADDING * 2, 50 }, "Back to map"))
			g_gs.current_level = {};
	}
}
