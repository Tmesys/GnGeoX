/*****************************************************************************/
/*     Generator - Sega Genesis emulation - (c) James Ponder 1997-2001       */
/*****************************************************************************/
/*                                                                           */
/* diss68k.h                                                                  */
/*                                                                           */
/*****************************************************************************/

int diss68k_gettext ( t_ipc* ipc, char* text );
void diss68k_getoperand ( char* text, t_ipc* ipc, t_iib* iib, t_type type );
void diss68k_getdumpline ( uint32 addr68k, char* dumpline );
