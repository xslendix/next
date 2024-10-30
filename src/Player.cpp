#include "Player.h"

#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "GameState.h"
#include "Level.h"

void Player::render(void)
{
	DrawPoly(this->position, 3, PLAYER_RADIUS, this->angle * RAD2DEG, GREEN);

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
}

Vector2 reflect(Vector2 const &v, Vector2 const &n)
{
	return Vector2Subtract(v, Vector2Scale(n, 2 * Vector2DotProduct(v, n)));
}

Vector2 Vector2Perpendicular(Vector2 const &v) { return { -v.y, v.x }; }

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
		for (auto const &wall : g_gs.level()->walls) {
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
				if (Vector2Length(Vector2Subtract(this->position, closest_point)) < radius) {
					Vector2 wall_normal = Vector2Normalize(Vector2Perpendicular(wall_dir));
					this->velocity
					    = Vector2Scale(reflect(this->velocity, wall_normal), BOUNCE_SLOWDOWN);
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

Vector2 Player::get_next_trail_position(void)
{
	if (this->trail.empty())
		return this->position;
	auto const &trail = this->trail.at(this->trail.size() - 1);
	return trail.position - Vector2Scale(trail.direction, 1);
}
