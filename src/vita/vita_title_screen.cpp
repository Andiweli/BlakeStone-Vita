/*
BStone Vita safe title-screen presentation.

The original DemoLoop title path combines movie-mode rendering, a dedicated
palette, version text and fades. Current Vita SDL2/GXM builds abort in that
combined path before the title becomes visible. The existing startup bypass
keeps that path disabled; this wrapper restores the title image safely before
the first main menu using the stable menu renderer and the game's TITLEPALETTE.
*/

#include <cstdint>

#include "../3d_def.h"
#include "../bstone_log.h"
#include "../id_ca.h"
#include "../id_heads.h"
#include "../id_in.h"
#include "../id_vh.h"
#include "../id_vl.h"


extern std::int16_t TITLEPIC;
extern std::int16_t TITLE1PIC;
extern std::int16_t TITLEPALETTE;

void CA_CacheScreen(std::int16_t chunk);
void VH_UpdateScreen();
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

	// Aliens of Gold initializes TITLEPIC, while Planet Strike initializes
	// TITLE1PIC.
	const auto title_chunk = TITLEPIC != 0 ? TITLEPIC : TITLE1PIC;
	const auto old_movie_state = ::vid_is_movie;

	bstone::Log::write("VITA: Showing safe title screen...");

	// Keep the stable menu rendering mode. The original movie-mode title path
	// remains bypassed until it can be repaired independently.
	::vid_is_movie = false;
	::IN_ClearKeysDown();

	bstone::Log::write("VITA: Caching title screen and palette...");
	::CA_CacheScreen(title_chunk);
	::CA_CacheGrChunk(TITLEPALETTE);

	const auto* const title_palette = static_cast<const std::uint8_t*>(
		::grsegs[TITLEPALETTE]);

	// Apply the correct game palette directly. Do not combine the title palette
	// with VL_SetPaletteIntensity/VL_FadeIn on Vita: that reproduces the GXM
	// abort seen immediately after the startup intro.
	::VL_SetPalette(0, 256, title_palette);

	bstone::Log::write("VITA: Title screen and palette cached.");

	VW_UpdateScreen();
	::VL_RefreshScreen();
	bstone::Log::write("VITA: Title screen presented without palette fade.");

	::UNCACHEGRCHUNK(TITLEPALETTE);

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
