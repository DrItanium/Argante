#!/bin/sh

#
# Argante Virtual OS
# ------------------
#
# Auto-generate debugging stuff
#
# Status: done
#
# Author:     Maurycy Prodeus <z33d@eth-security.net>
# Maintainer: Maurycy Prodeus <z33d@eth-security.net>
#


PATH=/usr/xpg4/bin:$PATH

INFILE=modules/syscall.h

OFILE=include/autogen-debug.h

if [ ! -f $INFILE ]; then
  echo "Sorry, no $INFILE..."
  exit 1
fi
echo "Generating $OFILE from $INFILE..."

grep '^#define SYSCALL_' $INFILE | awk -F'efine SYSCALL_' '{print $2}' | \
  grep -v 'ENDLIST' | awk '{print "{ \"" toupper($1) "\",  \t\t" tolower($2) "}," }' >$OFILE
