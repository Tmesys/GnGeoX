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

typedef enum
{
    PCNT_UP = 0,
    PCNT_DOWN = 1,
    PCNT_LEFT = 2,
    PCNT_RIGHT = 3,
    PCNT_A = 4,
    PCNT_B = 5,
    PCNT_C = 6,
    PCNT_D = 7,
} enum_gngeoxcontroller_button;

typedef enum
{
    STATUS_B_START_P1 = 0,
    STATUS_B_SELECT_P1 = 1,
    STATUS_B_START_P2 = 2,
    STATUS_B_SELECT_P2 = 3,
    /* Memory card inserted if 00 */
    STATUS_B_MEMCRD_INSERT_1 = 4,
    STATUS_B_MEMCRD_INSERT_2 = 5,
    /* Memory card write protected if 1 */
    STATUS_B_MEMCRD_PROT = 6,
    /* 0:AES / 1:MVS */
    STATUS_B_SYS = 7,
} enum_gngeoxcontroller_start_select;

typedef enum
{
    STATUS_A_COIN_P1 = 0,
    STATUS_A_COIN_P2 = 1,
    STATUS_A_SERVICE_P2 = 2,
    STATUS_A_COIN_P3 = 3,
    STATUS_A_COIN_P4 = 4,
    STATUS_A_SLOT_P4 = 5,
    STATUS_A_RTC_PULSE = 6,
    STATUS_A_RTC_DATA = 7,
} enum_gngeoxcontroller_reg_status_a;

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

typedef struct
{
    SDL_GameController *controller;
    SDL_JoystickID controller_id;
    SDL_JoystickGUID controller_guid;
} struct_gngeoxcontroller_player;

#ifdef _GNGEOX_CONTROLLER_C_
static void update_controllers_button ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state, enum_gngeoxcontroller_button );
static void update_controllers_start ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state );
static void update_controllers_coin_select ( enum_gngeoxcontroller_player, enum_gngeoxcontroller_button_state );
#endif // _GNGEOX_CONTROLLER_C_

SDL_bool neo_controllers_init ( void ) __attribute__ ( ( warn_unused_result ) );
void neo_controllers_dispatch ( enum_gngeoxcontroller_button_state, SDL_JoystickID, Uint8 );
void neo_controllers_close ( void );

#endif // _GNGEOX_CONTROLLER_H_
