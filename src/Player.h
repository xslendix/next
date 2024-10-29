#pragma once

#include <raylib.h>

#include "Level.h"

constexpr auto PLAYER_RADIUS = 10;
constexpr auto PLAYER_SPEED = 10;
constexpr auto PLAYER_TURNING_SPEED = 3;
constexpr auto PLAYER_FRICTION = 0.98;
constexpr auto PLAYER_MAX_SPEED = 5;

struct Player {
	struct TrailPickup {
		Level::Pickup *ptr;
		Vector2 position;
	};

	void render(void); // To be called inside a camera context.
	void update(double dt);

	Vector2 position;
	Vector2 velocity;
	float angle = -90 * DEG2RAD;

	std::vector<TrailPickup> trail;
};

