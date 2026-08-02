/*
BStone Vita safe title-screen presentation.

The original DemoLoop title path combines movie-mode rendering, a dedicated
palette, version text and fades. Current Vita SDL2/GXM builds abort in that
combined path before the title becomes visible. The existing startup bypass
keeps that path disabled; this wrapper restores the title image safely before
the first main menu by using the already stable menu renderer and VGA palette.
*/

#include <cstdint>

#include "../3d_def.h"
#include "../bstone_log.h"
#include "../id_heads.h"
#include "../id_in.h"
#include "../id_vh.h"
#include "../id_vl.h"


extern std::int16_t TITLEPIC;
extern std::int16_t TITLE1PIC;

void CA_CacheScreen(std::int16_t chunk);
void US_ControlPanel(ScanCode scancode);


namespace
{

void show_safe_title_screen_once()
{
	static bool was_shown = false;

	if (was_shown)
	{
		return;
	}

	was_shown = true;

	const auto& assets_info = AssetsInfo{};
	const auto title_chunk = assets_info.is_aog() ? TITLEPIC : TITLE1PIC;
	const auto old_movie_state = ::vid_is_movie;

	bstone::Log::write("VITA: Showing safe title screen...");

	// Keep the stable menu rendering mode. The original movie-mode/palette
	// combination remains bypassed until it can be repaired independently.
	::vid_is_movie = false;
	::IN_ClearKeysDown();

	bstone::Log::write("VITA: Caching title screen...");
	::CA_CacheScreen(title_chunk);
	bstone::Log::write("VITA: Title screen cached.");

	::VW_UpdateScreen();
	bstone::Log::write("VITA: Title screen presented.");

	if (::screenfaded)
	{
		::VW_FadeIn();
	}
	else
	{
		::VL_RefreshScreen();
	}

	bstone::Log::write("VITA: Waiting on title screen...");
	static_cast<void>(::IN_UserInput(TickBase * 6));

	::VW_FadeOut();
	::IN_ClearKeysDown();
	::vid_is_movie = old_movie_state;

	bstone::Log::write("VITA: Leaving safe title screen.");
}

} // namespace


void bstone_vita_control_panel(ScanCode scancode)
{
	show_safe_title_screen_once();
	::US_ControlPanel(scancode);
}
