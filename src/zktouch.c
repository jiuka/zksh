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
#include "zkeproxy.h"
#include <sys/prctl.h>

#include <getopt.h>

char *usage_description = \
"  Create a new NODE without content.\n"
"\n"
"  Options:\n"
"    -e, --ephemeral       create a ephemeral node\n";

static struct option const long_options[] = {
  {"ephemeral", no_argument, 0, 'e'},
  {NULL, 0, NULL, 0}
};

int main(int argc, char **argv) {
  int c;
  int i;
  int rc;
  int node_flag = 0;
  int ephemeral = 0;

  zksh_init(&argc, argv);

  while ((c = getopt_long(argc, argv, "e", long_options, (int *) 0)) != EOF) {
    switch(c) {
      case 'e':
        ephemeral = 1;
        node_flag = ZOO_EPHEMERAL;
        break;
      default:
        zk_usage(EXIT_FAILURE);
    }
  }

  // Validate user input
  if (argc <= optind) {
    fprintf(stderr, "%s: At least one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }

  if (ephemeral == 1)
    zksh_eproxy_init();

  zksh_connect();

  for(i=optind; i<argc; i++) {
    // Validate node path
    if (zksh_check_nodepath(argv[i]) != 0)
      continue;

    rc = zoo_create(zh, argv[optind], NULL, -1, zksh_get_acl(), node_flag, NULL, -1);
    zk_check_rc(rc, argv[i]);
  }

  if (ephemeral == 1 && zk_return_code == EXIT_SUCCESS) {
    zksh_eproxy_detach();
    zksh_eproxy_wait();
  }

  exit(zk_return_code);
}
