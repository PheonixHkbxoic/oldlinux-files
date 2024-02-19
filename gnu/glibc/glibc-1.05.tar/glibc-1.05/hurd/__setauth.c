/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <gnu-stabs.h>

/* Things in the library which want to be run when the auth port changes.  */
const struct
  {
    size_t n;
    void (*fn[0]) (auth_t new_auth);
  } _hurd_reauth_hook;


static struct mutex reauth_lock;


/* Set the auth port to NEW, and reauthenticate
   everything used by the library.  */
int
__setauth (auth_t new)
{
  error_t err;
  auth_t old;
  int d;
  mach_port_t newport;
  void (**fn) (auth_t);

  /* Give the new send right a user reference.
     This is a good way to check that it is valid.  */
  if (__mach_port_mod_refs (__mach_task_self (), new,
			    MACH_PORT_RIGHT_SEND, 1))
    {
      errno = EINVAL;
      return -1;
    }

  /* We lock against another thread doing setauth.
     Anyone who sets _hurd_auth some other way is asking to lose.  */
  __mutex_lock (&reauth_lock);

  /* Install the new port in the _hurd_auth cell.  */
  __mutex_lock (&_hurd_idlock);
  _hurd_port_set (&_hurd_auth, new);
  _hurd_id_valid = 0;
  __mutex_unlock (&_hurd_idlock);

  if (_hurd_init_dtable != NULL)
    /* We just have the simple table we got at startup.
       Otherwise, a reauth_hook in dtable.c takes care of this.  */
    for (d = 0; d < _hurd_init_dtablesize; ++d)
      if (_hurd_init_dtable[d] != MACH_PORT_NULL)
	{
	  mach_port_t new;
	  if (! __io_reauthenticate (_hurd_init_dtable[d]) &&
	      ! _HURD_PORT_USE (&_hurd_auth,
				__auth_user_authenticate (_hurd_init_dtable[d],
							  &new)))
	    {
	      __mach_port_deallocate (__mach_task_self (),
				      _hurd_init_dtable[d]);
	      _hurd_init_dtable[d] = new;
	    }
	}

  if (_HURD_PORT_USE (&_hurd_crdir,
		      ! __io_reauthenticate (port) &&
		      ! __auth_user_authenticate (new, port, &newport)))
    _hurd_port_set (&_hurd_crdir, newport);

  if (_HURD_PORT_USE (&_hurd_cwdir,
		      ! __io_reauthenticate (port) &&
		      ! __auth_user_authenticate (new, port, &newport)))
    _hurd_port_set (&_hurd_cwdir, newport);

  /* Run things which want to do reauthorization stuff.  */
  for (fn = _hurd_reauth_hook.fn; *fn != NULL; ++fn)
    (**fn) (new);

  __mutex_unlock (&reauth_lock);

  return 0;
}

static void
init_reauth (void)
{
  __mutex_init (&reauth_lock);
}

text_set_element (__libc_subinit, init_reauth);
