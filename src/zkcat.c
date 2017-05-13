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
#include <string.h>

char *usage_description = \
"  Print NODEs content to standard output.\n";

int main(int argc, char **argv) {
  zksh_init(&argc, argv);

  // Validate user input
  if (argc <= optind) {
    fprintf(stderr, "%s: At least one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }

  zksh_connect();

  char *buffer;
  int buffer_len, rc;
  struct Stat stat;

  for(; optind<argc; optind++) {
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

    // Prepare buffer
    buffer_len = stat.dataLength;
    buffer = malloc(buffer_len);

    // Reade node
    rc = zoo_get(zh, argv[optind], 0, buffer, &buffer_len, NULL);
    zk_check_rc(rc, argv[optind]);
    if (rc != ZOK)
      continue;

    // Output data
    rc = write(1, buffer, buffer_len);
    if (rc != buffer_len) {
      fprintf(stderr, "%s: %s: Only %d bytes written to stdout\n", zksh_program, argv[optind], rc);
    }
    free(buffer);
  }

  exit(zk_return_code);
}
