/*!
*
*   \file GnGeoXcontroller.c
*   \brief   .
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date 16/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_CONTROLLER_C_
#define _GNGEOX_CONTROLLER_C_
#endif // _GNGEOX_CONTROLLER_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"

#include "GnGeoXvideo.h"
#include "GnGeoXroms.h"
#include "GnGeoXmemory.h"
#include "GnGeoXscreen.h"
#include "GnGeoXconfig.h"
#include "GnGeoXcontroller.h"

struct_gngeoxcontroller_player players[CONTROLLER_PLAYER_MAX];
const Sint32 direction_vectors_2[8][2] =
{
    /* this is left */
    {-32767, -16384},
    /* this is up */
    {-16384, -32767},
    /* this is right */
    {32767, -16384},
    /* this is up */
    {16384, -32767},
    /* this is bottom */
    {-16384, 32767},
    /* this is left */
    {-32767, 16384},
    /* this is bottom */
    {16384, 32767},
    /* this is right */
    {32767, 32767}
};
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
static void update_controllers_button ( enum_gngeoxcontroller_player player, enum_gngeoxcontroller_button_state state, enum_gngeoxmemory_reg_pcnt button )
{
    switch ( state )
    {
    case ( CONTROLLER_STATE_DOWN ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.p1cnt, button );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.p2cnt, button );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
    case ( CONTROLLER_STATE_UP ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_SET ( neogeo_memory.p1cnt, button );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_SET ( neogeo_memory.p2cnt, button );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
        /*
            default:
                {
                }
                break;
        */
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
static void update_controllers_start ( enum_gngeoxcontroller_player player, enum_gngeoxcontroller_button_state state )
{
    switch ( state )
    {
    case ( CONTROLLER_STATE_DOWN ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.status_b, STATUS_B_START_P1 );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.status_b, STATUS_B_START_P2 );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
    case ( CONTROLLER_STATE_UP ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_SET ( neogeo_memory.status_b, STATUS_B_START_P1 );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_SET ( neogeo_memory.status_b, STATUS_B_START_P2 );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
        /*
            default:
                {
                }
                break;
        */
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
static void update_controllers_coin_select ( enum_gngeoxcontroller_player player, enum_gngeoxcontroller_button_state state )
{
    switch ( state )
    {
    case ( CONTROLLER_STATE_DOWN ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.status_a, STATUS_A_COIN_P1 );
                    QBIT_CLEAR ( neogeo_memory.status_b, STATUS_B_SELECT_P1 );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_CLEAR ( neogeo_memory.status_a, STATUS_A_COIN_P2 );
                    QBIT_CLEAR ( neogeo_memory.status_b, STATUS_B_SELECT_P2 );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
    case ( CONTROLLER_STATE_UP ) :
        {
            switch ( player )
            {
            case ( CONTROLLER_PLAYER_1 ) :
                {
                    QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P1 );
                    QBIT_SET ( neogeo_memory.status_b, STATUS_B_SELECT_P1 );
                }
                break;
            case ( CONTROLLER_PLAYER_2 ) :
                {
                    QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P2 );
                    QBIT_SET ( neogeo_memory.status_b, STATUS_B_SELECT_P2 );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unknown player" );
                }
                break;
            }
        }
        break;
        /*
            default:
                {
                }
                break;
        */
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
static SDL_bool neo_controllers_open ( Uint32 player_index, SDL_JoystickID controller_id )
{
    char guid_string[100];
    char * mapping_string = NULL;

    players[player_index].controller = SDL_GameControllerOpen ( controller_id );
    if ( players[player_index].controller == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Open Game controller number %d, %s", controller_id, SDL_GetError() );
        return ( SDL_FALSE );
    }

    players[player_index].controller_id = SDL_JoystickInstanceID ( SDL_GameControllerGetJoystick ( players[player_index].controller ) );
    players[player_index].controller_guid = SDL_JoystickGetDeviceGUID ( controller_id );
    SDL_JoystickGetGUIDString ( players[player_index].controller_guid, &guid_string, 100 );

    mapping_string = SDL_GameControllerMappingForGUID ( players[player_index].controller_guid );
    if ( mapping_string == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Mapping not found for Game controller number %d, %s", controller_id, SDL_GetError() );
        return ( SDL_FALSE );
    }

    zlog_info ( gngeox_config.loggingCat, "Game controller number %d : %s", controller_id, SDL_GameControllerName ( players[player_index].controller ) );
    zlog_info ( gngeox_config.loggingCat, "-> Vendor %x - product %x", SDL_GameControllerGetVendor ( players[player_index].controller ), SDL_GameControllerGetProduct ( players[player_index].controller ) );
    zlog_info ( gngeox_config.loggingCat, "-> Guid : %s", guid_string );
    zlog_info ( gngeox_config.loggingCat, "-> Mapping : %s", mapping_string );

    SDL_free ( mapping_string );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_controllers_init ( void )
{
    Uint32 index = 0;

    if ( SDL_GameControllerAddMappingsFromFile ( "./gamecontrollerdb.txt" ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    for ( Sint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        players[loop].controller_id = -1;
    }

    for ( Sint32 loop = 0; loop < SDL_NumJoysticks(); loop++ )
    {
        if ( SDL_IsGameController ( loop ) )
        {
            if ( neo_controllers_open ( index, loop ) == SDL_FALSE )
            {
                return ( SDL_FALSE );
            }

            /* Initial values */
            neogeo_memory.p1cnt = 0xFF;
            neogeo_memory.p2cnt = 0xFF;

            neogeo_memory.status_a = 0;
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P1 );
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P2 );
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_SERVICE_P2 );

            neogeo_memory.status_b = 0;
            QBIT_SET ( neogeo_memory.status_b, STATUS_B_START_P1 );
            QBIT_SET ( neogeo_memory.status_b, STATUS_B_SELECT_P1 );
            QBIT_SET ( neogeo_memory.status_b, STATUS_B_START_P2 );
            QBIT_SET ( neogeo_memory.status_b, STATUS_B_SELECT_P2 );
            QBIT_SET ( neogeo_memory.status_b, STATUS_B_SYS );
            switch ( gngeox_config.systemtype )
            {
            case ( SYS_ARCADE_MVS ) :
                {
                    QBIT_SET ( neogeo_memory.status_b, STATUS_B_SYS );
                }
                break;
            case ( SYS_HOME_AES ) :
                {
                    QBIT_CLEAR ( neogeo_memory.status_b, STATUS_B_SYS );
                }
                break;
            default:
                {
                    zlog_warn ( gngeox_config.loggingCat, "Unknown system type configuration value %d", gngeox_config.systemtype );
                }
                break;
            }

            index++;
            if ( index >= CONTROLLER_PLAYER_MAX )
            {
                break;
            }
        }
    }
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_controllers_plug ( SDL_JoystickID controller_id )
{
    Sint32 empty_slot = -1;

    if ( SDL_IsGameController ( controller_id ) == SDL_FALSE )
    {
        return ( SDL_TRUE );
    }

    /* Check if already connected */
    for ( Uint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller_id == controller_id )
        {
            zlog_warn ( gngeox_config.loggingCat, "Game controller already connected %d", controller_id );
            return ( SDL_TRUE );
        }
    }

    /* find an empty player slot */
    for ( Uint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller_id == -1 )
        {
            empty_slot = loop;
            break;
        }
    }

    if ( empty_slot == -1 )
    {
        zlog_warn ( gngeox_config.loggingCat, "All game controller slots connected" );
        return ( SDL_TRUE );
    }

    if ( neo_controllers_open ( empty_slot, controller_id ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_controllers_unplug ( SDL_JoystickID controller_id )
{
    /* Check if already connected */
    for ( Uint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller_id == controller_id )
        {
            SDL_GameControllerClose ( players[loop].controller );
            players[loop].controller_id = -1;
            SDL_zero ( players[loop].controller_guid );
            return ( SDL_TRUE );
        }
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
void neo_controllers_update ( enum_gngeoxcontroller_button_state state, SDL_JoystickID controller_id, Uint8 button )
{
    for ( Uint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller_id == controller_id )
        {
            switch ( button )
            {
            case ( SDL_CONTROLLER_BUTTON_BACK ) :
                {
                    update_controllers_coin_select ( loop, state );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_START ) :
                {
                    update_controllers_start ( loop, state );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_A ) :
                {
                    update_controllers_button ( loop, state, PCNT_A );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_B ) :
                {
                    update_controllers_button ( loop, state, PCNT_B );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_X ) :
                {
                    update_controllers_button ( loop, state, PCNT_C );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_Y ) :
                {
                    update_controllers_button ( loop, state, PCNT_D );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_DPAD_UP ) :
                {
                    update_controllers_button ( loop, state, PCNT_UP );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_DPAD_DOWN ) :
                {
                    update_controllers_button ( loop, state, PCNT_DOWN );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_DPAD_LEFT ) :
                {
                    update_controllers_button ( loop, state, PCNT_LEFT );
                }
                break;
            case ( SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) :
                {
                    update_controllers_button ( loop, state, PCNT_RIGHT );
                }
                break;
            default:
                {
                    zlog_error ( gngeox_config.loggingCat, "Unhandled controller button : %d", button );
                }
                break;
            }
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
void neo_controllers_update_axis ( SDL_JoystickID controller_id, Uint8 axis, Sint16 value )
{
    Sint32 player_id = -1;

    for ( Uint32 loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller_id == controller_id )
        {
            if ( axis == SDL_CONTROLLER_AXIS_LEFTX )
            {
                players[loop].axis0_x_value = value;
            }
            if ( axis == SDL_CONTROLLER_AXIS_LEFTY )
            {
                players[loop].axis1_y_value = value;
            }

            player_id = loop;

            break;
        }
    }

    update_controllers_button ( player_id, CONTROLLER_STATE_UP, PCNT_UP );
    update_controllers_button ( player_id, CONTROLLER_STATE_UP, PCNT_DOWN );
    update_controllers_button ( player_id, CONTROLLER_STATE_UP, PCNT_RIGHT );
    update_controllers_button ( player_id, CONTROLLER_STATE_UP, PCNT_LEFT );

    if ( player_id == -1 )
    {
        return;
    }


    /*
    @note (Tmesys#1#14/04/2024):
    Let's do some math and geometry : The idea is to identify wich quarter of the square
    tho stick is in, then consider this quarter as a new square to identify if the stick is on the right or
    the left side of its diagonal. So we will be using vectors and scalars.

    The square representation looks like a rectangle, but let's imagine somehow
    it's a square :

          (Y)
    A------B------A1
    |*     |     *|
    | * P1 |    * |
    |  *   |   *  |
    |   ###|###   |
    |   #* | *#   |
    |   # *|* #   |
    D---#--C--#X--D1 (X)
    |     *|*     |
    |    * | *    |
    |   *  |  *   |
    |  *   |   *  |
    | *    |    * |
    |*     |     *|
    A3------B'-----A2

    Considering the upper left square :
    A(-32767,-32767) and c(0,0)
    CA(-32767-0,-32767-0) = CA(-32767,-32767)

    P1 is where the stick is :
    P1(x,y) so CP(x-0,y-0)= CP(x,y)

    Vectorial 2D product is :

    CA*CP = (-32767 * y) - (-32767 * x)
    In the direction from A to C :
    Negative result : P is on the right
    Positive result : P is on the left
    Zero : P is in the diagonale

    UL CA is (-32767,-32767)
    UR CA is (32767,-32767)
    BL CA is (-32767,32767)
    BR CA is (32767,32767)

    Of course, we gonna make things more complicated ;)
    */

    if (
        ( players[player_id].axis0_x_value >= -CONTROLLER_DEAD_ZONE
          && players[player_id].axis0_x_value <= CONTROLLER_DEAD_ZONE ) &&
        ( players[player_id].axis1_y_value >= -CONTROLLER_DEAD_ZONE
          && players[player_id].axis1_y_value <= CONTROLLER_DEAD_ZONE )
    )
    {
        //zlog_info ( gngeox_config.loggingCat, "CENTER" );
        return;
    }

    /* direction UP / LEFT */
    if ( players[player_id].axis0_x_value <= 0
            && players[player_id].axis1_y_value <= 0 )
    {
        Sint32 product = 0;
        product = ( direction_vectors_2[DIR_UL_L][VECT_X] * players[player_id].axis1_y_value )
                  - ( direction_vectors_2[DIR_UL_L][VECT_Y] * players[player_id].axis0_x_value );

        if ( product < 0 )
        {
            //zlog_info ( gngeox_config.loggingCat, "LEFT" );
            update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_LEFT );
        }
        else
        {
            product = ( direction_vectors_2[DIR_UL_U][VECT_X] * players[player_id].axis1_y_value )
                      - ( direction_vectors_2[DIR_UL_U][VECT_Y] * players[player_id].axis0_x_value );
            if ( product >= 0 )
            {
                //zlog_info ( gngeox_config.loggingCat, "UP" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_UP );
            }
            else
            {
                //zlog_info ( gngeox_config.loggingCat, "UP&LEFT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_LEFT );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_UP );
            }
        }
        return;
    }

    /* direction UP / RIGHT */
    if ( players[player_id].axis0_x_value >= 0
            && players[player_id].axis1_y_value <= 0 )
    {
        Sint32 product = 0;
        product = ( direction_vectors_2[DIR_UR_R][VECT_X] * players[player_id].axis1_y_value )
                  - ( direction_vectors_2[DIR_UR_R][VECT_Y] * players[player_id].axis0_x_value );
        if ( product >= 0 )
        {
            //zlog_info ( gngeox_config.loggingCat, "RIGHT" );
            update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_RIGHT );
        }
        else
        {
            product = ( direction_vectors_2[DIR_UR_U][VECT_X] * players[player_id].axis1_y_value )
                      - ( direction_vectors_2[DIR_UR_U][VECT_Y] * players[player_id].axis0_x_value );

            if ( product < 0 )
            {
                //zlog_info ( gngeox_config.loggingCat, "UP" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_UP );
            }
            else
            {
                //zlog_info ( gngeox_config.loggingCat, "UP&RIGHT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_RIGHT );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_UP );
            }
        }
        return;
    }

    /* direction BOTTOM / LEFT */
    if ( players[player_id].axis0_x_value <= 0
            && players[player_id].axis1_y_value >= 0 )
    {
        Sint32 product = 0;
        product = ( direction_vectors_2[DIR_BL_B][VECT_X] * players[player_id].axis1_y_value )
                  - ( direction_vectors_2[DIR_BL_B][VECT_Y] * players[player_id].axis0_x_value );
        if ( product < 0 )
        {
            //zlog_info ( gngeox_config.loggingCat, "BOTTOM" );
            update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_DOWN );
        }
        else
        {
            product = ( direction_vectors_2[DIR_BL_L][VECT_X] * players[player_id].axis1_y_value )
                      - ( direction_vectors_2[DIR_BL_L][VECT_Y] * players[player_id].axis0_x_value );
            if ( product >= 0 )
            {
                //zlog_info ( gngeox_config.loggingCat, "LEFT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_LEFT );
            }
            else
            {
                //zlog_info ( gngeox_config.loggingCat, "BOTTOM&LEFT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_DOWN );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_LEFT );
            }
        }
        return;
    }

    /* direction BOTTOM / RIGHT */
    if ( players[player_id].axis0_x_value >= 0
            && players[player_id].axis1_y_value >= 0 )
    {
        Sint32 product = 0;
        product = ( direction_vectors_2[DIR_BR_B][VECT_X] * players[player_id].axis1_y_value )
                  - ( direction_vectors_2[DIR_BR_B][VECT_Y] * players[player_id].axis0_x_value );
        if ( product >= 0 )
        {
            //zlog_info ( gngeox_config.loggingCat, "BOTTOM" );
            update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_DOWN );
        }
        else
        {
            product = ( direction_vectors_2[DIR_BR_R][VECT_X] * players[player_id].axis1_y_value )
                      - ( direction_vectors_2[DIR_BR_R][VECT_Y] * players[player_id].axis0_x_value );
            if ( product < 0 )
            {
                //zlog_info ( gngeox_config.loggingCat, "RIGHT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_RIGHT );
            }
            else
            {
                //zlog_info ( gngeox_config.loggingCat, "BOTTOM&RIGHT" );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_DOWN );
                update_controllers_button ( player_id, CONTROLLER_STATE_DOWN, PCNT_RIGHT );
            }
        }
        return;
    }

    zlog_warn ( gngeox_config.loggingCat, "Undetected x=%d y=%d", players[player_id].axis0_x_value, players[player_id].axis1_y_value );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
void neo_controllers_close ( void )
{
    for ( int loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller != NULL )
        {
            SDL_GameControllerClose ( players[loop].controller );
            players[loop].controller_id = -1;
        }
    }
}

#ifdef _GNGEOX_CONTROLLER_C_
#undef _GNGEOX_CONTROLLER_C_
#endif // _GNGEOX_CONTROLLER_C_
