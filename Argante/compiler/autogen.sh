#!/bin/sh

#
# Argante Virtual OS
# ------------------
#
# Auto-generate language description .h files for agtc compiler
#
# Status: done
#
# Author:     Michal Zalewski <lcamtuf@ids.pl>
# Maintainer: Michal Zalewski <lcamtuf@ids.pl>
#

PATH=/usr/xpg4/bin:$PATH

INFILE=modules/syscall.h
INFILE2=include/exception.h

OFILE=compiler/autogen.h

if [ ! -f $INFILE ]; then
  echo "Sorry, no $INFILE..."
  exit 1
fi

if [ ! -f $INFILE2 ]; then
  echo "Sorry, no $INFILE2..."
  exit 1
fi

echo "Generating $OFILE from $INFILE..."

grep '^#define SYSCALL_' $INFILE | awk -F'efine SYSCALL_' '{print $2}' | \
  grep -v 'ENDLIST' | awk '{print "{ \"" tolower($1) "\",  \t\t" tolower($2) "}," }' >$OFILE

echo "Appending data from $INFILE2 to $INFILE..."

grep '^#define ERROR_' $INFILE2 | awk -F'efine ERROR_' '{print $2}' | \
  awk '{print "{ \"" tolower($1) "\",  \t\t" tolower($2) "}," }' >>$OFILE
