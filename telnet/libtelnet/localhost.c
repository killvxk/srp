/* A slightly more convenient wrapper for gethostname

   Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#if defined(STDC_HEADERS) || defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#include <string.h>
#include <errno.h>

#if	!defined(__P)
#ifdef	__STDC__
#define __P(x)	x
#else
#define __P(x)	()
#endif
#endif

extern char *xmalloc __P ((size_t));
/* Return the name of the localhost.  This is just a wrapper for gethostname,
   which takes care of allocating a big enough buffer, and caches the result
   after the first call (so the result should be copied before modification).
   If something goes wrong, 0 is returned, and errno set.  */
/* We no longer use static buffers, is to dangerous and
   cause subtile bugs.  */
char *
localhost (void)
{
  char *buf = 0;
  size_t buf_len = 0;

  do
    {
      errno = 0;

      buf_len = 128;	/* Initial guess */
      buf = xmalloc (buf_len);

      if (! buf)
	{
	  errno = ENOMEM;
	  return 0;
	}
    } while ((gethostname(buf, buf_len) == 0 && !memchr (buf, '\0', buf_len))
#ifdef ENAMETOOLONG
	     || errno == ENAMETOOLONG
#endif
	     );

  if (errno)
    /* gethostname failed, abort.  */
    {
      free (buf);
      buf = 0;
    }

  return buf;
}
