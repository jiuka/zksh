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
#include <time.h>
#include <sys/types.h>

char *usage_description = \
"  List information about the nodes in the NODE.\n"
"\n"
"  Options:\n"
"    -l, --long            use a long listing format\n";

static struct option const long_options[] = {
  {"long", no_argument, 0, 'l'},
  {NULL, 0, NULL, 0}
};

int strings_cmp(const void *a, const void *b) {
  const char **ia = (const char **)a;
  const char **ib = (const char **)b;
  return strcmp(*ia, *ib);
}

int main(int argc, char **argv) {
  zksh_init(&argc, argv);

  int c;
  int i;
  int opt_l = 0;

  while ((c = getopt_long(argc, argv, "hl", long_options, (int *) 0)) != EOF) {
    switch(c) {
      case 'l':
        opt_l = 1;
        break;
      default:
        zk_usage(EXIT_FAILURE);
    }
  }

  // Validate user input
  if (argc-optind != 1) {
    fprintf(stderr, "%s: Exactly one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }
  if (zksh_check_nodepath(argv[optind]) != 0)
    exit(EXIT_FAILURE);

  zksh_connect();

  int rc;
  struct String_vector strings;
  struct ACL_vector acl;
  struct Stat stat;

  rc = zoo_get_children(zh, argv[optind], 0, &strings);
  zk_check_rc(rc, argv[optind]);

  if (strings.count > 1)
    qsort(strings.data, strings.count, sizeof(char *), strings_cmp);

  for(i = 0; i < strings.count; i++) {
    if (opt_l) {
      char *path = zksh_join_path(argv[optind], strings.data[i]);

      rc = zoo_get_acl(zh, path, &acl, &stat);
      zk_check_rc(rc, path);
      if (rc != ZOK)
        continue;

      time_t c = stat.mtime/1000;
      char t[15];
      strftime(t, 20, "%D %R", localtime(&c));

      printf("%c %d %lX:%lX %d %s %s\n",
          stat.ephemeralOwner ? 'e' : '-',
          stat.numChildren,
          (u_int64_t)stat.czxid, (u_int64_t)stat.mzxid,
          stat.dataLength,
          t,
          strings.data[i]);

      deallocate_ACL_vector(&acl);
      deallocate_Stat(&stat);
      free(path);
    } else {
      printf("%s\n", strings.data[i]);
    }
  }
  deallocate_String_vector(&strings);

  exit(zk_return_code);
}
