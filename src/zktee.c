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
"  Create a new NODE without content.\n";

int main(int argc, char **argv) {
  int rc;
  size_t content_size = 0;
  char *content = NULL;
  struct Stat stat;

  zksh_init(&argc, argv);

  // Validate user input
  if (argc-optind != 1) {
    fprintf(stderr, "%s: Exactly one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }
  if (zksh_check_nodepath(argv[optind]) != 0)
    exit(EXIT_FAILURE);

  // Read stdin to memory
  do {
    content = realloc(content, sizeof(char) * (content_size + 1024));

    if(content == NULL) {
      fprintf(stderr, "%s: realloc failed\n", zksh_program);
      exit(EXIT_FAILURE);
    }

    rc = fread(content+content_size, sizeof(char), 1024, stdin);

    content_size += rc;
  } while(rc == 1024);

  if(ferror(stdin)) {
    fprintf(stderr, "%s: Error ocured while reading stdin.\n", zksh_program);
  }

  zksh_connect();

  // Test for file precense
  rc = zoo_exists(zh, argv[optind], 0, &stat);
  zk_check_rc(rc, argv[optind]);
  if (rc != ZOK)
    exit(zk_return_code);

  // Write to node
  rc = zoo_set(zh, argv[optind], content, content_size, stat.version);
  zk_check_rc(rc, argv[optind]);
  if (rc != ZOK)
    exit(zk_return_code);

  // Write to stdout
  rc = write(1, content, content_size);
  if (rc != content_size){
    fprintf(stderr, "%s: Only %d bytes written to stdout\n", zksh_program, rc);
  }

  free(content);

  exit(zk_return_code);
}
