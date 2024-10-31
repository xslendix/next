#include "Player.h"

#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "GameMath.h"
#include "GameState.h"
#include "Level.h"

void Player::render(void)
{
	for (auto const &trailer : this->trail) {
		auto const &pickup = *trailer.ptr;
		float       radius = PICKUP_RADIUS;
		if (pickup.time_since_pickup != -1) {
			if (pickup.time_since_pickup <= .3) {
				radius *= pickup.time_since_pickup / .3;
			}
		}
		DrawCircleV(trailer.position, radius, RED);
	}

	DrawPoly(this->position, 3, PLAYER_RADIUS, this->angle * RAD2DEG, GREEN);
}

void Player::update(double dt)
{
	{ // Player controller
		constexpr auto PLAYER_VELOCITY_ADDITION
		    = PLAYER_SPEED + PLAYER_SPEED * (1 - PLAYER_FRICTION);

		if (IsKeyDown(KEY_UP)) {
			this->velocity.x += std::cos(this->angle) * PLAYER_VELOCITY_ADDITION * dt;
			this->velocity.y += std::sin(this->angle) * PLAYER_VELOCITY_ADDITION * dt;
		}
		if (IsKeyDown(KEY_DOWN)) {
			this->velocity.x += std::cos(this->angle) * -PLAYER_VELOCITY_ADDITION * dt;
			this->velocity.y += std::sin(this->angle) * -PLAYER_VELOCITY_ADDITION * dt;
		}
		if (IsKeyDown(KEY_LEFT)) {
			this->angle -= PLAYER_TURNING_SPEED * dt;
		}
		if (IsKeyDown(KEY_RIGHT)) {
			this->angle += PLAYER_TURNING_SPEED * dt;
		}

		for (auto const &zone : g_gs.level()->zones) {
			if (zone.kind != Level::Zone::Kind::OneWay)
				continue;

			if (CheckCollisionCirclePoly(g_gs.player.position, PLAYER_RADIUS, zone.points)) {
				this->velocity.x += std::cos(zone.value.one_way_angle) * PLAYER_VELOCITY_ADDITION
				    * zone.power * dt;
				this->velocity.y += std::sin(zone.value.one_way_angle) * PLAYER_VELOCITY_ADDITION
				    * zone.power * dt;
			}
		}

		float vel_norm = std::sqrt(
		    (this->velocity.x * this->velocity.x) + (this->velocity.y * this->velocity.y));
		if (vel_norm > PLAYER_MAX_SPEED) {
			vel_norm = PLAYER_MAX_SPEED;
			auto vel_normalized = Vector2Normalize(this->velocity);
			this->velocity = {
				vel_normalized.x * vel_norm,
				vel_normalized.y * vel_norm,
			};
		}

		this->position = Vector2Add(this->position, this->velocity);
		this->velocity.x *= PLAYER_FRICTION;
		this->velocity.y *= PLAYER_FRICTION;
	}

	{ // Collision detection and response
		for (auto &wall : g_gs.level()->walls) {
			if (wall.time_since_trigger != -1)
				wall.time_since_trigger += dt;

			for (size_t i = 0; i < wall.points.size() - 1; ++i) {
				Vector2 wall_start = wall.points.at(i);
				Vector2 wall_end = wall.points.at(i + 1);
				Vector2 wall_dir = Vector2Subtract(wall_end, wall_start);

				Vector2 to_player = Vector2Subtract(this->position, wall_start);
				float   t = Vector2DotProduct(to_player, wall_dir)
				    / Vector2DotProduct(wall_dir, wall_dir);
				t = std::clamp(t, 0.0f, 1.0f);
				Vector2 closest_point = Vector2Add(wall_start, Vector2Scale(wall_dir, t));

				float radius = PLAYER_RADIUS;
				float distance = Vector2Length(Vector2Subtract(this->position, closest_point));
				if (distance < radius) {
					if (wall.kind == Level::Wall::Kind::Door) {
						auto should_cont = false;
						for (usize i = 0; i < this->trail.size(); i++) {
							auto &item = this->trail[i];
							if (item.ptr->id == wall.key_id) {
								should_cont = true;
								wall.time_since_trigger = 0;
								trail_remove(i);
							}
						}
						if (should_cont || wall.time_since_trigger != -1)
							continue;
					}

					Vector2 wall_normal = Vector2Normalize(Vector2Perpendicular(wall_dir));

					// Determine which side of the wall the player is on
					if (Vector2DotProduct(
					        Vector2Subtract(this->position, closest_point), wall_normal)
					    < 0) {
						wall_normal = Vector2Negate(wall_normal);
					}

					this->velocity = Vector2Scale(
					    Vector2Reflect(this->velocity, wall_normal), BOUNCE_SLOWDOWN);
					this->position = Vector2Add(closest_point, Vector2Scale(wall_normal, radius));
				}
			}
		}
	}

	{ // Trail
		constexpr float target_dist = 20.0f;
		constexpr float stiffness = 0.75f;
		constexpr float damping = 0.9f;

		Vector2 target_pos = this->position;

		for (auto &trailer : this->trail) {
			Vector2 diff = Vector2Subtract(target_pos, trailer.position);
			float   distance = Vector2Length(diff);
			Vector2 spring_force
			    = Vector2Scale(Vector2Normalize(diff), (distance - target_dist) * stiffness);
			trailer.direction = Vector2Scale(Vector2Add(trailer.direction, spring_force), damping);
			trailer.position = Vector2Add(trailer.position, Vector2Scale(trailer.direction, dt));
			target_pos = trailer.position;
		}
	}
}

void Player::trail_remove(usize i) { this->trail.erase(this->trail.begin() + i); }

Vector2 Player::get_next_trail_position(void)
{
	if (this->trail.empty())
		return this->position;
	auto const &trail = this->trail.at(this->trail.size() - 1);
	return trail.position - Vector2Scale(trail.direction, 1);
}
