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
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char *usage_description = \
"  Wait for NODE to vanish from ZooKeeper.\n";

pthread_mutex_t wait_for_changemutex;
pthread_cond_t wait_for_changethreshold_cv;

static void cb_signal_cv(zhandle_t* zzh, int type, int state, const char* path, void *watcherCtx) {
  pthread_cond_t *m = (pthread_cond_t *)watcherCtx;
  pthread_cond_signal(m);
}

int main(int argc, char **argv) {
  int rc;
  int i;

  zksh_init(&argc, argv);

  // Validate user input
  if (argc-optind != 1) {
    fprintf(stderr, "%s: Exactly one node needs to be specified.\n", zksh_program);
    exit(EXIT_FAILURE);
  }
  if (zksh_check_nodepath(argv[optind]) != 0)
    exit(EXIT_FAILURE);

  zksh_connect();

  // Initialize mutex and cv to wait logic
  pthread_mutex_init(&wait_for_changemutex, NULL);
  pthread_cond_init(&wait_for_changethreshold_cv, NULL);

  do {
    // Check if node exists
    rc = zoo_wexists(zh, argv[optind], cb_signal_cv, &wait_for_changethreshold_cv, NULL);
    if (rc == ZNONODE)
      break;
    zk_check_rc(rc, argv[optind]);
    if (rc != ZOK)
      exit(EXIT_FAILURE);

    // Wait till node changes
    pthread_mutex_lock(&wait_for_changemutex);
    pthread_cond_wait(&wait_for_changethreshold_cv, &wait_for_changemutex);
    pthread_mutex_unlock(&wait_for_changemutex);

  } while (1);

  exit(zk_return_code);
}
