#!/usr/bin/env python

#
#  Argante virtual OS
#  (C) 2000 Michal Zalewski <lcamtuf@ids.pl>
#
#  Modify .bin headers
#
#  Status: unfinished, for your amusement
#  Maintainer: Artur Skura <arturs@people.pl>
#


from sys import *
from string import *
from getopt import *


def usage():
    print """
Usage:
fiddler [-d domains -f flags -p priority -r ipc register -i initial ip
-c current domain -u domain uid -b bytesize -m memory flags -d datasize
-s signature -o output file -v ] filename

Example:
fiddler -s 'second hello version'  -o hello2.img hello.img
fiddler -v hello.img
    
	    """

def byte2dec(string):
    dec=0
    for i in range(len(string)):
	dec=dec+ord(string[i])
    return dec
    


    
try:
    options,args= getopt(argv[1:],"hvf:p:r:i:c:u:b:m:d:s:o:")
except error,msg:
    stdout=stderr
    print msg
    usage()
    exit(2)
    
if len(argv)<2:
    usage()
    exit(2)

if args[0]:
    try:
        f=open(args[0],"r")
    except IOError:
        "File open error"
    else:
        buffer=f.read(1000)
	
        agt_magic1=buffer[0:4]
        agt_domains=buffer[4:20]
        agt_flags=buffer[20:24]
        agt_priority=buffer[24:28]
        agt_ipc_reg=buffer[28:32]
        agt_init_ip=buffer[32:36]
        agt_current_domain=buffer[36:40]
        agt_domain_uid=buffer[40:44]
        agt_bytesize=buffer[44:48]
        agt_memflags=buffer[48:52]
        agt_datasize=buffer[52:56]
        agt_signature=buffer[56:120]
        agt_magic2=buffer[120:124]
    
        agt_bytesize_d=byte2dec(agt_bytesize)
        offset_b=(124+12*agt_bytesize_d)
        agt_bytes=buffer[124:offset_b]

        agt_datasize_d=byte2dec(agt_datasize)
        offset_d=(offset_b+12*agt_bytesize_d)
        agt_data=buffer[offset_b:offset_d]

    
for o,a in options:
    if o=='-h':
	usage()
	    
    if o=='-s':
	if len(a)>64:
	    print "signature length cannot exceed 64"
	agt_signature=a
        for i in range(64-len(a)):
	    agt_signature=agt_signature+' '
	

    if o=='-v':
	print "Program: ",args[0] 
	print "Code segment size: ", agt_bytesize_d
	print "Data segment size: ", agt_datasize_d    
	print "Signature: ", agt_signature

    if o=='-o':
        try:
    	    g=open(a,"w")
	except IOError:
    	    "File open error"
	else:
	    output=agt_magic1 + agt_domains + agt_flags + agt_priority \
	    + agt_ipc_reg + agt_init_ip + agt_current_domain + agt_domain_uid \
	    + agt_bytesize + agt_memflags + agt_datasize + agt_signature \
	    + agt_magic2 + agt_bytes + agt_data

	    g.write(output)
	    g.close()
