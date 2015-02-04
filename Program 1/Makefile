# Example makefile for CPE464 program 1
#
#  Remember to add /opt/csw/lib to your path in order to execute your program
#  under Solaris.  Putting something like:
#     [ -e "/opt/csw/lib" ] && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/csw/lib
#  in your ~/.mybashrc file should do the trick

CC = gcc
CFLAGS = -Wall -Werror
CFILES = trace.c checksum.c ethernet.c ip.c arp.c icmp.c udp.c tcp.c

all:  trace

trace: $(CFILES)
	$(CC) $(CFLAGS) -lpcap -o trace $(CFILES)

clean:
	rm trace
