/*
BStone Vita savegame compression fix.

jm_lzh.cpp rebuilds its adaptive Huffman tree by shifting elements inside the
same arrays. The original code uses memcpy for these overlapping ranges,
which is undefined behavior and can corrupt larger savegames. Include this
header only for the jm_lzh.cpp translation unit so those copies use memmove.
*/

#ifndef BSTONE_VITA_JM_LZH_OVERLAP_FIX_INCLUDED
#define BSTONE_VITA_JM_LZH_OVERLAP_FIX_INCLUDED

#include <cstring>

#ifdef memcpy
#undef memcpy
#endif

#define memcpy memmove

#endif // BSTONE_VITA_JM_LZH_OVERLAP_FIX_INCLUDED
