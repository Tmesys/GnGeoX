/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_category_table_h
#define __zlog_category_table_h

#include "zc_defs.h"
#include "category.h"

zc_hashtable_t * zlog_category_table_new ( void );
void zlog_category_table_del ( zc_hashtable_t * );
#ifdef _DEBUG_ZLOG
void zlog_category_table_profile ( zc_hashtable_t *, int );
#endif // _DEBUG_ZLOG
/* if none, create new and return */
zlog_category_t * zlog_category_table_fetch_category ( zc_hashtable_t *, const char *, zc_arraylist_t * );

int zlog_category_table_update_rules ( zc_hashtable_t *, zc_arraylist_t * );
void zlog_category_table_commit_rules ( zc_hashtable_t * );
void zlog_category_table_rollback_rules ( zc_hashtable_t * );

#endif
