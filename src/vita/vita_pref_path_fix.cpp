/*
BStone Vita profile-path compatibility layer.

The generic profile initialization calls SDL_GetPrefPath("bibendovsky",
"bstone") before replacing the result with ux0:/data/bstone/. On Vita that
unnecessary call creates ux0:/data/bibendovsky/ and its bstone subdirectory.
The Vita build redirects only that call here, so no unwanted directories are
created and the existing fixed Vita profile path remains in use.
*/

#include "SDL.h"


extern "C" char* SDLCALL bstone_vita_get_pref_path(
	const char* /* organization */,
	const char* /* application */)
{
	return nullptr;
}
