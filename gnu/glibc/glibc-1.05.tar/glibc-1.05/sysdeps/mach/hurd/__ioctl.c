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

#include <ansidecl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <hurd.h>

/* Perform the I/O control operation specified by REQUEST on FD.
   The actual type and use of ARG and the return value depend on REQUEST.  */
int
DEFUN(__ioctl, (fd, request, arg),
      int fd AND int request AND PTR arg)
{
  /* Map individual type fields to Mach IPC types.  */
  const static int mach_types[] =
    { MACH_MSG_TYPE_CHAR, MACH_MSG_TYPE_INTEGER_16, MACH_MSG_TYPE_INTEGER_32,
      -1 };

  /* Message buffer.  */
  char msg[sizeof (mach_msg_header_t) +	/* Header.  */
	   sizeof (mach_msg_type_t) + 4	/* Return code.  */
	   3 * (sizeof (mach_msg_type_t) + (4 * 32))]; /* Argument data.  */
  mach_msg_header_t *m = (mach_msg_header_t *) msg;
  mach_msg_type_t *t = (mach_msg_type_t *) &m[1];
  mach_msg_id_t msgid;

  union __ioctl r;
  error_t err;

#define io2mach_type(count, type) \
  ((mach_msg_type_t)
   { mach_types[type], (type << 1) * 8, count, 1, 0, 0 })

  /* Pack an argument into the message buffer.  */
  inline void in (unsigned int count, unsigned int type)
    {
      if (count > 0)
	{
	  void *const p = &t[1];
	  const size_t len = count * (type << 1);
	  *t = io2mach_type (count, type);
	  memcpy (p, arg, len);
	  arg += len;
	  t = p + ((len + sizeof (*t) - 1) / sizeof (*t) * sizeof (*t));
	}
    }

  /* Unpack the message buffer into the argument location.  */
  inline int out (unsigned int count, unsigned int type,
		  void *store, void **update)
    {
      if (count > 0)
	{
	  const size_t len = count * (type << 1);
	  union { mach_msg_type_t t; int i; } ipctype;
	  ipctype.t = io2mach_type (count, type);
	  if (*(int *) t != ipctype.i)
	    return 1;
	  ++t;
	  memcpy (t, store, len);
	  if (update != NULL)
	    *update += len;
	  t = (void *) t + ((len + sizeof (*t) - 1) / sizeof (*t) * sizeof *t);
	}
      return 0;
    }

  /* Pack the argument data.  */
  r.i = request;

  msgid = 100000 + ((r.s.group << 2) * 1000) + r.s.command;

  in (r.s.count0, r.s.type0);
  in (r.s.count1, r.s.type1);
  in (r.s.count2, r.s.type2);

  err = _HURD_DPORT_USE
    (fd,
     ({
       m->msgh_size = (char *) t - msg;
       m->msgh_request_port = port;
       m->msgh_reply_port = __mig_reply_port ();
       m->msgh_seqno = 0;
       m->msgh_id = msgid;
       m->msgh_bits = ?;	/* XXX */
       _HURD_EINTR_RPC (port, __mach_msg (m, MACH_SEND_MSG|MACH_RCV_MSG,
					  m->msgh_size, sizeof (msg),
					  m->msgh_reply_port,
					  MACH_MSG_TIMEOUT_NONE,
					  MACH_PORT_NULL));
     }));

  switch (err)
    {
    case MACH_MSG_SUCCESS:
      break;
    case MACH_SEND_INVALID_REPLY:
    case MACH_RCV_INVALID_NAME:
      __mig_dealloc_reply-port ();
    default:
      return __hurd_fail (err);
    }

  if (m->msgh_id != msgid + 100)
    return __hurd_fail (m->msgh_id == MACH_NOTIFY_SEND_ONCE ?
			MIG_SERVER_DIED : MIG_REPLY_MISMATCH);

  if ((m->msgh_bits  & MACH_MSGH_BITS_COMPLEX) ||
      m->msgh_size != (char *) t - msg)
    return __hurd_fail (MIG_TYPE_ERROR);

  t = (mach_msg_type_t) &m[1];
  if (out (1, _IOTS (sizeof (error_t)), &err, NULL) ||
      out (r.s.count0, r.s.type0, arg, &arg) ||
      out (r.s.count1, r.s.type2, arg, &arg) ||
      out (r.s.count2, r.s.type2, arg, &arg))
    return __hurd_fail (MIG_TYPE_ERROR);

  if (err)
    return __hurd_fail (err);
  return 0;
}
