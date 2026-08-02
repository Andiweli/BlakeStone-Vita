/*
BStone Vita direct title-screen presentation.

This is retained as a fallback for configurations that explicitly skip the
original startup screens. With the aligned movie parser active, the default
Vita path uses the original intro and title sequence instead.
*/

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include "../3d_def.h"
#include "../bstone_log.h"
#include "../id_ca.h"
#include "../id_heads.h"
#include "../id_in.h"
#include "../id_vh.h"
#include "../id_vl.h"

extern bool g_no_screens;
extern std::int16_t TITLEPIC;
extern std::int16_t TITLE1PIC;
extern std::int16_t TITLEPALETTE;

void CA_CacheScreen(std::int16_t chunk);
void VH_UpdateScreen();
void US_ControlPanel(ScanCode scancode);

namespace
{

constexpr int palette_color_count = 256;
constexpr int palette_channel_count = 3;
constexpr int palette_byte_count = palette_color_count * palette_channel_count;

using Palette = std::array<std::uint8_t, palette_byte_count>;

void show_direct_title_screen_once()
{
	// The direct title is only a fallback when the original startup/title
	// sequence is intentionally disabled.
	if (!::g_no_screens)
	{
		return;
	}

	static bool was_shown = false;

	if (was_shown)
	{
		return;
	}

	was_shown = true;

	const auto& assets_info = AssetsInfo{};
	const auto title_chunk = assets_info.is_aog() ? TITLEPIC : TITLE1PIC;
	const auto old_movie_state = ::vid_is_movie;

	Palette previous_palette{};
	Palette title_palette{};

	::VL_GetPalette(0, palette_color_count, previous_palette.data());

	bstone::Log::write("VITA: Loading direct title screen...");

	::CA_CacheGrChunk(TITLEPALETTE);

	const auto* const cached_title_palette = static_cast<const std::uint8_t*>(
		::grsegs[TITLEPALETTE]);

	if (cached_title_palette)
	{
		std::memcpy(
			title_palette.data(),
			cached_title_palette,
			title_palette.size());
	}
	else
	{
		title_palette = previous_palette;
	}

	::UNCACHEGRCHUNK(TITLEPALETTE);

	::vid_is_movie = false;
	::IN_ClearKeysDown();
	::CA_CacheScreen(title_chunk);
	::VL_SetPalette(0, palette_color_count, title_palette.data());
	::VH_UpdateScreen();
	::VL_RefreshScreen();

	bstone::Log::write("VITA: Direct title screen presented.");

	static_cast<void>(::IN_UserInput(TickBase * 6));

	::VL_SetPalette(0, palette_color_count, previous_palette.data());
	::IN_ClearKeysDown();
	::vid_is_movie = old_movie_state;

	bstone::Log::write("VITA: Leaving direct title screen.");
}

} // namespace

void bstone_vita_control_panel(ScanCode scancode)
{
	show_direct_title_screen_once();
	::US_ControlPanel(scancode);
}
