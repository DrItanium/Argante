/*
 * A2 Virtual Machine - message reporting functions
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; version 2 of the License, with the
 * added restriction that it may only be converted to the version 2 of the
 * GNU General Public License.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "autocfg.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "printk.h"

static const char lev_chars[]="..--++!!";

/* Actual Work Done Here! */
static int printk2v(int level, const char *format, va_list ap) {
	int bytes;
	bytes=fprintf(stderr, "<%c> ", lev_chars[level]);
	bytes+=vfprintf(stderr, format, ap);
	return bytes;
}

int printk2(int level, const char *format, ...)
{
	va_list ap;
	int bytes;

	va_start(ap, format);
	bytes=printk2v(level, format, ap);
	va_end(ap);
	return bytes;
}

/* The obsolete version. It's pretty useless ATM, but
	when agents arrive (cmon, Michal! :) it will be. */
int printk(const char *format, ...)
{
	va_list ap;
	int bytes;
	int level=PRINTK_ERR;

	if (format[0] == '<' && format[1] != 0 && format[2] == '>') {
		const char *x;
		x=strchr(lev_chars, format[1]);
		if (x) {
			level=x-lev_chars;
			format+=3;
			if (*format == ' ') format++;
		}
	}

	va_start(ap, format);
	bytes=printk2v(level, format, ap);
	va_end(ap);
	return bytes;
}


