/*!
*
*   \file    GnGeoXglslblitter.c
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
#ifndef _GNGEOX_GLSLBLITTER_C_
#define _GNGEOX_GLSLBLITTER_C_
#endif // _GNGEOX_GLSLBLITTER_C_

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"

#ifdef USE_GL2
#include <GL/glew.h>
#else
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_opengles2_gl2.h>
#endif

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXconfig.h"
#include "GnGeoXglslblitter.h"

/// The entire GLSL multi-pass rendering
static pass_t pipeline;

/// Number of passes in the GLSL rendering
static Sint32 nb_passes;

/*
 * Rendering settings
 */

/// configured viewport in the total visible area (320x224)
static SDL_Rect screen_rect = { 8,  0, 304, 224};
/// configured screen ratio, maps classic CRT ratio
static float screen_ratio = 4.0 / 3.0;
static Uint32 current_window_width;
static Uint32 current_window_height;

/// The pixel data to upload into the source GL texture
static SDL_Surface* input_pixels;
/// The input texture that is feeded to the GLSL rendering
static GLuint input_tex;

/// Number of frames to clear after resize, for double buffer
Sint32 clear_after_resize;

#ifdef USE_GL2
/// The vertex array object required for rendering with OpenGL
static GLuint vao;
#endif

/*
 * vertex and texel coordinate buffers for rendering
 */

static GLuint vertex_buffer;
static GLuint tex_buffer;
static GLuint screen_buffer;

static GLfloat MVPMatrix[] =
{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static GLfloat VertexData[] =
{
    -1.0, -1.0,
        1.0, -1.0,
        -1.0,  1.0,
        -1.0,  1.0,
        1.0, -1.0,
        1.0,  1.0
    };

static GLfloat TexData[] =
{
    0.0,  1.0,
    1.0,  1.0,
    0.0,  0.0,
    0.0,  0.0,
    1.0,  1.0,
    1.0,  0.0
};

static GLfloat ScreenData[] =
{
    0.0,  0.0,
    1.0,  0.0,
    0.0,  1.0,
    0.0,  1.0,
    1.0,  0.0,
    1.0,  1.0
};
/* ******************************************************************************************************************/
/*!
* \brief  Read the content of a file into an allocated buffer and wrap the content into a CPP define statement.
*
* \param file Shader definition file path.
* \param shadertag glsl shader define.
* \return Shader script.
*/
/* ******************************************************************************************************************/
static char* load_file_and_wrap ( const char* file, const char* shadertag )
{
    FILE* fptr = NULL;
    Sint64 length = 0;
    char* buf = NULL;
    char* shader_buf = NULL;
    char def_buf[SHADER_TAG_LEN];
    snprintf ( def_buf, SHADER_TAG_LEN, "#version 100\n#define %s 1\n", shadertag );

    fptr = fopen ( file, "rb" );

    if ( !fptr )
    {
        printf ( "Could not open %s for reading: %s\n", file, strerror ( errno ) );
        return NULL;
    }

    fseek ( fptr, 0, SEEK_END );
    length = ftell ( fptr );
    buf = ( char* ) qalloc ( SHADER_TAG_LEN + length + 1 );

    strncpy ( buf, def_buf, SHADER_TAG_LEN );
    shader_buf = buf + strlen ( buf );

    fseek ( fptr, 0, SEEK_SET );
    fread ( shader_buf, length, 1, fptr );
    fclose ( fptr );
    shader_buf[length] = 0;

    return ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param shader_name Shader definition file path.
* \param shader_type Can be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
* \return Shader script.
*/
/* ******************************************************************************************************************/
static GLuint compile_shader ( const char* shader_name, GLenum shader_type )
{
    //char* shader_path = NULL;
    char* full_name = NULL;
    char* src = NULL;
    GLuint shader = 0;
    char* type_name = NULL;
    Sint32 buflen = 0;
    Sint32 status = 0;

    buflen = strlen ( gngeox_config.shaderpath ) + strlen ( shader_name ) + 2;
    full_name = qalloc ( buflen );
    snprintf ( full_name, buflen, "%s/%s", gngeox_config.shaderpath, shader_name );

    type_name = shader_type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT";
    shader = glCreateShader ( shader_type );

    if ( shader == 0 )
    {
        printf ( "Could not create a %s shader object\n", type_name );
        return ( GL_FALSE );
    }

    src = load_file_and_wrap ( full_name, type_name );

    if ( src == NULL )
    {
        return ( GL_FALSE );
    }

    glShaderSource ( shader, 1, ( const GLchar** ) &src, 0 );
    glCompileShader ( shader );
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &status );

    if ( status == GL_FALSE )
    {
        Sint32 length;
        char* log;
        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &length );
        log = qalloc ( length );
        glGetShaderInfoLog ( shader, length, &length, log );
        printf ( "Error when compiling shader:\n%s\n", log );
        qalloc_delete ( log );

        return ( GL_FALSE );
    }

    return ( shader );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param shader_name Shader definition file path.
* \return Shader script.
*/
/* ******************************************************************************************************************/
static GLuint compile_shader_program ( char* shader_name )
{
    GLuint vertex = 0, fragment = 0, program = 0;
    Sint32 linked = 0;

    vertex = compile_shader ( shader_name, GL_VERTEX_SHADER );

    if ( vertex == GL_FALSE )
    {
        return ( GL_FALSE );
    }

    fragment = compile_shader ( shader_name, GL_FRAGMENT_SHADER );

    if ( fragment == GL_FALSE )
    {
        return ( GL_FALSE );
    }

    program = glCreateProgram();
    glAttachShader ( program, vertex );
    glAttachShader ( program, fragment );
    glLinkProgram ( program );
    glGetProgramiv ( program, GL_LINK_STATUS, ( Sint32* ) &linked );

    if ( !linked )
    {
        Sint32 length = 0;
        char* log;
        glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &length );
        log = ( char* ) qalloc ( length );
        glGetProgramInfoLog ( program, length, &length, log );
        printf ( "Error when linking shader:\n%s\n", log );
        qalloc_delete ( log );
        return ( SDL_FALSE );
    }

    // GLuint attr;
    // GLuint uni;
    // glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attr);
    // glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uni);
    zlog_info ( gngeox_config.loggingCat, "Linked shader program: %s", shader_name );

    return ( program );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param texture ?.
* \param linear ?.
*/
/* ******************************************************************************************************************/
static void set_texture_filter_linear ( GLuint texture, Sint32 linear )
{
    glBindTexture ( GL_TEXTURE_2D, texture );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \return ?.
*/
/* ******************************************************************************************************************/
static GLuint create_input_texture ( void )
{
    GLuint texture = 0;

    glGenTextures ( 1, &texture );
    glBindTexture ( GL_TEXTURE_2D, texture );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    set_texture_filter_linear ( texture, GL_FALSE );

    return ( texture );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param width ?.
* \param height ?.
* \param texture ?.
* \param framebufferid ?.
*/
/* ******************************************************************************************************************/
static void create_pass_framebuffer ( GLuint* texture, GLuint* framebufferid )
{
    *texture = create_input_texture();
    *framebufferid = 0;

    glGenFramebuffers ( 1, framebufferid );
    glBindFramebuffer ( GL_FRAMEBUFFER, *framebufferid );
    glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
static void init_static_gl_coords_buffers ( void )
{
#ifdef USE_GL2
    glGenVertexArrays ( 1, &vao );
    glBindVertexArray ( vao );
#endif
    glGenBuffers ( 1, &vertex_buffer );
    glBindBuffer ( GL_ARRAY_BUFFER, vertex_buffer );
    glBufferData ( GL_ARRAY_BUFFER, 12 * sizeof ( GLfloat ), VertexData, GL_STATIC_DRAW );

    glGenBuffers ( 1, &tex_buffer );
    glBindBuffer ( GL_ARRAY_BUFFER, tex_buffer );
    glBufferData ( GL_ARRAY_BUFFER, 12 * sizeof ( GLfloat ), TexData, GL_STATIC_DRAW );

    glGenBuffers ( 1, &screen_buffer );
    glBindBuffer ( GL_ARRAY_BUFFER, screen_buffer );
    glBufferData ( GL_ARRAY_BUFFER, 12 * sizeof ( GLfloat ), ScreenData, GL_STATIC_DRAW );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param viewport_width ?.
* \param viewport_height ?.
* \param verbose ?.
*/
/* ******************************************************************************************************************/
static void update_texture_dimensions ( Sint32 viewport_width, Sint32 viewport_height, Sint32 verbose )
{
    for ( Sint32 i = 0; i < nb_passes; i++ )
    {
        pass_t p = &pipeline[i];

        switch ( p->scale_type_x )
        {
        case ( VIEWPORT ) :
            {
                p->dst_width = viewport_width;
            }
            break;

        case ( ABSOLUTE ) :
            {
                p->dst_width = 1;
            }
            break;

        case ( SOURCE ) :
        default:
            {
                p->dst_width = p->src_width;
            }
            break;
        }

        switch ( p->scale_type_y )
        {
        case ( VIEWPORT ) :
            {
                p->dst_height = viewport_height;
            }
            break;

        case ( ABSOLUTE ) :
            {
                p->dst_height = 1;
            }
            break;

        case ( SOURCE ) :
        default:
            {
                p->dst_height = p->src_height;
            }
            break;
        }

        p->dst_width *= p->scale_x;
        p->dst_height *= p->scale_y;

        if ( verbose )
        {
            printf ( "Pass %d output texture size: %d x %d\n",
                     i, p->dst_width, p->dst_height );
        }

        if ( i < nb_passes - 1 )
        {
            pipeline[i + 1].src_width = p->dst_width;
            pipeline[i + 1].src_height = p->dst_height;
            glBindTexture ( GL_TEXTURE_2D, pipeline[i + 1].src_texture );
            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA,
                           p->dst_width, p->dst_height, 0,
                           GL_RGBA, GL_UNSIGNED_BYTE, NULL );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param start ?.
* \param end ?.
* \return ?.
*/
/* ******************************************************************************************************************/
static char* trim ( char* start, char* end )
{
    for ( ; end - 1 >= start && ( isspace ( * ( end - 1 ) ) || * ( end - 1 ) == '"' ); end-- );

    *end = '\0';

    for ( ; start < end && ( isspace ( *start ) || *start == '"' ); start++ );

    return ( start );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param val ?.
* \param start ?.
* \return ?.
*/
/* ******************************************************************************************************************/
static Sint32 startsWith ( char* val, char* start )
{
    return strncasecmp ( start, val, strlen ( start ) ) == 0;
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param glslpname ?.
* \param start_width ?.
* \param start_height ?.
* \param window_width ?.
* \param window_height ?.
* \return ?.
*/
/* ******************************************************************************************************************/
static SDL_bool parse_shader_preset ( const char* glslpname, Sint32 start_width, Sint32 start_height, Sint32 window_width, Sint32 window_height )
{
    char* stmt = NULL;
    Sint32 pos = 0;
    Sint32 buflen = 0;
    Sint32 i = 0;
    char* /*shaderpath = NULL,*/ sbuf = NULL;
    char* glslp = NULL;

    buflen = strlen ( gngeox_config.shaderpath ) + strlen ( glslpname ) + 2;
    sbuf = qalloc ( buflen );
    snprintf ( sbuf, buflen, "%s/%s", gngeox_config.shaderpath, glslpname );
    printf ( "Loading GLSL preset %s\n", sbuf );
    glslp = load_file_and_wrap ( sbuf, "" );

    if ( glslp == NULL )
    {
        return ( SDL_FALSE );
    }

    for ( stmt = strtok ( glslp, "\n" ); stmt != NULL; stmt = strtok ( NULL, "\n" ) )
    {
        //printf("\nSTMT: |%s|\n",stmt);
        char* eqpos = strchr ( stmt, '=' );

        if ( eqpos == NULL )
        {
            continue;
        }

        char* val = trim ( eqpos + 1, stmt + strlen ( stmt ) );
        char* key = trim ( stmt, eqpos );

        //printf("KEY: |%s|\n", key);
        //printf("VAL: |%s|\n", val);
        if ( strcmp ( key, "shaders" ) == 0 )
        {
            PARSE_INT ( val, nb_passes );
            pipeline = qalloc ( nb_passes * sizeof ( struct _pass_ ) );

            for ( i = 0; i < nb_passes; i++ )
            {
                pipeline[i].scale_type_x = SOURCE;
                pipeline[i].scale_type_y = SOURCE;
                pipeline[i].scale_x = 1.0;
                pipeline[i].scale_y = 1.0;
            }

            pipeline[0].src_texture = input_tex;
            pipeline[0].src_width = start_width;
            pipeline[0].src_height = start_height;
            pipeline[0].filter_linear = SDL_FALSE;
            pipeline[nb_passes - 1].scale_type_x = VIEWPORT;
            pipeline[nb_passes - 1].scale_type_y = VIEWPORT;
            pipeline[nb_passes - 1].dst_framebuffer = 0;
            printf ( "Initializing a %d-passes shader pipeline\n", nb_passes );

            continue;
        }

        else if ( startsWith ( key, GLSLP_SHADER ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SHADER ), pos );
            GLuint shader = compile_shader_program ( val );

            if ( shader == GL_FALSE )
            {
                printf ( "Unable to compile shader program %s\n", val );
                return ( SDL_FALSE );
            }

            pipeline[pos].program = shader;
        }

        else if ( startsWith ( key, GLSLP_SCALE_TYPE_X ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE_TYPE_X ), pos );
            PARSE_SCALE ( val, pipeline[pos].scale_type_x );
        }

        else if ( startsWith ( key, GLSLP_SCALE_TYPE_Y ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE_TYPE_Y ), pos );
            PARSE_SCALE ( val, pipeline[pos].scale_type_y );
        }

        else if ( startsWith ( key, GLSLP_SCALE_TYPE ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE_TYPE ), pos );
            PARSE_SCALE ( val, pipeline[pos].scale_type_x );
            PARSE_SCALE ( val, pipeline[pos].scale_type_y );
        }

        else if ( startsWith ( key, GLSLP_SCALE_X ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE_X ), pos );
            PARSE_FLOAT ( val, pipeline[pos].scale_x );
        }

        else if ( startsWith ( key, GLSLP_SCALE_Y ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE_Y ), pos );
            PARSE_FLOAT ( val, pipeline[pos].scale_y );
        }

        else if ( startsWith ( key, GLSLP_SCALE ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_SCALE ), pos );
            PARSE_FLOAT ( val, pipeline[pos].scale_x );
            PARSE_FLOAT ( val, pipeline[pos].scale_y );
        }

        else if ( startsWith ( key, GLSLP_FILTER_LINEAR ) )
        {
            PARSE_POS ( key + strlen ( GLSLP_FILTER_LINEAR ), pos );
            pipeline[pos].filter_linear = ( strcasecmp ( val, "true" ) == 0 );
        }
    }

    // Initialize FBO and texture for each pass
    for ( i = 0; i < nb_passes; i++ )
    {
        pass_t p = &pipeline[i];
        set_texture_filter_linear ( p->src_texture, p->filter_linear );

        if ( i < nb_passes - 1 )
        {
            GLuint texture;
            GLuint framebufferid;
            create_pass_framebuffer ( &texture, &framebufferid );
            p->dst_framebuffer = framebufferid;
            pipeline[i + 1].src_texture = texture;
        }
    }

    // Set the textures' initial dimensions
    update_texture_dimensions ( window_width, window_height, SDL_TRUE );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
* \param orig_width ?.
* \param orig_height ?.
* \param src_texture ?.
* \param src_width ?.
* \param src_height ?.
* \param dst_framebuffer ?.
* \param dst_width ?.
* \param dst_height ?.
* \param program ?.
* \return ?.
*/
/* ******************************************************************************************************************/
static void render_pass ( Sint32 orig_width, Sint32 orig_height, GLuint src_texture, Sint32 src_width, Sint32 src_height, GLuint dst_framebuffer, Sint32 dst_width, Sint32 dst_height, GLuint program )
{
    glBindTexture ( GL_TEXTURE_2D, src_texture );
    glBindFramebuffer ( GL_FRAMEBUFFER, dst_framebuffer );
    GLint x = 0;
    GLint y = 0;

    if ( dst_framebuffer == 0 )
    {
        x = ( ( Sint32 ) current_window_width - dst_width ) / 2;
        y = ( ( Sint32 ) current_window_height - dst_height ) / 2;
    }

    glViewport ( x, y, dst_width, dst_height );

    glUseProgram ( program );

    // location of uniform input in the linked program
    GLuint l_VertexCoord = glGetAttribLocation ( program, "VertexCoord" );
    GLuint l_TexCoord = glGetAttribLocation ( program, "TexCoord" );
    GLuint l_TextureSize = glGetUniformLocation ( program, "TextureSize" );
    GLuint l_InputSize = glGetUniformLocation ( program, "InputSize" );
    GLuint l_OutputSize = glGetUniformLocation ( program, "OutputSize" );
    GLuint l_MVPMatrix = glGetUniformLocation ( program, "MVPMatrix" );
    // GLuint l_FrameDirection = glGetUniformLocation(program, "FrameDirection");
    // GLuint l_FrameCount = glGetUniformLocation(program, "FrameCount");

#ifdef USE_GL2
    glBindVertexArray ( vao );
#endif

    glBindBuffer ( GL_ARRAY_BUFFER, vertex_buffer );
    glVertexAttribPointer ( l_VertexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray ( l_VertexCoord );

    glBindBuffer ( GL_ARRAY_BUFFER, dst_framebuffer != 0 ? screen_buffer : tex_buffer );
    glVertexAttribPointer ( l_TexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray ( l_TexCoord );

    if ( l_TextureSize != -1 )
    {
        glUniform2f ( l_TextureSize, ( float ) orig_width, ( float ) orig_height );
    }

    if ( l_InputSize != -1 )
    {
        glUniform2f ( l_InputSize, ( float ) src_width, ( float ) src_height );
    }

    if ( l_OutputSize != -1 )
    {
        glUniform2f ( l_OutputSize, ( float ) dst_width, ( float ) dst_height );
    }

    if ( l_MVPMatrix != -1 )
    {
        glUniformMatrix4fv ( l_MVPMatrix, 1, GL_FALSE, MVPMatrix );
    }

    // if (l_FrameDirection != -1) glUniform1f(l_FrameDirection, 0);
    // if (l_FrameCount != -1) glUniform1i(l_FrameCount, 0);

    // pass previsouly registered buffers to the fragment shader
    glDrawArrays ( GL_TRIANGLES, 0, 6 );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes Glsl blitter.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool blitter_glsl_init ( void )
{
    SDL_GLContext context;
    Sint32 glsl_ret = 0;

    if ( sdl_window != NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "SDL window already initialized" );
        return ( SDL_FALSE );
    }

    // compute real window size and save it for when switching in
    // and out of fullscreen
    current_window_height = screen_rect.h * gngeox_config.scale;
    current_window_width = ( Sint32 ) ( current_window_height * screen_ratio );
    gngeox_config.res_x = current_window_width;
    gngeox_config.res_y = current_window_height;

    // the surface that will be uploaded
    input_pixels = SDL_CreateRGBSurface ( SDL_SWSURFACE, screen_rect.w, screen_rect.h,
                                          32, 0, 0, 0, 0 );

    sdl_window = SDL_CreateWindow ( "GnGeo-X",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    ( Sint32 ) current_window_width,
                                    ( Sint32 ) current_window_height,
                                    ( gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 ) |
                                    SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN |
                                    SDL_WINDOW_OPENGL );

    if ( sdl_window == NULL )
    {
        return ( SDL_FALSE );
    }

    // Configure GL or GL ES depending on the lib we depend on
#ifdef USE_GL2
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#else
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#endif

    SDL_GL_SetAttribute ( SDL_GL_DOUBLEBUFFER, 1 );
    context = SDL_GL_CreateContext ( sdl_window );
    SDL_GL_SetSwapInterval ( 1 );

#ifdef USE_GL2
    // GLEW ensures that we get access to all the GL functions
    // required for proper shader compilation
    glewExperimental = GL_TRUE;
    glewInit();
#endif
    /* printf ( "Using GL: %s\n", glGetString ( GL_VERSION ) ); */

    // Initialize the GLSL rendering pipeline
    input_tex = create_input_texture();
    init_static_gl_coords_buffers();
    /* @fixme (Tmesys#1#15/10/2023): shaderpath is in fact shader file name */
    glsl_ret = parse_shader_preset ( gngeox_config.shaderpath,
                                     screen_rect.w, screen_rect.h,
                                     current_window_width, current_window_height );

    if ( glsl_ret == SDL_FALSE )
    {
        printf ( "Could not initialize GLSL shaders\n" );
        return SDL_FALSE;
    }

    SDL_ShowWindow ( sdl_window );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Resizes Glsl viewport.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool blitter_glsl_resize ( Sint32 w, Sint32 h )
{
    Sint32 viewport_width = w, viewport_height = h;
    current_window_width = w;
    current_window_height = h;

    /* @fixme (Tmesys#1#15/10/2023): Undocumented option ? */
    /*
        if ( CF_BOOL ( cf_get_item_by_name ( "resize" ) ) )
        {
            if ( ( ( float ) w / h ) >= screen_ratio )
            {
                viewport_width = ( Sint32 ) ( h * screen_ratio );
            }
            else
            {
                viewport_height = ( Sint32 ) ( w / screen_ratio );
            }

            update_texture_dimensions ( viewport_width, viewport_height, SDL_FALSE );
        }
    */
    /* @fixme (Tmesys#1#12/25/2022): Only need to clear buffer during resize because the viewport may be smaller than the window. */
    clear_after_resize = 2;

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates Glsl blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_glsl_update ( void )
{
    // GnGeo renders graphics in a big internal 2D buffer. Pixels are
    // written in the "visible area", a 320x224 sub-rect with a
    // (16,16) offset from the start of the internal buffer.
    //
    // A lot of ROMs crop the screen to 304x224, at the center of the
    // visible area (represented by screen_rect).
    //
    // viewport_rect is the final visible area in GnGeo's internal 2D
    // buffer.
    SDL_Rect viewport_rect = screen_rect;
    viewport_rect.x += visible_area.x;
    viewport_rect.y += visible_area.y;
    SDL_BlitSurface ( sdl_surface_buffer, &viewport_rect, input_pixels, NULL );

    if ( clear_after_resize > 0 )
    {
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        glClearColor ( 0.0, 0.0, 0.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        clear_after_resize--;
    }

    glActiveTexture ( GL_TEXTURE0 );
    glBindTexture ( GL_TEXTURE_2D, input_tex );
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, screen_rect.w, screen_rect.h,
                   0, GL_BGRA, GL_UNSIGNED_BYTE, input_pixels->pixels );

    // render: TEX -> (pass1_prog) -> FBO -> ... -> (pass2_prog) -> OUTPUT
    for ( Sint32 i = 0; i < nb_passes; i++ )
    {
        pass_t p = &pipeline[i];
        render_pass ( screen_rect.w, screen_rect.h,
                      p->src_texture, p->src_width, p->src_height,
                      p->dst_framebuffer, p->dst_width, p->dst_height,
                      p->program );
    }

    SDL_GL_SwapWindow ( sdl_window );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes Glsl blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_glsl_close ( void )
{
    //if (sdl_surface_screen != NULL)
    //  SDL_FreeSurface(sdl_surface_screen);
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets Glsl blitter in full screen mode.
*
*/
/* ******************************************************************************************************************/
void blitter_glsl_fullscreen ( void )
{
    SDL_SetWindowFullscreen ( sdl_window,
                              gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );

    if ( gngeox_config.fullscreen )
    {
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode ( sdl_window, &mode );
        blitter_glsl_resize ( mode.w, mode.h );
    }
    else
    {
        Sint32 w, h;
        SDL_GetWindowSize ( sdl_window, &w, &h );
        blitter_glsl_resize ( gngeox_config.res_x, gngeox_config.res_y );
    }
}
