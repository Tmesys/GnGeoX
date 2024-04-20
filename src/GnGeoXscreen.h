/*!
*
*   \file    GnGeoXscreen.c
*   \brief   Screen routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SCREEN_H_
#define _GNGEOX_SCREEN_H_


typedef struct
{
    const char* name;
    const char* desc;
    SDL_bool ( *init ) ();
    SDL_bool ( *resize ) ( Sint32 w, Sint32 h );
    void ( *update ) ();
    void ( *fullscreen ) ();
    void ( *close ) ();
} blitter_func;

#ifndef _GNGEOX_SCREEN_C_
extern SDL_Surface* sdl_surface_screen;
extern SDL_Surface* sdl_surface_buffer;
extern SDL_Surface* sdl_surface_blend;
extern SDL_Window* sdl_window;
extern SDL_Renderer* sdl_renderer;
extern SDL_Rect visible_area;
extern TTF_Font* sys_font;
extern Sint32 yscreenpadding;
extern Uint8 scale;
extern Sint32 last_line;
#else
static void neo_screen_blend ( void );
#endif // _GNGEOX_SCREEN_C_

void print_blitter_list ( void );
Uint8 get_blitter_by_name ( const char* ) __attribute__ ( ( warn_unused_result ) );
SDL_bool neo_screen_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool neo_screen_resize ( Sint32, Sint32 ) __attribute__ ( ( warn_unused_result ) );
void neo_screen_efects_apply ( void );
void neo_screen_close ( void );
void neo_screen_fullscreen ( void );
void neo_screen_windowtitle_set ( void );
void neo_screen_update ( void );
void neo_screen_capture ( void );

#endif
