#!/usr/bin/perl -w
#
# A2 Virtual Machine - exception-id header generator
# Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
# 
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License, with the
# added restriction that it may only be converted to the version 2 of the
# GNU General Public License.
# 
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
#

# Converts exception.h into a plain list.
#

my ($in);

# FIRST READ IN THE CODE...

#open F, "<exception.h";

while($in=<STDIN>)
{
	if ($in =~ m/^#define ERR_/) {
		my (@l, $i);
		$in=substr $in, length("#define ERR_");
		@l=split(' ', $in, 2);
		chomp($l[1]);
		printf "{ \"%s\", %s, 2 },\n", lc($l[0]), lc($l[1]);
	}
}
#close F;
