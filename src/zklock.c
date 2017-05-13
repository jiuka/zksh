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
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char *usage_description = \
"  Print NODEs content to standard output.\n";

pthread_mutex_t wait_for_answer_mutex;
pthread_cond_t wait_for_answer_threshold_cv;

static void watcher_exist(zhandle_t* zzh, int type, int state,
    const char* path, void *watcherCtx) {
pthread_mutex_lock(&wait_for_answer_mutex);
pthread_cond_signal(&wait_for_answer_threshold_cv);
pthread_mutex_unlock(&wait_for_answer_mutex);
}

static void cb_signal_cv(zhandle_t* zzh, int type, int state, const char* path, void *watcherCtx) {
  pthread_cond_t *m = (pthread_cond_t *)watcherCtx;
  pthread_cond_signal(m);
  printf("[%d] changed %s\n", getpid(), path);
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
  struct Stat stat;
  struct String_vector strings;

  zksh_init(&argc, argv);

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

    pthread_mutex_init(&wait_for_answer_mutex, NULL);
    pthread_cond_init(&wait_for_answer_threshold_cv, NULL);


  // Wait for Lock
  int qpos;
  do {
    // Reset status
    locks = 0;
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

    printf("[%d] QPOS: %d\n", getpid(), qpos);

    if (qpos == locksize) {
      for (i=0; i < locksize; i++) {
        previous = zksh_join_path(argv[optind], strings.data[i]);
        rc = zoo_wexists(zh, previous, cb_signal_cv, &wait_for_answer_threshold_cv, NULL);
        
        printf("[%d] Wait for %s\n", getpid(), previous);
      }
      pthread_mutex_lock(&wait_for_answer_mutex);
      pthread_cond_wait(&wait_for_answer_threshold_cv, &wait_for_answer_mutex);
      pthread_mutex_unlock(&wait_for_answer_mutex);
    } else if(qpos > locksize) {
      previous = zksh_join_path(argv[optind], strings.data[qpos-1]);
      rc = zoo_wexists(zh, previous, cb_signal_cv, &wait_for_answer_threshold_cv, NULL);
      
      printf("[%d] Wait for %s\n", getpid(), previous);
      
      pthread_mutex_lock(&wait_for_answer_mutex);
      pthread_cond_wait(&wait_for_answer_threshold_cv, &wait_for_answer_mutex);
      pthread_mutex_unlock(&wait_for_answer_mutex);
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

optind++;

  printf("[%d] GOT LOCK %s\n", getpid(), mylock_short);

 if (argc > optind) {

    pid_t w, f;

    /* Clear any inherited settings */
    signal(SIGCHLD, SIG_DFL);
    f = fork();

    int err;

    if ( f < 0 ) {
      err = errno;
      fprintf(stderr, "%s: fork failed: %s\n", zksh_program, strerror(err));
      exit(1);
    } else if ( f == 0 ) {
      char *cmd_argv[argc-optind+1];
      for (i=0; i < (argc-optind); i++) {
        cmd_argv[i] = argv[optind+i];
      }
      cmd_argv[argc-optind] = NULL;
      execvp(cmd_argv[0], cmd_argv);
      err = errno;
      /* execvp() failed */
      fprintf(stderr, "%s: %s: %s\n", zksh_program, argv[optind], strerror(err));
      exit(1);
    } else {
      do {
	w = waitpid(f, &rc, 0);
	if (w == -1 && errno != EINTR)
	  break;
      } while ( w != f );

      if (w == -1) {
	err = errno;
	rc = EXIT_FAILURE;
	fprintf(stderr, "%s: waitpid failed: %s\n", zksh_program, strerror(err));
      } else if ( WIFEXITED(rc) )
	rc = WEXITSTATUS(rc);
      else if ( WIFSIGNALED(rc) )
	rc = WTERMSIG(rc) + 128;
      else
	rc = -123;	/* WTF? */
    }
  }


  printf("[%d] Die %s\n", getpid(), mylock_short);

      // Test for file precense
      rc = zoo_exists(zh, mylock, 0, &stat);
      zk_check_rc(rc, mylock);
      if (rc != ZOK)
        exit(1);
     
      // Delete Node
      rc = zoo_delete(zh, mylock, stat.version);
      zk_check_rc(rc, mylock);
     
  exit(zk_return_code);
}
