cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

set(SHORT_NAME bstone)
project(${SHORT_NAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Blake Stone")
set(VITA_TITLEID  "BSTONE012")
set(VITA_VERSION  "00.40")

option(VITA_TEST "Vita debug mode" OFF)
option(VITA_SHOW_WARNINGS "Show compiler warnings from the original sources" OFF)

if(VITA_SHOW_WARNINGS)
  set(BSTONE_WARNING_FLAGS "-Wall")
else()
  set(BSTONE_WARNING_FLAGS "-w")
endif()

# Keep the original Vita port's compiler and optimization settings unchanged,
# but retain symbols and a linker map for exact Vita core-dump resolution.
set(BSTONE_FLAGS "${BSTONE_WARNING_FLAGS} -fpermissive -Ofast -flto -funroll-loops -fvariable-expansion-in-unroller -g")

if(VITA_TEST)
  add_definitions(-DVITATEST)
  set(VITA_APP_NAME "Blake Stone Test")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 ${BSTONE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${BSTONE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bstone.map")

include_directories(
  "../dosbox"
  "${VITASDK}/arm-vita-eabi/include/SDL2"
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(
    BSTONE_HEADERS
    "../dosbox/adlib.h"
    "../dosbox/dbopl.h"
    "../dosbox/dosbox.h"
    "../dosbox/hardware.h"
    "../dosbox/inout.h"
    "../dosbox/mixer.h"
    "../dosbox/pic.h"
    "../dosbox/setup.h"
    "../3d_def.h"
    "../3d_menu.h"
    "../an_codes.h"
    "../audio.h"
    "../bstone_adlib_decoder.h"
    "../bstone_adlib_music_decoder.h"
    "../bstone_adlib_sfx_decoder.h"
    "../bstone_archiver.h"
    "../bstone_audio_decoder.h"
    "../bstone_audio_mixer.h"
    "../bstone_atomic.h"
    "../bstone_binary_reader.h"
    "../bstone_binary_writer.h"
    "../bstone_cl_args.h"
    "../bstone_crc32.h"
    "../bstone_endian.h"
    "../bstone_file_stream.h"
    "../bstone_fizzle_fx.h"
    "../bstone_fixed_point.h"
    "../bstone_format_string.h"
    "../bstone_generic_fizzle_fx.h"
    "../bstone_stream.h"
    "../bstone_log.h"
    "../bstone_memory_binary_reader.h"
    "../bstone_memory_stream.h"
    "../bstone_mt_queue_1r1w.h"
    "../bstone_opl2.h"
    "../bstone_ref_values.h"
    "../bstone_pcm_decoder.h"
    "../bstone_precompiled.h"
    "../bstone_ps_fizzle_fx.h"
    "../bstone_scope_guard.h"
    "../bstone_sha1.h"
    "../bstone_sprite.h"
    "../bstone_sprite_cache.h"
    "../bstone_string_helper.h"
    "../bstone_text_reader.h"
    "../bstone_text_writer.h"
    "../bstone_un_value.h"
    "../gfxv.h"
    "../id_ca.h"
    "../id_heads.h"
    "../id_in.h"
    "../id_pm.h"
    "../id_sd.h"
    "../id_us.h"
    "../id_vh.h"
    "../id_vl.h"
    "../jam_io.h"
    "../jm_cio.h"
    "../jm_io.h"
    "../jm_lzh.h"
    "../jm_tp.h"
    "../jm_vl.h"
    "../movie.h"
    "jm_lzh_overlap_fix.h"
)

set(
    BSTONE_SOURCES
    "../dosbox/dbopl.cpp"
    "../3d_act1.cpp"
    "../3d_act2.cpp"
    "../3d_agent.cpp"
    "../3d_debug.cpp"
    "../3d_draw.cpp"
    "../3d_draw2.cpp"
    "../3d_game.cpp"
    "../3d_inter.cpp"
    "../3d_main.cpp"
    "../3d_menu.cpp"
    "../3d_msgs.cpp"
    "../3d_play.cpp"
    "../3d_scale.cpp"
    "../3d_state.cpp"
    "../bstone_adlib_decoder.cpp"
    "../bstone_adlib_music_decoder.cpp"
    "../bstone_adlib_sfx_decoder.cpp"
    "../bstone_archiver_impl.cpp"
    "../bstone_audio_decoder.cpp"
    "../bstone_audio_mixer.cpp"
    "../bstone_binary_reader.cpp"
    "../bstone_binary_writer.cpp"
    "../bstone_cl_args.cpp"
    "../bstone_crc32.cpp"
    "../bstone_file_stream.cpp"
    "../bstone_fizzle_fx.cpp"
    "../bstone_fixed_point.cpp"
    "../bstone_format_string.cpp"
    "../bstone_generic_fizzle_fx.cpp"
    "../bstone_stream.cpp"
    "../bstone_log.cpp"
    "../bstone_memory_binary_reader.cpp"
    "../bstone_memory_stream.cpp"
    "../bstone_opl2.cpp"
    "../bstone_pcm_decoder.cpp"
    "../bstone_precompiled.cpp"
    "../bstone_ps_fizzle_fx.cpp"
    "../bstone_sha1.cpp"
    "../bstone_sprite.cpp"
    "../bstone_sprite_cache.cpp"
    "../bstone_string_helper.cpp"
    "../bstone_text_reader.cpp"
    "../bstone_text_writer.cpp"
    "../colormap.cpp"
    "../d3_d2.cpp"
    "../d3_dr2.cpp"
    "../id_ca.cpp"
    "../id_in.cpp"
    "../id_pm.cpp"
    "../id_sd.cpp"
    "../id_sdd.cpp"
    "../id_us.cpp"
    "../id_us_1.cpp"
    "../id_vh.cpp"
    "../id_vl.cpp"
    "../jm_cio.cpp"
    "../jm_free.cpp"
    "../jm_io.cpp"
    "../jm_lzh.cpp"
    "../jm_tp.cpp"
    "../markhack.cpp"
    "../movie.cpp"
    "../scale.cpp"
    "../stub.cpp"
    "../vgapal.cpp"
    "vita_input.cpp"
    "vita_palette_fade.cpp"
    "vita_widescreen_fix.cpp"
    "vita_pref_path_fix.cpp"
    "vita_startup_screen_fix.cpp"
    "vita_title_screen.cpp"
    "vita_save_osk.cpp"
    "vita_touch_fix.cpp"
)

add_executable(${SHORT_NAME}
    ${BSTONE_HEADERS}
    ${BSTONE_SOURCES}
)

# jm_lzh.cpp moves elements inside overlapping array ranges while rebuilding
# the adaptive Huffman tree. Force-include the Vita-specific compatibility
# header so these copies use memmove instead of undefined memcpy behavior.
set_source_files_properties(
  "../jm_lzh.cpp"
  PROPERTIES
    COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/jm_lzh_overlap_fix.h"
)

# Keep the original renderer code intact, but rename the desktop-oriented
# entry points that are unsafe on current Vita SDL2/GXM builds. Vita uses the
# compatibility implementations from the files in this directory instead.
set_source_files_properties(
  "../id_vl.cpp"
  PROPERTIES
    COMPILE_DEFINITIONS "VL_FadeIn=VL_FadeIn_legacy_unsafe;VL_FadeOut=VL_FadeOut_legacy_unsafe;vl_update_widescreen=vl_update_widescreen_legacy_unsafe"
)

# Prevent SDL_GetPrefPath from creating bibendovsky/bstone on Vita. Redirect
# the first menu call through a safe title-screen presenter while the original
# crash-prone movie/palette title path remains bypassed.
set_source_files_properties(
  "../3d_main.cpp"
  PROPERTIES
    COMPILE_DEFINITIONS "SDL_GetPrefPath=bstone_vita_get_pref_path;US_ControlPanel=bstone_vita_control_panel"
)

# Use the native Vita on-screen keyboard only for savegame-name entry.
set_source_files_properties(
  "../3d_menu.cpp"
  PROPERTIES
    COMPILE_DEFINITIONS "US_LineInput=bstone_vita_line_input"
)

# Current Vita SDL2 reports the front panel as TouchID 1. Retain the original
# handler for reference and use vita_touch_fix.cpp as the active implementation.
set_source_files_properties(
  "vita_input.cpp"
  PROPERTIES
    COMPILE_DEFINITIONS "TranslateTouchEvent=TranslateTouchEvent_legacy_wrong_touch_id"
)

target_link_libraries(${SHORT_NAME}
  SDL2
  vita2d
  SceDisplay_stub
  SceCtrl_stub
  SceAudio_stub
  SceAudioIn_stub
  SceSysmodule_stub
  SceGxm_stub
  SceCommonDialog_stub
  SceTouch_stub
  SceHid_stub
  SceMotion_stub
  m
  ScePower_stub
  SceAppUtil_stub
  pthread
  SceAppMgr_stub
)

vita_create_self(${SHORT_NAME}.self ${SHORT_NAME})
vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/ps.png sce_sys/livearea/contents/ps.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
