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
#include <math.h>

/* Return X to the Y power.  */
double
DEFUN(pow, (x, y), double x AND double y)
{
  if (y == 0.0 || x == 1.0)
    return 1.0;
  else if (y == 1.0)
    return x;
  else if (y == 2.0)
    return x * x;
  else if (y == -1.0)
    return 1.0 / x;

  return exp(y * log(x));
}
