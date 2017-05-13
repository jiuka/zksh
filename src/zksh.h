/* 
 *  zksh - ZooKeeper Shell utils.
 *
 *  Copyright (C) 2017 Marius Rieder
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"
#include <zookeeper/zookeeper.h>

#ifndef _ZKSH_H
#define _ZKSH_H

#ifndef HAVE_SECURE_GETENV
#  ifdef HAVE___SECURE_GETENV
#    define secure_getenv __secure_getenv
#  else
#    define secure_getenv getenv
#  endif
#endif

/**
 * @brief Storage for the actual programm called.
 */
extern const char *zksh_program;

/**
 * @brief Storage for the zookeeper servers.
 */
extern const char *zksh_server;

/**
 * @brief Optional acl to add to created nodes.
 */
extern const char *zksh_acl;

/**
 * @brief Optional auth to use with zookeeper.
 */
extern const char *zksh_auth;

extern struct String_vector zksh_node_list;

extern int zksh_loglevel;

extern zhandle_t *zh;

extern char *usage_description;

/**
 * @brief Initialize the zksh library
 */
int zksh_init();
int zksh_connect();

extern int zk_return_code;
void zk_check_rc(int rc, char *node);

int zksh_check_nodepath(char *node);


void qsort_strings(struct String_vector *strings);

void zk_usage(int status);

char *zksh_join_path(const char *path, const char *node);

struct ACL_vector *zksh_get_acl();

#endif /* _ZKSH_H */
