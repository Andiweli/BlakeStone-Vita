/*
BStone Vita savegame-name keyboard support.

The original save menu calls US_LineInput(), which expects a physical keyboard.
On Vita, SDL_StartTextInput opens the native IME dialog. The GXM renderer must
continue presenting frames while the common dialog is active; otherwise the
keyboard remains invisible and the input routine appears to hang.
*/

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "SDL.h"
#include "../id_in.h"
#include "../id_us.h"
#include "../id_vl.h"


namespace
{

void append_ascii_text(
	std::string& text,
	const char* source,
	const std::size_t max_length)
{
	if (!source)
	{
		return;
	}

	for (auto value = reinterpret_cast<const unsigned char*>(source); *value; ++value)
	{
		if (*value < 0x20 || *value > 0x7E)
		{
			continue;
		}

		if (text.size() >= max_length)
		{
			break;
		}

		text += static_cast<char>(std::toupper(*value));
	}
}

void finish_text_input()
{
	::SDL_StopTextInput();
	::SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	::IN_ClearKeysDown();
}

} // namespace


bool bstone_vita_line_input(
	std::int16_t x,
	std::int16_t y,
	char* buf,
	char* def,
	bool escok,
	std::int16_t maxchars,
	std::int16_t maxwidth)
{
	static_cast<void>(x);
	static_cast<void>(y);
	static_cast<void>(maxwidth);

	if (!buf || maxchars <= 0)
	{
		return false;
	}

	auto* const window = ::SDL_GetKeyboardFocus();

	// Never fall back to US_LineInput on Vita. Without a physical keyboard that
	// editor blocks the menu indefinitely. A failed IME simply cancels naming.
	if (!window || ::SDL_HasScreenKeyboardSupport() != SDL_TRUE)
	{
		::IN_ClearKeysDown();
		return false;
	}

	const auto max_length = static_cast<std::size_t>(maxchars);
	auto text = std::string{};
	auto keyboard_was_shown = false;
	auto accepted = false;
	auto finished = false;
	auto startup_frames = 0;

	::IN_ClearKeysDown();
	::SDL_FlushEvent(SDL_TEXTINPUT);
	::SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	::SDL_StartTextInput();

	while (!finished)
	{
		// Vita's GXM SDL renderer calls sceCommonDialogUpdate from
		// SDL_RenderPresent. Keep presenting the current save-menu frame so the
		// IME becomes visible and remains interactive.
		::VL_RefreshScreen();

		auto event = SDL_Event{};

		while (::SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_TEXTINPUT:
				// The dialog backend returns the complete string at once. The
				// live IME backend may return one character at a time.
				if (::std::strlen(event.text.text) > 1)
				{
					text.clear();
				}

				append_ascii_text(text, event.text.text, max_length);
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					accepted = true;
					finished = true;
					break;

				case SDLK_ESCAPE:
					if (escok)
					{
						finished = true;
					}
					break;

				case SDLK_BACKSPACE:
					if (!text.empty())
					{
						text.pop_back();
					}
					break;

				case SDLK_SPACE:
					if (text.size() < max_length)
					{
						text += ' ';
					}
					break;

				default:
					break;
				}
				break;

			case SDL_QUIT:
				finished = true;
				break;

			default:
				break;
			}
		}

		const auto keyboard_is_shown =
			::SDL_IsScreenKeyboardShown(window) == SDL_TRUE;

		keyboard_was_shown = keyboard_was_shown || keyboard_is_shown;

		if (keyboard_was_shown && !keyboard_is_shown)
		{
			finished = true;
		}

		// If this SDL build cannot start the IME, cancel cleanly instead of
		// entering the physical-keyboard editor and locking the menu.
		if (!keyboard_was_shown && ++startup_frames > 180)
		{
			finish_text_input();
			return false;
		}

		if (!finished)
		{
			::SDL_Delay(16);
		}
	}

	finish_text_input();

	if (!accepted)
	{
		return false;
	}

	if (text.empty() && def)
	{
		append_ascii_text(text, def, max_length);
	}

	const auto copy_length = std::min(text.size(), max_length);
	::std::memcpy(buf, text.data(), copy_length);
	buf[copy_length] = '\0';

	return true;
}
