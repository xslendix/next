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
		trailer.ptr->render(trailer.position, radius, atan2(trailer.direction.y, trailer.direction.x) * RAD2DEG);
	}

	g_gs.render_texture(this->position, 0, this->angle * RAD2DEG + 90, PLAYER_RADIUS, g_gs.palette.primary);
}

void Player::update(double dt)
{
	{ // Player controller
		constexpr auto PLAYER_VELOCITY_ADDITION = PLAYER_SPEED;

		if (!g_gs.completion_time) {
			if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
				this->velocity.x += std::cos(this->angle) * PLAYER_VELOCITY_ADDITION * dt;
				this->velocity.y += std::sin(this->angle) * PLAYER_VELOCITY_ADDITION * dt;
			}
			if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
				this->velocity.x += std::cos(this->angle) * -PLAYER_VELOCITY_ADDITION * dt;
				this->velocity.y += std::sin(this->angle) * -PLAYER_VELOCITY_ADDITION * dt;
			}
			if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
				this->angle -= PLAYER_TURNING_SPEED * dt;
			}
			if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
				this->angle += PLAYER_TURNING_SPEED * dt;
			}
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

		this->position = Vector2Add(this->position, Vector2Scale(this->velocity, dt));

		float friction_factor = std::pow(PLAYER_FRICTION, dt);
		this->velocity.x *= friction_factor;
		this->velocity.y *= friction_factor;
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

				float distance = Vector2Length(Vector2Subtract(this->position, closest_point));
				float radius = PLAYER_RADIUS + (WALL_THICKNESS / 2);
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

					if (Vector2DotProduct(
					        Vector2Subtract(this->position, closest_point), wall_normal)
					    < 0) {
						wall_normal = Vector2Negate(wall_normal);
					}

					float angle_cos
					    = Vector2DotProduct(Vector2Normalize(this->velocity), wall_normal);
					float angle_degrees = std::acos(angle_cos) * RAD2DEG;

					float initial_speed = Vector2Length(this->velocity);

					constexpr float BOUNCE_ANGLE_THRESHOLD = 20.0f;
					if (angle_degrees > BOUNCE_ANGLE_THRESHOLD) {
						constexpr float bounce_factor = BOUNCE_SLOWDOWN;
						this->velocity = Vector2Scale(
						    Vector2Reflect(this->velocity, wall_normal), bounce_factor);
					} else {
						Vector2 wall_tangent = Vector2Normalize(wall_dir);
						this->velocity = Vector2Scale(wall_tangent, initial_speed);
					}

					float correction_offset = radius + 0.15f;
					this->position
					    = Vector2Add(closest_point, Vector2Scale(wall_normal, correction_offset));
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
