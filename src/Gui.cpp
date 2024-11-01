#include "Gui.h"

#include "GameState.h"

#include <cmath>

bool GuiButton(Rectangle rect, std::string text)
{
	DrawRectangleRec(rect, g_gs.palette.primary);
	DrawRectangle(rect.x + BORDER_WIDTH, rect.y + BORDER_WIDTH, rect.width - BORDER_WIDTH * 2,
	    rect.height - BORDER_WIDTH * 2, g_gs.palette.menu_background);

	auto const size = round((rect.height - BORDER_WIDTH * 2) * .75);

	auto w = MeasureTextEx(g_gs.font, text.c_str(), size, size * 0.02).x;
	DrawTextEx(g_gs.font, text.c_str(),
	    { rect.x + rect.width / 2 - w / 2,
	        static_cast<float>(rect.y + rect.height / 2 - size / 2) },
	    size, size * 0.02, g_gs.palette.primary);

	return CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(0);
}
