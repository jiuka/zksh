/* 
 *  zksh - ZooKeeper Shell utils.
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

#include "zksh.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

char *usage_description = \
"  Print NODEs ACL to standard output.\n"
"\n"
"  Output format is as follows:\n"
"      1: NODE\n"
"      2:   rwcda ID SCHEME\n"
"      3:   r---- ID SCHEME\n"
"\n"
"  ZooKeeper Permissions:\n"
"    r: read\n"
"    w: write\n"
"    c: create\n"
"    d: delete\n"
"    a: admin\n";

int main(int argc, char **argv) {
  int rc, i;
  struct ACL_vector acl;
  struct Stat stat;

  zksh_init(&argc, argv);

  // Validate user input
  if (argc <= optind) {
    fprintf(stderr, "%s: At least one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }

  zksh_connect();

  for(; optind<argc; optind++) {
    // Validate node path
    if (zksh_check_nodepath(argv[optind]) != 0) {
      zk_return_code = EXIT_FAILURE;
      continue;
    }

    // Get node acl
    rc = zoo_get_acl(zh, argv[optind], &acl, &stat);
    zk_check_rc(rc, argv[optind]);
    if (rc != ZOK)
      continue;

    // Output
    printf("%s\n", argv[optind]);
    for(i=0; i<acl.count; i++) {
      printf("  %c%c%c%c%c %s %s\n",
          acl.data[i].perms & ZOO_PERM_READ ? 'r' : '-',
          acl.data[i].perms & ZOO_PERM_WRITE ? 'w' : '-',
          acl.data[i].perms & ZOO_PERM_CREATE ? 'c' : '-',
          acl.data[i].perms & ZOO_PERM_DELETE ? 'd' : '-',
          acl.data[i].perms & ZOO_PERM_ADMIN ? 'a' : '-',
          acl.data[i].id.id, acl.data[i].id.scheme);
    }
  }

  exit(zk_return_code);
}
