#ifndef _TRANSPACK_H_
#define _TRANSPACK_H_

#include <SDL2/SDL.h>

typedef enum
{
    TILE_NORMAL = 0,
    TILE_INVISIBLE = 1,
    TILE_TRANSPARENT25 = 2,
    TILE_TRANSPARENT50 = 3,
    TILE_TRANS_UNKNOWN = 4,
    TILE_TRANS_MAX = 5,
} enum_gngeoxtranspack_tile_type;

SDL_bool neo_transpack_init ( void ) __attribute__ ( ( warn_unused_result ) );
enum_gngeoxtranspack_tile_type neo_transpack_find ( Uint32 );
SDL_bool neo_transpack_close ( void );

#endif
