/*
BStone Vita savegame-name keyboard support.

The original save menu calls US_LineInput(), which expects a physical keyboard.
On Vita, SDL_StartTextInput opens the native IME dialog. The Vita SDL backend
returns the completed text through SDL_TEXTINPUT and sends Return only when the
user confirms the dialog.
*/

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "SDL.h"
#include "../id_in.h"
#include "../id_us.h"


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
	if (!buf || maxchars <= 0)
	{
		return false;
	}

	auto* const window = ::SDL_GetKeyboardFocus();

	if (!window || ::SDL_HasScreenKeyboardSupport() != SDL_TRUE)
	{
		return ::US_LineInput(x, y, buf, def, escok, maxchars, maxwidth);
	}

	const auto max_length = static_cast<std::size_t>(maxchars);
	auto text = std::string{};
	auto keyboard_was_shown = false;
	auto accepted = false;
	auto finished = false;
	auto startup_polls = 0;

	::IN_ClearKeysDown();
	::SDL_FlushEvent(SDL_TEXTINPUT);
	::SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	::SDL_StartTextInput();

	while (!finished)
	{
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

		// Fall back to the original editor if this SDL build unexpectedly
		// reports keyboard support but cannot open the Vita IME.
		if (!keyboard_was_shown && ++startup_polls > 180)
		{
			::SDL_StopTextInput();
			::IN_ClearKeysDown();
			return ::US_LineInput(x, y, buf, def, escok, maxchars, maxwidth);
		}

		if (!finished)
		{
			::SDL_Delay(10);
		}
	}

	::SDL_StopTextInput();
	::IN_ClearKeysDown();

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
