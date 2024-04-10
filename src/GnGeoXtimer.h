/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#define MAX_TIMER 3

typedef struct struct_gngeoxtimer_timer struct_gngeoxtimer_timer;
struct struct_gngeoxtimer_timer
{
    double time;
    Uint32 odo_debut;
    Uint32 nb_cycle;
    Sint32 param;
    Uint32 del_it;
    void ( *func ) ( Sint32 param );
    struct_gngeoxtimer_timer* next;
};

//extern double timer_count;

struct_gngeoxtimer_timer* insert_timer ( double, Sint32, void ( *func ) ( Sint32 ) );
void del_timer ( struct_gngeoxtimer_timer* );
void my_timer ( void );
double timer_get_time ( void );
void free_all_timer ( void );

#endif
