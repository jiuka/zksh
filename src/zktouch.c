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
