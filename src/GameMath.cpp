#include "GameMath.h"

#include <raylib.h>
#include <raymath.h>

Vector2 Vector2Perpendicular(Vector2 const &v) { return { -v.y, v.x }; }

Vector2 ClosestPointOnSegment(Vector2 p, Vector2 a, Vector2 b)
{
	Vector2 ab = { b.x - a.x, b.y - a.y };
	float   t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / (ab.x * ab.x + ab.y * ab.y);
	t = fmax(0, fmin(1, t));
	return { a.x + t * ab.x, a.y + t * ab.y };
}

bool CheckCollisionPointPoly(Vector2 p, std::vector<Vector2> const &poly)
{
	auto px = p.x;
	auto py = p.y;
	bool collision = false;
	int  next = 0;
	for (int current = 0; current < poly.size(); current++) {
		next = current + 1;
		if (next == poly.size())
			next = 0;

		Vector2 vc = poly[current];
		Vector2 vn = poly[next];

		if (((vc.y > py && vn.y < py) || (vc.y < py && vn.y > py))
		    && (px < (vn.x - vc.x) * (py - vc.y) / (vn.y - vc.y) + vc.x)) {
			collision = !collision;
		}
	}
	return collision;
}

bool CheckCollisionCirclePoly(Vector2 p, float r, std::vector<Vector2> const &poly, bool inside)
{
	if (inside && CheckCollisionPointPoly(p, poly))
		return true;

	for (auto const &vertex : poly) {
		if (CheckCollisionPointCircle(vertex, p, r)) {
			return true;
		}
	}

	for (size_t i = 0; i < poly.size(); i++) {
		Vector2 a = poly[i];
		Vector2 b = poly[(i + 1) % poly.size()];
		Vector2 closest = ClosestPointOnSegment(p, a, b);
		if (CheckCollisionPointCircle(closest, p, r)) {
			return true;
		}
	}
	return false;
}
