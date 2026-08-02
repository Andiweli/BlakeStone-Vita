/*
BStone Vita palette fade compatibility layer.

The original renderer stores palettes as uint8_t[256][3] and accesses them as
one flat 768-byte array. Crossing sub-array boundaries that way is undefined
C++ behavior and newer VitaSDK GCC builds can generate invalid code during the
title-screen fade. These Vita-only replacements use real flat buffers while
preserving the original fade timing and final palette state.
*/

#include <array>
#include <cassert>
#include <cstdint>

#include "../id_vl.h"


extern bool g_no_fade_in_or_out;


namespace
{

constexpr int color_count = 256;
constexpr int channel_count = 3;
constexpr int palette_size = color_count * channel_count;

using Palette = std::array<std::uint8_t, palette_size>;


int clamp_color_index(const int value)
{
	if (value < 0)
	{
		return 0;
	}

	if (value >= color_count)
	{
		return color_count - 1;
	}

	return value;
}

void refresh_fade_frame()
{
	::VL_RefreshScreen();

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}
}

} // namespace


void VL_FadeOut(
	const int start,
	const int end,
	const int red,
	const int green,
	const int blue,
	const int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(red >= 0 && red <= 0xFF);
	assert(green >= 0 && green <= 0xFF);
	assert(blue >= 0 && blue <= 0xFF);
	assert(steps > 0);
	assert(start <= end);

	const auto first_color = clamp_color_index(start);
	const auto last_color = clamp_color_index(end);
	const auto safe_steps = steps > 0 ? steps : 1;

	if (!::g_no_fade_in_or_out && first_color <= last_color)
	{
		auto original_palette = Palette{};
		auto frame_palette = Palette{};

		::VL_GetPalette(0, color_count, original_palette.data());

		for (int step = 0; step < safe_steps; ++step)
		{
			frame_palette = original_palette;

			for (int color = first_color; color <= last_color; ++color)
			{
				const auto base_index = color * channel_count;
				const int targets[channel_count] = {red, green, blue};

				for (int channel = 0; channel < channel_count; ++channel)
				{
					const auto index = base_index + channel;
					const auto original = static_cast<int>(original_palette[index]);
					const auto delta = targets[channel] - original;

					frame_palette[index] = static_cast<std::uint8_t>(
						original + ((delta * step) / safe_steps));
				}
			}

			::VL_SetPalette(0, color_count, frame_palette.data());
			refresh_fade_frame();
		}
	}

	::VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	refresh_fade_frame();
	::screenfaded = true;
}

void VL_FadeIn(
	const int start,
	const int end,
	const std::uint8_t* const palette,
	const int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(palette != nullptr);
	assert(steps > 0);
	assert(start <= end);

	if (!palette)
	{
		return;
	}

	const auto first_color = clamp_color_index(start);
	const auto last_color = clamp_color_index(end);
	const auto safe_steps = steps > 0 ? steps : 1;

	if (!::g_no_fade_in_or_out && first_color <= last_color)
	{
		auto original_palette = Palette{};
		auto frame_palette = Palette{};

		::VL_GetPalette(0, color_count, original_palette.data());

		for (int step = 0; step < safe_steps; ++step)
		{
			frame_palette = original_palette;

			for (int color = first_color; color <= last_color; ++color)
			{
				const auto base_index = color * channel_count;

				for (int channel = 0; channel < channel_count; ++channel)
				{
					const auto index = base_index + channel;
					const auto original = static_cast<int>(original_palette[index]);
					const auto target = static_cast<int>(palette[index]);
					const auto delta = target - original;

					frame_palette[index] = static_cast<std::uint8_t>(
						original + ((delta * step) / safe_steps));
				}
			}

			::VL_SetPalette(0, color_count, frame_palette.data());
			refresh_fade_frame();
		}
	}

	::VL_SetPalette(0, color_count, palette);
	refresh_fade_frame();
	::screenfaded = false;
}
