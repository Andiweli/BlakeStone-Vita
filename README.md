<img width="2172" height="724" alt="Blake Stone PS Vita port title artwork" src="https://github.com/user-attachments/assets/9c39af93-7a33-462e-b876-e3189ae73e6a" />

# Blake Stone PS Vita Port

**Blake Stone Vita** is a PlayStation Vita port of BStone for **Blake Stone: Aliens of Gold** and **Blake Stone: Planet Strike**. It brings both classic first-person shooters to PS Vita with controller, touchscreen and native on-screen keyboard support.

This repository contains the current **PS Vita homebrew version 0.5**. Original game data is required and is not included.

## PS Vita Changes

- Native PS Vita on-screen keyboard for savegame names and high-score entries.
- Reliable saving and loading through a fix for overlapping LZH compression memory operations.
- Correct front-touchscreen detection with weapon, elevator-number and Planet Strike map-zoom touch zones.
- PS Vita confirmation controls: **X = Yes** and **O = No**.
- `SWITCHES2` renamed to **SKIP OPTIONS**, including the submenu title.
- Vita profile path fixed to `ux0:/data/bstone/`.
- Vita-specific palette, widescreen and SDL compatibility fixes.
- Application, in-game and LiveArea version updated to **0.5**.

## Overview

### Supported Games

- Blake Stone: Aliens of Gold — shareware and full versions 1.0, 2.0, 2.1 and 3.0
- Blake Stone: Planet Strike — versions 1.0 and 1.1

### Installation and Launch

1. Install the `BlakeStone.vpk` on your PS Vita.
2. Create the directory `ux0:/data/bstone/` if it does not already exist.
3. Copy the required original game assets into that directory.
4. Start the game via LiveArea

Assets for multiple supported versions may coexist in the same directory. For reliable operation, close an already running instance before launching the application again from LiveArea.

### Controls

| PS Vita input | Action |
|---|---|
| Left stick | Move and strafe |
| Right stick | Turn |
| D-pad | Move forward/backward and turn |
| L / Square | Use or open |
| R / Triangle | Fire or accept |
| X | About face; Yes in Y/N confirmation dialogs |
| O | Next weapon or menu back; No in Y/N confirmation dialogs |
| Select | Toggle map or status window |
| Start | Open menu or go back |

The front touchscreen provides additional controls:

- Top-left and top-right areas select the previous or next weapon.
- The right-side column acts as number keys for weapon selection and elevator buttons.
- The left and right halves of the lower HUD area control Planet Strike map zoom.

Configuration, logs, high scores and savegames are stored in `ux0:/data/bstone/`.

## Required Assets

The commercial game data is not included. You must own a legal copy of the game you want to play.

| Aliens of Gold Shareware | Aliens of Gold Full | Planet Strike |
|---|---|---|
| `AUDIOHED.BS1` | `AUDIOHED.BS6` | `AUDIOHED.VSI` |
| `AUDIOT.BS1` | `AUDIOT.BS6` | `AUDIOT.VSI` |
|  | `EANIM.BS6` | `EANIM.VSI` |
|  | `GANIM.BS6` |  |
| `IANIM.BS1` | `IANIM.BS6` | `IANIM.VSI` |
| `MAPHEAD.BS1` | `MAPHEAD.BS6` | `MAPHEAD.VSI` |
| `MAPTEMP.BS1` | `MAPTEMP.BS6` | `MAPTEMP.VSI` |
| `SANIM.BS1` | `SANIM.BS6` |  |
| `VGADICT.BS1` | `VGADICT.BS6` | `VGADICT.VSI` |
| `VGAGRAPH.BS1` | `VGAGRAPH.BS6` | `VGAGRAPH.VSI` |
| `VGAHEAD.BS1` | `VGAHEAD.BS6` | `VGAHEAD.VSI` |
| `VSWAP.BS1` | `VSWAP.BS6` | `VSWAP.VSI` |

## Credits

- **Boris Bendovsky** — BStone source port
- **JAM Productions** — Blake Stone development
- **id Software** — Wolfenstein 3D engine
- **Apogee Entertainment** — publishing and original source release
- **01y** — original PS Vita port
- **Andiweli** — compatibility fixes and version 0.5

## Disclaimer

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013-2019 Boris I. Bendovsky (`bibendovsky@hotmail.com`)

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**, including the implied warranties of merchantability or fitness for a particular purpose. See the GNU General Public License for more details.

See [`LICENSE`](LICENSE) for the GNU General Public License and `Blake Stone source code license.doc` for the original source-code license.
