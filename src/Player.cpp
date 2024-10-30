#include "Player.h"

#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "GameState.h"

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
