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

#include "zksh.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

char *usage_description = \
"  Delete NODEs from zookeeper.\n";

int main(int argc, char **argv) {
  int rc;
  struct Stat stat;

  zksh_init(&argc, argv);

  // Validate user input
  if (argc <= optind) {
    fprintf(stderr, "%s: At least one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }

  zksh_connect();

  for(; optind < argc; optind++) {
    // Validate node path
    if (zksh_check_nodepath(argv[optind]) != 0) {
      zk_return_code = EXIT_FAILURE;
      continue;
    }

    // Test for file precense
    rc = zoo_exists(zh, argv[optind], 0, &stat);
    zk_check_rc(rc, argv[optind]);
    if (rc != ZOK)
      continue;

    // Delete Node
    rc = zoo_delete(zh, argv[optind], stat.version);
    zk_check_rc(rc, argv[optind]);
  }

  exit(zk_return_code);
}
