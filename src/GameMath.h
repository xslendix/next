#pragma once

#include <vector>

#include <raylib.h>

Vector2 Vector2Perpendicular(Vector2 const &v);
Vector2 ClosestPointOnSegment(Vector2 p, Vector2 a, Vector2 b);
bool    CheckCollisionCirclePoly(
       Vector2 p, float r, std::vector<Vector2> const &poly, bool inside = true);
