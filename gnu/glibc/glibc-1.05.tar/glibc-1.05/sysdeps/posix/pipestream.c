/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define	SH_PATH	"/bin/sh"	/* Shell to run.  */
#define	SH_NAME	"sh"		/* Name to give it.  */

/* Structure describing a popen child.  */
struct child
  {
    /* It is important that the first member of this structure be an `int' that
       is the file descriptor.  This is because the `fileno' function assumes
       that __cookie(STREAM) points to the file descriptor.  */
    int fd;
    pid_t pid;
  };

#ifndef	FORK
#define	FORK	fork
#endif

/* Open a new stream that is a one-way pipe to a
   child process running the given shell command.  */
FILE *
DEFUN(popen, (command, mode), CONST char *command AND CONST char *mode)
{
  pid_t pid;
  int pipedes[2];
  FILE *stream;
  struct child *child;

  if (command == NULL || mode == NULL || (*mode != 'r' && *mode != 'w'))
    {
      errno = EINVAL;
      return NULL;
    }

  /* Create the pipe.  */
  if (pipe(pipedes) < 0)
    return NULL;

  /* Fork off the child.  */
  pid = FORK();
  if (pid == (pid_t) -1)
    {
      /* The fork failed.  */
      (void) close(pipedes[0]);
      (void) close(pipedes[1]);
      return NULL;
    }
  else if (pid == (pid_t) 0)
    {
      /* We are the child side.  Make the write side of
	 the pipe be stdin or the read side be stdout.  */

      CONST char *new_argv[4];

      if ((*mode == 'w' ? dup2(pipedes[STDIN_FILENO], STDIN_FILENO) :
	  dup2(pipedes[STDOUT_FILENO], STDOUT_FILENO)) < 0)
	_exit(127);

      /* Close the pipe descriptors.  */
      (void) close(pipedes[STDIN_FILENO]);
      (void) close(pipedes[STDOUT_FILENO]);

      /* Exec the shell.  */
      new_argv[0] = SH_NAME;
      new_argv[1] = "-c";
      new_argv[2] = command;
      new_argv[3] = NULL;
      (void) execve(SH_PATH, (char *CONST *) new_argv, environ);
      /* Die if it failed.  */
      _exit(127);
    }

  /* We are the parent side.  */

  /* Close the irrelevant side of the pipe and open the relevant side as a
     new stream.  Mark our side of the pipe to close on exec, so new children
     won't see it.  */
  if (*mode == 'r')
    {
      (void) close(pipedes[STDOUT_FILENO]);
      (void) fcntl (pipedes[STDIN_FILENO], F_SETFD, FD_CLOEXEC);
      stream = fdopen(pipedes[STDIN_FILENO], mode);
    }
  else
    {
      (void) close(pipedes[STDIN_FILENO]);
      (void) fcntl (pipedes[STDOUT_FILENO], F_SETFD, FD_CLOEXEC);
      stream = fdopen(pipedes[STDOUT_FILENO], mode);
    }

  if (stream == NULL)
    goto error;

  child = (struct child *) malloc(sizeof(struct child));
  if (child == NULL)
    goto error;
  child->fd = fileno(stream);
  child->pid = pid;
  stream->__cookie = (PTR) child;
  stream->__ispipe = 1;
  return stream;

 error:;
  {
    /* The stream couldn't be opened or the child structure couldn't be
       allocated.  Kill the child and close the other side of the pipe.  */
    int save = errno;
    (void) kill(pid, SIGKILL);
    if (stream == NULL)
      (void) close(pipedes[*mode == 'r' ? STDOUT_FILENO : STDIN_FILENO]);
    else
      (void) fclose(stream);
#ifndef	NO_WAITPID
    (void) waitpid(pid, (int *) NULL, 0);
#else
    {
      pid_t dead;
      do
	dead = wait ((int *) NULL);
      while (dead > 0 && dead != pid);
    }
#endif
    errno = save;
    return NULL;
  }
}

/* Close a stream opened by popen and return its status.
   Returns -1 if the stream was not opened by popen.  */
int
DEFUN(pclose, (stream), register FILE *stream)
{
  pid_t pid, dead;
  int status;

  if (!__validfp(stream) || !stream->__ispipe)
    {
      errno = EINVAL;
      return -1;
    }

  pid = ((struct child *) stream->__cookie)->pid;
  free(stream->__cookie);
  stream->__cookie = (PTR) &stream->__fileno;
  stream->__ispipe = 0;
  if (fclose(stream))
    return -1;

#ifndef	NO_WAITPID
  dead = waitpid (pid, &status, 0);
#else
  do
    dead = wait (&status);
  while (dead > 0 && dead != pid);
#endif
  if (dead != pid)
    status = -1;

  return status;
}
