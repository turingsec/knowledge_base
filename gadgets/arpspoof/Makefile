CC				= gcc
CXX				= g++
RM				= rm -f

CPPFLAGS		= $(shell root-config --cflags)
CPPFLAGS 		+= -Wall -W -g -O2 -I. -Iinclude/
CPPFLAGS		+= -Wno-sign-compare -Wno-unused-parameter -Wno-write-strings

LDFLAGS			= $(shell root-config --ldflags)
LDFLAGS			+= -rdynamic

LDLIBS			= -L/usr/lib/x86_64-linux-gnu -rdynamic

LNETLIB			= -lnet

SRCS			= $(wildcard src/*.c) src/rawsocket/route.c

HEADERS			= $(patsubst %.h,%.h,$(wildcard resource/include/*.h)) $(patsubst %.h,%.h,$(wildcard syscalls/include/*.h)) $(patsubst %.h, %.h, $(wildcard include/*.h))
OBJS			= $(patsubst .cpp, .o, $(SRCS))

all: arpspoof

arpspoof: $(OBJS) $(HEADERS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o arpspoof $(OBJS) $(LDLIBS) $(LNETLIB)

%.o : %.c
	$(CC) $(CPPFLAGS) -o $@ -c $<

clean:
	$(RM) *.o arpspoof