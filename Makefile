# BW-Admin Makefile
#
# $Id$


#############################################################################
# SETUP
#############################################################################

RM = rm -f
LN = ln -s
CC = gcc

# Compiler Flags

BASE_CFLAGS     = -I./include -I../crypt-string/include -Dstricmp=strcasecmp

SHFLAGS		= -fPIC -shared
LDFLAGS		= -Wl,--retain-symbols-file shared.def

CFLAGS		= $(BASE_CFLAGS) $(LDFLAGS) -O3 -fomit-frame-pointer

# OUTPUT FILES

OUT = ..

I386	= $(OUT)/gamei386.so
I486	= $(OUT)/gamei486.so
I586	= $(OUT)/gamei586.so
I686	= $(OUT)/gamei686.so

SPARC	= $(OUT)/gamesparc.so

#############################################################################
# BUILD
#############################################################################

# All the Source Files
SRC =	src/admin.c src/auth.c src/bans.c src/id_stuff.c src/input.c \
		src/logging.c src/strings.c src/version.c src/wget.c \
		../crypt-string/src/codec.c

# Linux / FreeBSD Builds

menu:
	@echo "**********************************"
	@echo " Type 'make <tag>'"
	@echo " where 'tag' is one of :"
	@echo " default"
	@echo " i486, i586, i686"
	@echo " sol-intel, sol-sparc"
	@echo "**********************************"

default:
	$(CC) $(CFLAGS) $(SHFLAGS) $(SRC) -o $(I386)

i486:
	$(CC) $(CFLAGS) $(SHFLAGS) -march=i486 $(SRC) -o $(I486)

i586:
	$(CC) $(CFLAGS) $(SHFLAGS) -march=i586 $(SRC) -o $(I586)

i686:
	$(CC) $(CFLAGS) $(SHFLAGS) -march=i686 $(SRC) -o $(I686)

# Solaris Builds

sol-intel:
	$(CC) $(CFLAGS) $(SHFLAGS) $(SRC) src/g_so.c -o $(I386)

sol-sparc:
	$(CC) $(CFLAGS) $(SHFLAGS) $(SRC) src/g_so.c -o $(SPARC)

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f *.o *~ $(OUT)/*.so core

distclean: clean
	-rm -f *.gz *.bz2
