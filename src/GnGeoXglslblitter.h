/*!
*
*   \file    GnGeoXglslblitter.h
*   \brief   Glsl blitter routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    07/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_GLSLBLITTER_H_
#define _GNGEOX_GLSLBLITTER_H_

/*
 * Libretro's GLSL preset
 * https://github.com/libretro/docs/blob/master/docs/specs/shader.md
 */
#define SOURCE 0
#define ABSOLUTE 1
#define VIEWPORT 2

#define SHADER_TAG_LEN 200

#define GLSLP_SHADER        "shader"
#define GLSLP_SCALE_TYPE_X  "scale_type_x"
#define GLSLP_SCALE_TYPE_Y  "scale_type_y"
#define GLSLP_SCALE_TYPE    "scale_type"
#define GLSLP_SCALE_X       "scale_x"
#define GLSLP_SCALE_Y       "scale_y"
#define GLSLP_SCALE         "scale"
#define GLSLP_FILTER_LINEAR "filter_linear"

#define PARSE_INT(expr, var)                        \
{                                   \
    char* fail = NULL;                      \
    var = strtol(expr, &fail, 10);                  \
    if (*fail != '\0') {                        \
        printf("bad parsing, not an integer: '%s'\n", val); \
        return SDL_FALSE;                    \
    }                               \
}

#define PARSE_FLOAT(expr, var)                      \
{                                   \
    char* fail = NULL;                      \
    var = strtof(expr, &fail);                  \
    if (*fail != '\0') {                        \
        printf("bad parsing, not an integer: '%s'\n", val); \
        return SDL_FALSE;                    \
    }                               \
}

#define PARSE_SCALE(val, var)                       \
{                                   \
    Sint32 stype;                          \
    if (!strcmp(val, "source"))        { stype = SOURCE; }      \
    else if (!strcmp(val, "absolute")) { stype = ABSOLUTE; }    \
    else if (!strcmp(val, "viewport")) { stype = VIEWPORT; }    \
    else {                              \
        printf("Invalid scale type: '%s'\n", val);      \
        return SDL_FALSE;                                        \
    }                               \
    var = stype;                            \
}

#define PARSE_POS(expr, var)                        \
{                                   \
    char* fail = NULL;                      \
    var = strtol(expr, &fail, 10);                  \
    if (*fail != '\0') {                        \
        printf("bad parsing, not an integer: '%s'\n", val); \
        return SDL_FALSE;                    \
    }                               \
    if (pipeline == NULL) {                     \
        printf("Number of shaders not yet initialized\n");  \
        return SDL_FALSE;                    \
    }                               \
    if (var<0 || var>=nb_passes) {                  \
        printf("Shader number is invalid: '%d'\n", var);    \
        return SDL_FALSE;                    \
    }                               \
}

/// A pass configuration in a GLSL preset
typedef struct _pass_
{
    Sint32    scale_type_x;
    Sint32    scale_type_y;
    Sint32    filter_linear;
    float  scale_x;
    float  scale_y;
    GLuint program;
    Sint32    src_width;
    Sint32    src_height;
    GLuint src_texture;
    Sint32    dst_width;
    Sint32    dst_height;
    GLuint dst_framebuffer;
}* pass_t;

#ifdef _GNGEOX_GLSLBLITTER_C_
static char* load_file_and_wrap ( const char*, const char* );
static GLuint compile_shader ( const char*, GLenum );
static GLuint compile_shader_program ( char* );
static void set_texture_filter_linear ( GLuint, Sint32 );
static GLuint create_input_texture ( void );
static void create_pass_framebuffer ( GLuint*, GLuint* );
static void init_static_gl_coords_buffers ( void );
static char* trim ( char*, char* );
static Sint32 startsWith ( char*, char* );
static SDL_bool parse_shader_preset ( const char*, Sint32, Sint32, Sint32, Sint32 );
static void render_pass ( Sint32, Sint32, GLuint, Sint32, Sint32, GLuint, Sint32, Sint32, GLuint );
#endif // _GNGEOX_GLSLBLITTER_C_

SDL_bool blitter_glsl_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool blitter_glsl_resize ( Sint32, Sint32 ) __attribute__ ( ( warn_unused_result ) );
void blitter_glsl_update ( void );
void blitter_glsl_close ( void );
void blitter_glsl_fullscreen ( void );

#endif
