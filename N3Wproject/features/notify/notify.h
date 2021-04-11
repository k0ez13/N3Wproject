#pragma once
#include <string>
#include <deque>
#include "../../render/render.h"
#include "../../valve_sdk/sdk.h"
#include "../../options/options.h"

namespace notify
{
	void notify_events(i_gameevent* event);
	void render();

	namespace console
	{
		void clear();

		void notify(std::string pre, std::string msg, Color clr_pre = settings::misc::menu_color, Color clr_msg = Color(255, 255, 255, 255));
	}
	namespace screen
	{
		void notify(std::string pre, std::string text, Color color_pre = settings::misc::menu_color, Color color_text = Color(255, 255, 255, 255), int life_time = 4700);
	}
}