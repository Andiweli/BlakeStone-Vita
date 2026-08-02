/*
BStone Vita startup-screen bypass.

The original startup movie leaves current Vita SDL2/GXM builds in a state that
aborts when the following title screen is presented. Skip the optional intro
sequence and the original rotating demo/title loop. vita_title_screen.cpp shows
one stable title screen immediately before the first menu instead.
*/

extern bool g_no_screens;

namespace
{

struct VitaStartupScreenBypass
{
	VitaStartupScreenBypass()
	{
		g_no_screens = true;
	}
};

VitaStartupScreenBypass vita_startup_screen_bypass;

} // namespace
