#pragma once

#include "render.h"
#include <vector>
#include "../valve_sdk/math/Vector2D.h"

class Dot;
extern std::vector<Dot*> Dots;

class Dot
{
public:
	Dot(Vector2D p, Vector2D v)
	{
		relative_position = p;
		velocity = v;
	}
	void draw()
	{
		render::draw_boxfilled(relative_position.x - 2, relative_position.y - 2, relative_position.x + 2, relative_position.y + 2, Color(255, 255, 255, 255));
		auto t = std::find(Dots.begin(), Dots.end(), this);
		if (t == Dots.end()) return;
		for (auto i = t; i != Dots.end(); i++)
		{
			if ((*i) == this) continue;
			Vector2D Pos = relative_position;
			float Dist = Pos.DistTo((*i)->relative_position);
			if (Dist < 200)
			{
				Vector2D Dir = ((*i)->relative_position - relative_position).Normalized();
				render::draw_line(Pos.x, Pos.y, (Pos + Dir * Dist).x, (Pos + Dir * Dist).y, settings::misc::menu_color);
			}
		}
	}
	Vector2D relative_position = Vector2D(0, 0);
	Vector2D velocity;
};

namespace backdrop
{
	void render();
}