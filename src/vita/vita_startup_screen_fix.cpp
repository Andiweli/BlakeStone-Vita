/*
BStone Vita startup-screen compatibility workaround.

The original DemoLoop title cycle remains disabled on Vita because its combined
movie-mode, palette and title presentation path crashes in current SDL2/GXM.
PreDemo is invoked through bstone_vita_pre_demo(), which temporarily enables
only the original startup sequence (Apogee/JAM intro/PC-13) and then restores
the title-cycle bypass before entering DemoLoop.
*/

#include "../bstone_log.h"


extern bool g_no_screens;

void PreDemo();


void bstone_vita_pre_demo()
{
	const auto old_no_screens = ::g_no_screens;

	bstone::Log::write("VITA: Starting original intro sequence...");

	// PreDemo still honours the user's NO INTRO/OUTRO setting. Only the
	// internal title-cycle bypass is temporarily lifted here.
	::g_no_screens = false;
	::PreDemo();
	::g_no_screens = old_no_screens;

	bstone::Log::write("VITA: Original intro sequence finished.");
}


namespace
{

struct VitaStartupScreenFix
{
	VitaStartupScreenFix()
	{
		// Keep the crash-prone original DemoLoop title/credits/high-score cycle
		// disabled. vita_title_screen.cpp supplies the safe replacement title.
		::g_no_screens = true;
	}
};

VitaStartupScreenFix vita_startup_screen_fix;

} // namespace
