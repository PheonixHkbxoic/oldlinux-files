/* _hurd_socket_server - Find the server for a socket domain.

Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <sys/socket.h>
#include <gnu-stabs.h>

static struct mutex lock;
static void
init_sock (void)
{
  __mutex_init (&lock);
}
text_set_element (__libc_subinit, init_sock);

static file_t sockdir = MACH_PORT_NULL;
static file_t *servers;
static int max_domain;

/* Return a port to the socket server for DOMAIN.
   Socket servers translate nodes in the directory _SERVERS_SOCKET
   (canonically /servers/socket).  These naming point nodes are named
   by the simplest decimal representation of the socket domain number,
   for example "/servers/socket/3".

   Socket servers are assumed not to change very often.
   The library keeps all the server socket ports it has ever looked up,
   and does not look them up in /servers/socket more than once.  */

socket_t
_hurd_socket_server (int domain)
{
  error_t err;

  __mutex_lock (&lock);

  if (sockdir == MACH_PORT_NULL)
    {
      sockdir = __path_lookup (_SERVERS_SOCKET, FS_LOOKUP_EXEC, 0);
      if (sockdir == MACH_PORT_NULL)
	{
	  __mutex_unlock (&lock);
	  return MACH_PORT_NULL;
	}
    }

  if (domain > max_domain)
    {
      file_t *new = realloc (servers, (domain + 1) * sizeof (file_t));
      if (new == NULL)
	{
	  __mutex_unlock (&lock);
	  return MACH_PORT_NULL;
	}
      while (max_domain < domain)
	new[max_domain++] = MACH_PORT_NULL;
      servers = new;
    }

  {
    char name[100];
    sprintf (name, "%d", domain);
    if (err = _HURD_PORT_USE (&_hurd_crdir,
			      __hurd_path_lookup (port, sockdir,
						  name, 0, 0,
						  &servers[domain])))
      errno = err;
  }

  __mutex_unlock (&lock);

  return servers[domain];
}
