/*!
*
*   \file    GnGeoXscreen.c
*   \brief   Screen routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
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
    Uint8 x_ratio, y_ratio;
    SDL_bool ( *init ) ();
    void ( *update ) ();
} struct_gngeoxscreen_effect_func;

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
extern SDL_Texture* sdl_texture;
extern SDL_Window* sdl_window;
extern SDL_Renderer* sdl_renderer;
extern SDL_Rect visible_area;
extern TTF_Font* sys_font;
extern Sint32 yscreenpadding;
extern Uint8 nblitter;
extern Uint8 neffect;
extern Uint8 scale;
extern Sint32 last_line;
extern struct_gngeoxscreen_effect_func effect[];
#else
static SDL_bool effect_none_init ( void ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool init_screen ( void ) __attribute__ ( ( warn_unused_result ) );
static void do_interpolation ( void );
#endif // _GNGEOX_SCREEN_C_

void print_blitter_list ( void );
void print_effect_list ( void );
Uint8 get_effect_by_name ( char* name ) __attribute__ ( ( warn_unused_result ) );
Uint8 get_blitter_by_name ( char* name ) __attribute__ ( ( warn_unused_result ) );
SDL_bool screen_resize ( Sint32, Sint32 ) __attribute__ ( ( warn_unused_result ) );
void screen_update ( void );
void screen_close ( void );
void screen_fullscreen ( void );
void sdl_set_title ( void );
SDL_bool init_sdl ( void );
void update_screen ( void );
void take_screenshot ( void );

#endif
