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
