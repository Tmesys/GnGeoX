# GnGeoX : Another fork of the Gngeo NeoGeo emulator for linux (and maybe some other unix)

## GnGeoX:
GnGeoX is a fork of GnGeo that aims to modernize this great emulator yet giving it a new refresh.

## REQUIREMENT : 
### SDL2
Simple DirectMedia Layer Version 2.0.0 or above.
### SDL-TTF
Version 2.0.0 or above.
### sqlite
Rom driver database and tile transparency data database, version 3.8.1.11 or above.
### bstrlib : 
Customized version of the library by Paul Hsieh (included in this code distribution).
### Qlib :  
Customized version of the library by Seungyoung Kim (included in this code distribution).
### Zlog :  
Customized version of the library by Hardy Simpson (included in this code distribution).
### generator68k : 
Customized version of the library by James Ponder (included in this code distribution).
### Z80 : 
Customized version of the library by Juergen Buchmueller (included in this code distribution).
### OpenGL: 
For hardware accelerated blitters.

## INSTALLATION :
GnGeoX uses codeblocks workspace and project files to build.

## CONFIGURATION :
All configuration can be done in *gngeox.ini*, please refer to this file for further details, as it is well commented.
Every option is also accessible on the command line.

## USAGE :
You can start a game with the folowing command:
>gngeo -f game or > gngeo --gamename game
where game is the name of the rom zip file, for example mslug for Metal Slug

Usage: gngeo [OPTION]... ROMSET
Emulate the NeoGeo rom designed by ROMSET

      --68kclock=x           Overclock the 68k by x% (-x% for underclk)
      --autoframeskip        Enable auto frameskip
      --bench                Draw x frames, then quit and show average fps
  -B, --biospath=PATH        Tell gngeo where your neogeo bios is
  -b, --blitter=Blitter      Use the specified blitter (help for a list)
      --country=...          Set the contry to japan, asia, usa or europe
  -D, --debug                Start with inline debuger
  -e, --effect=Effetc        Use the specified effect (help for a list)
      --forcepc              Force the PC to a correct value at startup
  -f, --fullscreen           Start gngeo in fullscreen
  -d, --gngeo.dat=PATH       Tell gngeo where his ressource file is
  -h, --help                 Print this help and exit
  -H, --hwsurface            Use hardware surface for the screen
  -I, --interpolation        Merge the last frame and the current
      --joystick             Enable joystick support
  -l, --listgame             Show all the game available in the romrc
      --libglpath=PATH       Path to your libGL.so
  -P, --pal                  Use PAL timing (buggy)
  -r, --raster               Enable the raster interrupt
  -i, --rompath=PATH         Tell gngeo where your roms are
      --sound                Enable sound
      --showfps              Show FPS at startup
      --sleepidle            Sleep when idle
      --screen320            Use 320x224 output screen (instead 304x224)
      --system=...           Set the system to home, arcade or unibios
      --scale=X              Scale the resolution by X
      --samplerate=RATE      Set the sample rate to RATE
  -t, --transpack=Transpack  Use the specified transparency pack
  -v, --version              Show version and exit
      --z80clock=x           Overclock the Z80 by x% (-x% for underclk)

## LICENSE:
### GnGeo
Gngeo is build arround many different block with various license.
The original code is released under the GPLV2 with this special exception:

_As a special exception, You have
the permission to link the code of this program with
independent modules,regardless of the license terms of these
independent modules, and to copy and distribute the resulting
executable under terms of your choice, provided that you also
meet, for each linked independent module, the terms and conditions
of the license of that module. An independent module is a module
which is not derived from or based on Gngeo. If you modify
this library, you may extend this exception to your version of the
library, but you are not obligated to do so.  If you do not wish
to do so, delete this exception statement from your version._

### MAME
Gngeo could not exist without the Mame project, and some code come
directly from it (the ym2610 for example). As you may know, the Mame
license forbid commercial use, and as a consequence, commercial use
of gngeo (as a whole) is also forbided :

Redistribution and use of the MAME code or any derivative works are 
permitted provided that the following conditions are met:

1 - Redistributions may not be sold, nor may they be used in a commercial product or activity.
2 - Redistributions that are modified from the original source 
must include the complete source code, including the source 
code for all components used by a binary built from the modified 
sources. However, as a special exception, the source code distributed 
need not include anything that is normally distributed (in either 
source or binary form) with the major components (compiler, kernel, 
and so on) of the operating system on which the executable runs, 
unless that component itself accompanies the executable.
3 - Redistributions must reproduce the above copyright notice, this 
list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

This software is provided by the copyright holders and contributors "as is" 
and any express or implied warranties, including, but not limited to, the 
implied warranties of merchantability and fitness for a particular purpose 
are disclaimed. in no event shall the copyright owner or contributors be 
liable for any direct, indirect, incidental, special, exemplary, or 
consequential damages (including, but not limited to, procurement of 
substitute goods or services; loss of use, data, or profits; or business 
interruption) however caused and on any theory of liability, whether in 
contract, strict liability, or tort (including negligence or otherwise) 
arising in any way out of the use of this software, even if advised of 
the possibility of such damage.

### Z80
The original code is released under this licence :
- This source code is released as freeware for non-commercial purposes.
- You are free to use and redistribute this code in modified or unmodified form, provided you list me in the credits.
- If you modify this source code, you must add a notice to each modified source file that it has been changed.  If you're a nice person, you will clearly mark each change too.  :)
- If you wish to use this for commercial purposes, please contact me at pullmoll@t-online.de.
- The author of this copywritten work reserves the right to change the terms of its usage and license at any time, including retroactively.
- This entire notice must remain in the source code.

### Generator (68 emulation layer)
The original code is released under the GPLV2.

### qLibc
The original code is released under this licence :
Copyright (c) 2010-2015 Seungyoung Kim.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

this software is provided by the copyright holders and contributors "as is"
and any express or implied warranties, including, but not limited to, the
implied warranties of merchantability and fitness for a particular purpose
are disclaimed. in no event shall the copyright owner or contributors be
liable for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in
contract, strict liability, or tort (including negligence or otherwise)
arising in any way out of the use of this software, even if advised of the
possibility of such damage.

### bstrlib
The original code is released under the GPLV2.

### Zlog
The original code is released under the LGPL.
