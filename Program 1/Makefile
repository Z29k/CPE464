# Example makefile for CPE464 program 1
#
#  Remember to add /opt/csw/lib to your path in order to execute your program
#  under Solaris.  Putting something like:
#     [ -e "/opt/csw/lib" ] && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/csw/lib
#  in your ~/.mybashrc file should do the trick

CC = gcc
CFLAGS = -g -Wall -Werror
OS = $(shell uname -s)
PROC = $(shell uname -p)
EXEC_SUFFIX=$(OS)-$(PROC)
CFILES = trace.c checksum.c ethernet.c ip.c arp.c icmp.c udp.c tcp.c

ifeq ("$(OS)", "SunOS")
	OSLIB=-L/opt/csw/lib -lsocket -lnsl
	OSINC=-I/opt/csw/include
	OSDEF=-DSOLARIS
else
ifeq ("$(OS)", "Darwin")
	OSLIB=
	OSINC=
	OSDEF=-DDARWIN
else
	OSLIB=
	OSINC=
	OSDEF=-DLINUX
endif
endif

all:  trace-$(EXEC_SUFFIX)

trace-$(EXEC_SUFFIX): $(CFILES)
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) -lpcap -o $@ $(CFILES)

clean:
	rm -f trace-*
