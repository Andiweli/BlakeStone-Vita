/*
BStone Vita runtime video-mode compatibility fix.

The Vita renderer is already initialized at the native display size during
VL_Startup. The original desktop-oriented runtime widescreen update destroys
and recreates the screen texture later during startup. With current VitaSDK
SDL2/GXM this second texture creation is followed by a data-abort before the
title screen appears.

Keep the initial native Vita renderer and texture instead of recreating them.
*/

#include "../id_vl.h"


void vl_update_widescreen()
{
	// Intentionally empty on Vita.
	// The initial renderer already uses the Vita display dimensions and the
	// default widescreen layout. Recreating only the screen texture here leaves
	// the renderer in an invalid state on current SDL2/GXM builds.
}
