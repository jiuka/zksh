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
"  Print NODEs content to standard output.\n";

pthread_mutex_t wait_for_changemutex;
pthread_cond_t wait_for_changethreshold_cv;

static void cb_signal_cv(zhandle_t* zzh, int type, int state, const char* path, void *watcherCtx) {
  pthread_cond_t *m = (pthread_cond_t *)watcherCtx;
  pthread_cond_signal(m);
}

int lock_cmp(const void *a, const void *b) {
  const char **ia = (const char **) a;
  const char **ib = (const char **) b;

  if (strncmp(*ia, "lock-", 5) != 0)
    return -1;
  if (strncmp(*ib, "lock-", 5) != 0)
    return 1;

  return strcmp(*ia, *ib);
}

int main(int argc, char **argv) {
  char *mylock;
  char *mylock_short;
  char *buffer;
  char *previous;
  int rc;
  int locksize = 5;
  int locks;
  int i;
  int c;
  struct Stat stat;
  struct String_vector strings;

  zksh_init(&argc, argv);

  if (strcmp(argv[optind], "--") == 0)
    optind++;

  if (argc == optind+1)
    zksh_eproxy_init();

  zksh_connect();

  buffer = malloc(1024);

  // Get Lock node
  rc = zoo_get(zh, argv[optind], 0, buffer, &locksize, &stat);
  zk_check_rc(rc, argv[optind]);
  if (rc != ZOK)
    exit(EXIT_FAILURE);

  // Read lock capacity
  locksize = (int)strtol(buffer, NULL, 10);
  if (locksize < 1) {
    fprintf(stderr, "%s: Invalid lock size: %d\n", zksh_program, locksize);
    exit(EXIT_FAILURE);
  }
  if (zksh_loglevel >= ZOO_LOG_LEVEL_INFO) {
    printf("Locksize of %s is: %d\n", argv[optind], locksize);
  }

  // Create lock node
  mylock = malloc(1024);
  snprintf(buffer, 1024, "%s/lock-", argv[optind]);

  rc = zoo_create(zh, buffer, NULL, -1, zksh_get_acl(), ZOO_EPHEMERAL | ZOO_SEQUENCE, mylock, 1024);
  zk_check_rc(rc, argv[optind]);
  if (rc != ZOK)
    exit(EXIT_FAILURE);

  mylock_short = mylock + strlen(argv[optind]) + 1;

  if (zksh_loglevel >= ZOO_LOG_LEVEL_INFO) {
    printf("Created lock node %s\n", mylock_short);
  }

  // Initialize mutex and cv to wait logic
  pthread_mutex_init(&wait_for_changemutex, NULL);
  pthread_cond_init(&wait_for_changethreshold_cv, NULL);

  // Wait for Lock
  int qpos;
  do {
    // Reset status
    previous = NULL;

    // List all locks
    rc = zoo_get_children(zh, argv[optind], 0, &strings);
    zk_check_rc(rc, argv[optind]);
    if (rc != ZOK)
      exit(EXIT_FAILURE);

    qsort(strings.data, strings.count, sizeof(char *), lock_cmp);

    char *m;
    m = bsearch(&mylock_short, strings.data, strings.count, sizeof(char *), lock_cmp);
    qpos = ((intptr_t)m - (intptr_t)strings.data) / sizeof(char *);

    if (qpos == locksize) {
      // Next in line, watch all current
      for (i=0; i < locksize; i++) {
        previous = zksh_join_path(argv[optind], strings.data[i]);
        rc = zoo_wexists(zh, previous, cb_signal_cv, &wait_for_changethreshold_cv, NULL);
      }
      pthread_mutex_lock(&wait_for_changemutex);
      pthread_cond_wait(&wait_for_changethreshold_cv, &wait_for_changemutex);
      pthread_mutex_unlock(&wait_for_changemutex);
    } else if(qpos > locksize) {
      // Watch previous queue entry
      previous = zksh_join_path(argv[optind], strings.data[qpos-1]);
      rc = zoo_wexists(zh, previous, cb_signal_cv, &wait_for_changethreshold_cv, NULL);
      
      pthread_mutex_lock(&wait_for_changemutex);
      pthread_cond_wait(&wait_for_changethreshold_cv, &wait_for_changemutex);
      pthread_mutex_unlock(&wait_for_changemutex);
    }

  } while (qpos >= locksize);

  // Test for file precense
  rc = zoo_exists(zh, mylock, 0, &stat);
  zk_check_rc(rc, mylock);
  if (rc != ZOK)
    exit(zk_return_code);

  // Write to node
  rc = zoo_set(zh, mylock, "got lock", 8, stat.version);
  zk_check_rc(rc, mylock);
  if (rc != ZOK)
    exit(zk_return_code);


  if (argc == optind+1) {
    zksh_eproxy_detach();
    zksh_eproxy_wait();
  } else {
    optind++;

    pid_t w, f;
    signal(SIGCHLD, SIG_DFL);
    f = fork();

    if (f < 0) {
      fprintf(stderr, "%s: fork failed: %s\n", zksh_program, strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (f == 0) {
      // Child runs the code
      execvp(argv[optind], argv + optind);
      /* execvp() failed */
      fprintf(stderr, "%s: %s: %s\n", zksh_program, argv[optind], strerror(errno));
      exit(EXIT_FAILURE);
    }

    // Wait for child to exit
    do {
      w = waitpid(f, &rc, 0);
      if (w == -1 && errno != EINTR)
        break;
    } while ( w != f );

    if (w == -1) {
      zk_return_code = EXIT_FAILURE;
      fprintf(stderr, "%s: waitpid failed: %s\n", zksh_program, strerror(errno));
    } else if ( WIFEXITED(rc) ) {
      zk_return_code = WEXITSTATUS(rc);
    } else if ( WIFSIGNALED(rc) ) {
      zk_return_code = WTERMSIG(rc) + 128;
    } else {
      zk_return_code = -123;
    }
  }

  exit(zk_return_code);
}
