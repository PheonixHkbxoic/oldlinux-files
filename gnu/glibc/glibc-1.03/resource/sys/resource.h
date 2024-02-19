/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#ifndef	_SYS_RESOURCE_H

#define	_SYS_RESOURCE_H	1
#include <features.h>


#include <gnu/time.h>

/* Kinds of resource limit.  */
enum __rlimit_resource
  {
    /* Per-process CPU limit, in seconds.  */
    RLIMIT_CPU,
    /* Largest file that can be created, in bytes.  */
    RLIMIT_FSIZE,
    /* Maximum size of data segment, in bytes.  */
    RLIMIT_DATA,
    /* Maximum size of stack segment, in bytes.  */
    RLIMIT_STACK,
    /* Largest core file that can be created, in bytes.  */
    RLIMIT_CORE,
    /* Largest resident set size, in bytes.
       This affects swapping; processes that are exceeding their
       resident set size will be more likely to have physical memory
       taken from them.  */
    RLIMIT_RSS,

    RLIM_NLIMITS
  };

struct rlimit
  {
    /* The current (soft) limit.  */
    int rlim_cur;
    /* The hard limit.  */
    int rlim_max;
  };

/* Value used to indicate that there is no limit.  */
#define RLIM_INFINITY 0x7fffffff

/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
int EXFUN(getrlimit, (enum __rlimit_resource __resource,
		      struct rlimit *__rlimits));

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
int EXFUN(setrlimit, (enum __rlimit_resource __resource,
		      struct rlimit *__rlimits));


/* Whose usage statistics do you want?  */
enum __rusage_who
  {
    /* The calling process.  */
    RUSAGE_SELF = 0,
    /* All of its terminated child processes.  */
    RUSAGE_CHILDREN = -1,
  };

/* Structure which says how much of each resource has been used.  */
struct rusage
  {
    /* Total amount of user time used.  */
    struct __timeval ru_utime;
    /* Total amount of system time used.  */
    struct __timeval ru_stime;
    /* Maximum resident set size (in kilobytes).  */
    int ru_maxrss;
    /* Amount of sharing of text segment memory
       with other processes (kilobyte-seconds).  */
    int ru_ixrss;
    /* Amount of data segment memory used (kilobyte-seconds).  */
    int ru_idrss;
    /* Amount of stack memory used (kilobyte-seconds).  */
    int ru_isrss;
    /* Number of soft page faults (i.e. those serviced by reclaiming
       a page from the list of pages awaiting reallocation.  */
    int ru_minflt;
    /* Number of hard page faults (i.e. those that required I/O).  */
    int ru_majflt;
    /* Number of times a process was swapped out of physical memory.  */
    int ru_nswap;
    /* Number of input operations via the file system.  Note: This
       and `ru_oublock' do not include operations with the cache.  */
    int ru_inblock;
    /* Number of output operations via the file system.  */
    int ru_oublock;
    /* Number of IPC messages sent.  */
    int ru_msgsnd;
    /* Number of IPC messages received.  */
    int ru_msgrcv;
    /* Number of signals delivered.  */
    int ru_nsignals;
    /* Number of voluntary context switches, i.e. because the process
       gave up the process before it had to (usually to wait for some
       resource to be available).  */
    int ru_nvcsw;
    /* Number of involuntary context switches, i.e. a higher priority process
       became runnable or the current process used up its time slice.  */
    int ru_nivcsw;
  };

/* Return resource usage information on process indicated by WHO
   and put it in *USAGE.  Returns 0 for success, -1 for failure.  */
int EXFUN(__getrusage, (enum __rusage_who __who, struct rusage *__usage));
int EXFUN(getrusage, (enum __rusage_who __who, struct rusage *__usage));

#ifdef	__OPTIMIZE__
#define	getrusage(who, usage)	__getrusage((who), (usage))
#endif	/* Optimizing.  */

/* Function depends on CMD:
   1 = Return the limit on the size of a file, in units of 512 bytes.
   2 = Set the limit on the size of a file to NEWLIMIT.  Only the
       super-user can increase the limit.
   3 = Return the maximum possible address of the data segment.
   4 = Return the maximum number of files that the calling process can open.
   Returns -1 on errors.  */
long int EXFUN(__ulimit, (int __cmd, long int __newlimit));
long int EXFUN(ulimit, (int __cmd, long int __newlimit));

#ifdef	__OPTIMIZE__
#define	ulimit(cmd, newlimit)	__ulimit((cmd), (newlimit))
#endif	/* Optimizing.  */


/* Priority limits.  */
#define	PRIO_MIN	-20	/* Minimum priority a process can have.  */
#define	PRIO_MAX	20	/* Maximum priority a process can have.  */

/* The type of the WHICH argument to `getpriority' and `setpriority',
   indicating what flavor of entity the WHO argument specifies.  */
enum __priority_which
  {
    PRIO_PROCESS = 0,	/* WHO is a process ID.  */
    PRIO_PGRP = 1,	/* WHO is a process group ID.  */
    PRIO_USER = 2,	/* WHO is a user ID.  */
  };

/* Return the highest priority of any process specified by WHICH and WHO
   (see above); if WHO is zero, the current process, process group, or user
   (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX (above).  */
extern int EXFUN(getpriority, (enum __priority_which __which, int __who));

/* Set the priority of all processes specified by WHICH and WHO (see above)
   to PRIO.  Returns 0 on success, -1 on errors.  */
extern int EXFUN(setpriority, (enum __priority_which __which, int __who,
			       int __prio));


#endif	/* resource.h  */
