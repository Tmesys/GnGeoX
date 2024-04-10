/*!
*
*   \file GnGeoXcontroller.c
*   \brief   .
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
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

#include "GnGeoXcontroller.h"
#include "GnGeoXvideo.h"
#include "GnGeoXroms.h"
#include "GnGeoXmemory.h"
#include "GnGeoXscreen.h"
#include "GnGeoXconfig.h"

struct_gngeoxcontroller_player players[CONTROLLER_PLAYER_MAX];
/* ******************************************************************************************************************/
/*!
* \brief  Initializes event system.
*
*/
/* ******************************************************************************************************************/
static void update_controllers_button ( enum_gngeoxcontroller_player player, enum_gngeoxcontroller_button_state state, enum_gngeoxcontroller_button button )
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
SDL_bool neo_controllers_init ( void )
{
    Uint32 index = 0;
    char guid_string[100];
    char * mapping_string = NULL;

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
            players[index].controller = SDL_GameControllerOpen ( loop );
            if ( players[index].controller == NULL )
            {
                zlog_error ( gngeox_config.loggingCat, "Open Game controller number %d, %s", loop, SDL_GetError() );
                return ( SDL_FALSE );
            }

            players[index].controller_id = SDL_JoystickInstanceID ( SDL_GameControllerGetJoystick ( players[index].controller ) );
            players[index].controller_guid = SDL_JoystickGetDeviceGUID ( loop );
            SDL_JoystickGetGUIDString ( players[index].controller_guid, &guid_string, 100 );

            mapping_string = SDL_GameControllerMappingForGUID ( players[index].controller_guid );
            if ( mapping_string == NULL )
            {
                zlog_error ( gngeox_config.loggingCat, "Mapping not found for Game controller number %d, %s", loop, SDL_GetError() );
                return ( SDL_FALSE );
            }

            /* Initial values */
            neogeo_memory.p1cnt = 0xFF;
            neogeo_memory.p2cnt = 0xFF;
            //neogeo_memory.status_a = 0x7;
            neogeo_memory.status_a = 0;
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P1 );
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_COIN_P2 );
            QBIT_SET ( neogeo_memory.status_a, STATUS_A_SERVICE_P2 );

            //neogeo_memory.status_b = 0x8F;
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
                    zlog_error ( gngeox_config.loggingCat, "Unknown system type configuration value %d", gngeox_config.systemtype );
                }
                break;
            }


            zlog_info ( gngeox_config.loggingCat, "Open Game controller number %d : %s", loop, SDL_GameControllerName ( players[index].controller ) );
            zlog_info ( gngeox_config.loggingCat, "-> Vendor %x - product %x", SDL_GameControllerGetVendor ( players[index].controller ), SDL_GameControllerGetProduct ( players[index].controller ) );
            zlog_info ( gngeox_config.loggingCat, "-> Guid : %s", guid_string );
            zlog_info ( gngeox_config.loggingCat, "-> Mapping : %s", mapping_string );

            SDL_free ( mapping_string );

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
void neo_controllers_dispatch ( enum_gngeoxcontroller_button_state state, SDL_JoystickID controller_id, Uint8 button )
{
    for ( int loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
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
void neo_controllers_close ( void )
{
    for ( int loop = 0; loop < CONTROLLER_PLAYER_MAX; loop++ )
    {
        if ( players[loop].controller != NULL )
        {
            SDL_GameControllerClose ( players[loop].controller );
        }
    }
}

#ifdef _GNGEOX_CONTROLLER_C_
#undef _GNGEOX_CONTROLLER_C_
#endif // _GNGEOX_CONTROLLER_C_
