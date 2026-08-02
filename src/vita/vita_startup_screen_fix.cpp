/*
BStone Vita startup-screen compatibility workaround.

Current VitaSDK SDL2/GXM builds crash in the common startup/title-screen path
before either game's title image becomes visible. Until that renderer path is
fully symbolized, skip only the optional startup screens on Vita so both games
can proceed directly to the menu. Gameplay, menus, saves and game data loading
remain unchanged.
*/


extern bool g_no_screens;


namespace
{

struct VitaStartupScreenFix
{
	VitaStartupScreenFix()
	{
		g_no_screens = true;
	}
};

VitaStartupScreenFix vita_startup_screen_fix;

} // namespace
