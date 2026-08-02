/*
BStone Vita front-touch compatibility.

Older versions of the port assumed that SDL reported the front panel as
TouchID 0. Current Vita SDL2 registers the front panel as TouchID 1 and the
rear panel as TouchID 2. Accept both historical front IDs and ignore the rear
panel.
*/

#include "SDL.h"
#include "../3d_def.h"


extern bool vid_is_ui_stretched;
extern gametype gamestate;
extern int lastautopress;

int GetPrevWeaponNum();
int GetNextWeaponNum();
void DepressGivenKey(int whichkey, int upordown);


namespace
{

void set_auto_weapon_key(const bool next_weapon, const int pressed)
{
	if (lastautopress == 0 && pressed)
	{
		lastautopress = next_weapon ? GetNextWeaponNum() : GetPrevWeaponNum();
		DepressGivenKey(lastautopress, 1);
	}
	else if (!pressed)
	{
		DepressGivenKey(lastautopress, 0);
		lastautopress = 0;
	}
}

} // namespace


void TranslateTouchEvent(SDL_Event* event)
{
	if (!event)
	{
		return;
	}

	// Vita SDL2: front = 1, rear = 2. TouchID 0 is accepted for
	// compatibility with older SDL2 builds used by the original port.
	if (event->tfinger.touchId != 0 && event->tfinger.touchId != 1)
	{
		return;
	}

	const auto pressed = event->type == SDL_FINGERDOWN ? 1 : 0;
	auto screen_width = 960.0F;
	const auto column_midpoint = 760.0F;
	const auto screen_height = 544.0F;
	auto finger_x = event->tfinger.x;
	const auto finger_y = event->tfinger.y;

	if (::vid_is_ui_stretched)
	{
		screen_width *= 0.75F;
		finger_x += 0.166667F;
	}

	// Elevator/weapon number column.
	if (finger_x > 660.0F / screen_width && finger_x < 860.0F / screen_width)
	{
		if (finger_y > 50.0F / screen_height && finger_y <= 139.0F / screen_height)
		{
			DepressGivenKey(finger_x < column_midpoint / screen_width ? 9 : 0, pressed);
		}
		else if (finger_y > 139.0F / screen_height && finger_y <= 194.0F / screen_height)
		{
			DepressGivenKey(finger_x < column_midpoint / screen_width ? 7 : 8, pressed);
		}
		else if (finger_y > 194.0F / screen_height && finger_y <= 249.0F / screen_height)
		{
			DepressGivenKey(finger_x < column_midpoint / screen_width ? 5 : 6, pressed);
		}
		else if (finger_y > 249.0F / screen_height && finger_y <= 304.0F / screen_height)
		{
			DepressGivenKey(finger_x < column_midpoint / screen_width ? 3 : 4, pressed);
		}
		else if (finger_y > 304.0F / screen_height && finger_y <= 410.0F / screen_height)
		{
			DepressGivenKey(finger_x < column_midpoint / screen_width ? 1 : 2, pressed);
		}
	}

	// Bottom HUD halves control map zoom in Planet Strike.
	if (finger_y > 410.0F / screen_height)
	{
		DepressGivenKey(finger_x > 480.0F / screen_width ? 11 : 12, pressed);
	}

	// Top-left/top-right choose previous/next available weapon.
	if (finger_y < 50.0F / screen_height)
	{
		set_auto_weapon_key(finger_x > 480.0F / screen_width, pressed);
	}
}
