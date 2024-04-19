/*!
*
*   \file    GnGeoXcontroller.h
*   \brief   .
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    16/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_CONTROLLER_H_
#define _GNGEOX_CONTROLLER_H_

#define CONTROLLER_DEAD_ZONE 128


typedef enum
{
    CONTROLLER_STATE_UP = 0,
    CONTROLLER_STATE_DOWN,
    CONTROLLER_MAX_STATE,
} enum_gngeoxcontroller_button_state;

typedef enum
{
    CONTROLLER_PLAYER_1 = 0,
    CONTROLLER_PLAYER_2 = 1,
    CONTROLLER_PLAYER_MAX = 2,
} enum_gngeoxcontroller_player;

typedef enum
{
    VECT_X = 0,
    VECT_Y = 1,
} enum_gngeoxcontroller_vcord;

typedef enum
{
    DIR_UL_L = 0,
    DIR_UL_U = 1,
    DIR_UR_R = 2,
    DIR_UR_U = 3,
    DIR_BL_B = 4,
    DIR_BL_L = 5,
    DIR_BR_B = 6,
    DIR_BR_R = 7,
} enum_gngeoxcontroller_dirindex;

typedef struct
{
    SDL_GameController *controller;
    SDL_JoystickID controller_id;
    SDL_JoystickGUID controller_guid;
    Sint16 axis0_x_value;
    Sint16 axis1_y_value;
} struct_gngeoxcontroller_player;

#ifdef _GNGEOX_CONTROLLER_C_
static void update_controllers_button ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state, enum_gngeoxmemory_reg_pcnt );
static void update_controllers_start ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state );
static void update_controllers_coin_select ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state );
static SDL_bool neo_controllers_open ( Uint32, SDL_JoystickID );
#endif // _GNGEOX_CONTROLLER_C_

SDL_bool neo_controllers_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool neo_controllers_plug ( SDL_JoystickID );
SDL_bool neo_controllers_unplug ( SDL_JoystickID );
void neo_controllers_update ( enum_gngeoxcontroller_button_state, SDL_JoystickID, Uint8 );
void neo_controllers_update_axis ( SDL_JoystickID, Uint8, Sint16 );
void neo_controllers_close ( void );

#endif // _GNGEOX_CONTROLLER_H_
