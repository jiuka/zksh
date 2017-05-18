/* 
 *  zksh - ZooKeeper Shell utils.
 *
 *  Copyright (C) 2017 Marius Rieder
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
