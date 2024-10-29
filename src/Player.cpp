#include "Player.h"

#include "GameState.h"

#include <cmath>

#include <raylib.h>
#include <raymath.h>

void Player::render(void) {
	DrawPoly(this->position, 3, PLAYER_RADIUS, this->angle * RAD2DEG, GREEN);
}

void Player::update(double dt) {
	constexpr auto PLAYER_VELOCITY_ADDITION = PLAYER_SPEED + PLAYER_SPEED * (1 - PLAYER_FRICTION);

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

    float vel_norm = std::sqrt((this->velocity.x*this->velocity.x) + (this->velocity.y*this->velocity.y));
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
