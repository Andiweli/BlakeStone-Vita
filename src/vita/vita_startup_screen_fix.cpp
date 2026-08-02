/*
BStone Vita startup-screen compatibility setup.

The movie parser now reads packed animation structures through aligned local
copies, so the original intro/title flow can be used again on PS Vita.
*/

extern bool g_no_screens;

namespace
{

struct VitaStartupScreenSetup
{
	VitaStartupScreenSetup()
	{
		g_no_screens = false;
	}
};

VitaStartupScreenSetup vita_startup_screen_setup;

} // namespace
