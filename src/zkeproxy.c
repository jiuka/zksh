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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>

pid_t zksh_ep_parent;
pid_t zksh_ep_proxy;
int zksh_ep_proxyexit = 0;

void zksh_eproxy_usr1(int signum) {
  if (signum == SIGUSR1)
    zksh_ep_proxyexit = 1;
  if (signum == SIGCHLD)
    zksh_ep_proxyexit = 1;
}

int zksh_eproxy_init() {

  zksh_ep_proxy = getpid();
  zksh_ep_parent = getppid();

  // Prepare the pipes before forking
  int pipe_stdin[2];
  int pipe_stdout[2];
  int pipe_stderr[2];

  if (pipe(pipe_stdin) < 0 || pipe(pipe_stdout) < 0 || pipe(pipe_stderr) < 0) {
    perror("Pipe broke");
    exit(EXIT_FAILURE);
  }

  // Fork away.
  pid_t pid = fork();

  if (pid == -1) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }

  // Handle the child
  if (pid == 0) {
    close(pipe_stdin[1]);
    close(pipe_stdout[0]);
    close(pipe_stderr[0]);

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    dup2(pipe_stdin[0], STDIN_FILENO);
    dup2(pipe_stdout[1], STDOUT_FILENO);
    dup2(pipe_stderr[1], STDERR_FILENO);
    
    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    close(pipe_stderr[1]);
    
    return EXIT_SUCCESS;
  }

  // Do the proxy
  fd_set rfds;
  struct timespec ts = { 5, 0 };
  int rc;
  int stdin_eof = 0;
  char buffer[1024];
  int status = 0;
  ssize_t size_r, size_w;
  sigset_t unblockset;
  sigfillset(&unblockset);

  sigdelset(&unblockset, SIGUSR1);
  sigdelset(&unblockset, SIGCHLD);

  signal(SIGUSR1, zksh_eproxy_usr1);
  signal(SIGCHLD, zksh_eproxy_usr1);

  do {
    //Setup the select
    FD_ZERO (&rfds);
    if (!feof(stdin))
      FD_SET (STDIN_FILENO, &rfds);
    FD_SET (pipe_stdout[0], &rfds);
    FD_SET (pipe_stderr[0], &rfds);

    if (zksh_ep_proxyexit!= 0)
      ts = (struct timespec){ 0, 1 };

    rc = pselect(pipe_stderr[0]+1, &rfds, NULL, NULL, &ts, &unblockset);

    if (rc > 0) {
      if (FD_ISSET(STDIN_FILENO, &rfds)) {
        size_r = read(STDIN_FILENO, &buffer, 1024);
        if (size_r > 1) {
          size_w = write(pipe_stdin[1], buffer, size_r);
          if (size_r != size_w)
            fprintf(stderr, "Piping to stdin failed\n");
        }
      }

      if (FD_ISSET(pipe_stdout[0], &rfds)) {
        size_r = read(pipe_stdout[0], &buffer, 1024);
        if (size_r > 1) {
         size_w = write(STDOUT_FILENO, buffer, size_r);
         if (size_r != size_w)
           fprintf(stderr, "Piping to stdout failed\n");
        }
      }

      if (FD_ISSET(pipe_stderr[0], &rfds)) {
        size_r = read(pipe_stderr[0], &buffer, 1024);
        if (size_r > 0) {
          size_w = write(STDERR_FILENO, buffer, size_r);
          if (size_r != size_w)
            fprintf(stderr, "Piping to stderr failed\n");
        }
      }
    }

    if (waitpid(pid, &status, WNOHANG) > 0)
      zksh_ep_proxyexit = 1;
  } while (zksh_ep_proxyexit == 0 || rc > 0);

  exit(WEXITSTATUS(status));
}

void zksh_eproxy_detach() {
  kill(zksh_ep_proxy, SIGUSR1);
}

void zksh_eproxy_wait() {
  int alive;

  do {
    sleep(3);
  } while (kill(zksh_ep_parent,0) != -1);
}
