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

#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifndef	HAVE_GNU_LD
#define	__tzname	tzname
#define	__daylight	daylight
#define	__timezone	timezone
#endif

/* Return the `struct tm' representation of *TIMER in the local timezone.  */
struct tm *
DEFUN(localtime, (timer), CONST time_t *timer)
{
  extern int EXFUN(__tz_compute, (time_t, struct tm));
  register struct tm *tp;

  if (timer == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  tp = gmtime(timer);
  if (tp == NULL)
    return NULL;

  if (!__tz_compute (*timer, *tp))
    return NULL;

  tp = __offtime(timer, __timezone);
  tp->tm_isdst = __daylight;
  tp->tm_gmtoff = __timezone;
  tp->tm_zone = __tzname[__daylight];
  return tp;
}
