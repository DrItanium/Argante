#!/bin/bash

# My which insists on printing an error message.
# This shuts it up.

WHICH='type -path'

PIDOF="`$WHICH pidof 2>/dev/null`"

if [ -z "$PIDOF" ];
then {
	echo "You don't have pidof!"
	echo "It is generally a killall or killall5 under"
	echo "a different name. Create a link and try again."
	exit 1;
};
fi

AGTPID="`$PIDOF argante`"

if [ -z "$AGTPID" ];
then {
	echo "I can't find an argante process."
	echo "I'm going to try and start one."
	echo .
	
	AGTDIR="`$WHICH argante 2>/dev/null`"

	echo $AGTDIR
	
	if [ -z "$AGTDIR" ] || [ ! -x $AGTDIR/argante ];
	then AGTDIR=".."; fi
	
	if [ ! -x $AGTDIR/argante ];
	then AGTDIR="."; fi
	
	if [ ! -x $AGTDIR/argante ];
	then {
		echo "Sorry, can't find your argante dir."
		echo "Try changing to it."
		exit 1;
	};
	fi

	AGTBACK=`$WHICH agtback 2>/dev/null`
	echo $AGTBACK
	
	test -z "$AGTBACK" && AGTBACK=$AGTDIR/tools/agtback 
	
	if [ -z "$AGTBACK" ];
	then {
	  echo 'Cannot find agtback utility.'
	  exit 1;
	};
	fi
	
	$AGTBACK $AGTDIR/argante $AGTDIR/conf/scripts/argboot.scr
	AGTPID="`$PIDOF argante`"
	test -z "$AGTPID" && exit 1;
};
fi

AGTGTK="`$WHICH agtgtk 2>/dev/null`"

if [ ! -x "$AGTGTK" ]; 
then AGTGTK="./agtgtk"; fi
	
if [ ! -x "$AGTGTK" ]; 
then AGTGTK="./gtk/agtgtk"; fi

if [ ! -x "$AGTGTK" ];
then {
	echo "Hmm, can't find agtgtk."
	echo "Try putting it in your PATH."
	exit 1;
};
fi

for I in $AGTPID;
do {
	$AGTGTK $I &
};
done;

