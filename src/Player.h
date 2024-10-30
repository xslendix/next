#pragma once

#include <raylib.h>

#include "Level.h"

constexpr auto PLAYER_RADIUS = 10;
constexpr auto PLAYER_SPEED = 10;
constexpr auto PLAYER_TURNING_SPEED = 3;
constexpr auto PLAYER_FRICTION = 0.98;
constexpr auto PLAYER_MAX_SPEED = 5;
constexpr auto BOUNCE_SLOWDOWN = 0.25;

struct Player {
	struct TrailPickup {
		Level::Pickup *ptr;
		Vector2        position;
		Vector2        direction;
	};

	void    render(void); // To be called inside a camera context.
	void    update(double dt);
	Vector2 get_next_trail_position(void);

	Vector2 position;
	Vector2 velocity;
	float   angle = -90 * DEG2RAD;

	std::vector<TrailPickup> trail;
};
