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
