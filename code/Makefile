#
#  (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
#  This file is part of ONS, see COPYING for details.
#

#
# File information:
# - Created: 17. January 2009
# - Lead-Dev: - David Herrmann
# - Contributors: /
# - Last-Change: 26. May 2009
#

#
# Compiles libons.
# Each module has an own meta-target here. To build the module just
# build the meta-target.
#


#
# Configuration
#

# Path to the "code" directory.
CODEDIR=.

# Path to machine.h file.
CONFIG=$(CODEDIR)/config/machine.h

# Compiler to use.
CC=gcc

# Config to use.
CONFDEF=ONS_SYS_GENERIC

# Extra flags.
CEXTRA=-Wno-unused-function

# Basic compiler flags.
CFLAGS=-O2 -Wall -fPIC -c -I$(CODEDIR)/config -D$(CONFDEF) $(CEXTRA)

# Basic linker flags.
LFLAGS=-fPIC -Wall -O2

# Dynamic library additional linker flags.
DFLAGS=-shared

# Suffix of dynamic libraries.
SUFFIX=so

# Libraries we link against.
SUNDRY_LIBS=-lpthread
MEMORIA_LIBS=

# Prefix where to install the libraries and includes.
# Include files are copied to $(PREFIX)/include
# Libraries are copied to $(PREFIX)/lib
PREFIX=/usr

# Phony targets.
.PHONY=none clean install uninstall \
 sundry sundry_begin sundry_end \
 memoria memoria_begin memoria_end


#
# Default targets.
#

none:
	@echo "Please specify a target."

# Hint: (ADDMOD)
clean:
	@echo "Removing the following files:"
	@rm -fv $(CODEDIR)/sundry/src/*.o
	@rm -fv $(CODEDIR)/memoria/src/*.o
	@rm -fv $(CODEDIR)/libsundry.*
	@rm -fv $(CODEDIR)/libmemoria.*
	@echo "Successfully cleaned the directory."

# Hint: (ADDMOD)
install:
	@test -d $(PREFIX) || mkdir -vp $(PREFIX)

	@cd $(CODEDIR) && for file in libsundry.* ; do \
		if test ! $$file = "libsundry.*" ; then \
			INST="yes" ; \
			cp -v $$file $(PREFIX)/lib/ ; \
			chmod 755 $(PREFIX)/lib/$$file ; \
		fi ; \
	done ; \
	if test "x$$INST" = "xyes" ; then \
		cp -Rv sundry/include/sundry $(PREFIX)/include/ ; \
		chmod 755 $(PREFIX)/include/sundry ; \
		chmod 644 $(PREFIX)/include/sundry/*.h ; \
	fi ;

	@cd $(CODEDIR) && for file in libmemoria.* ; do \
		if test ! $$file = "libmemoria.*" ; then \
			INST="yes" ; \
			cp -v $$file $(PREFIX)/lib/ ; \
			chmod 755 $(PREFIX)/lib/$$file ; \
		fi ; \
	done ; \
	if test "x$$INST" = "xyes" ; then \
		cp -Rv memoria/include/memoria $(PREFIX)/include/ ; \
		chmod 755 $(PREFIX)/include/memoria ; \
		chmod 644 $(PREFIX)/include/memoria/*.h ; \
	fi ;

# Hint: (ADDMOD)
uninstall:
	@rm -vRf $(PREFIX)/include/sundry $(PREFIX)/include/memoria $(PREFIX)/lib/libsundry.* $(PREFIX)/lib/libmemoria.*


# Hint: (ADDMOD)
# Add own section for every module.

#
# Module: Sundry
# Metatargets to build sundry.
#

SUNDRY_SOURCES=thread.c time.c
SUNDRY_INCLUDES=sundry.h thread.h time.h

SUNDRY_TSOURCES=$(foreach file,$(SUNDRY_SOURCES),$(CODEDIR)/sundry/src/$(file))
SUNDRY_OBJECTS=$(SUNDRY_TSOURCES:%.c=%.o)
SUNDRY_TINCLUDES=$(foreach file,$(SUNDRY_INCLUDES),$(CODEDIR)/sundry/include/sundry/$(file))
SUNDRY_INCPATH=-I$(CODEDIR)/sundry/include

sundry: sundry_begin $(CODEDIR)/libsundry.$(SUFFIX) sundry_end

sundry_begin:
	@echo "Building module 'sundry'..."

sundry_end:
	@echo "Module 'sundry' successfully built."

$(CODEDIR)/libsundry.$(SUFFIX): $(SUNDRY_OBJECTS)
	$(CC) $(LFLAGS) $(DFLAGS) $(SUNDRY_LIBS) $(SUNDRY_OBJECTS) -o $(CODEDIR)/libsundry.$(SUFFIX)

$(CODEDIR)/sundry/src/%.o: $(CONFIG) $(SUNDRY_TINCLUDES)
	$(CC) $(CFLAGS) $(SUNDRY_INCPATH) $(CODEDIR)/sundry/src/$*.c -o $(CODEDIR)/sundry/src/$*.o


#
# Module: Memoria
# Metatargets to build memoria.
#

MEMORIA_SOURCES=random.c hash.c list.c rbtree.c splay.c table.c
MEMORIA_INCLUDES=memoria.h alloc.h array.h list.h

MEMORIA_TSOURCES=$(foreach file,$(MEMORIA_SOURCES),$(CODEDIR)/memoria/src/$(file))
MEMORIA_OBJECTS=$(MEMORIA_TSOURCES:%.c=%.o)
MEMORIA_TINCLUDES=$(foreach file,$(MEMORIA_INCLUDES),$(CODEDIR)/memoria/include/memoria/$(file))
MEMORIA_INCPATH=-I$(CODEDIR)/sundry/include -I$(CODEDIR)/memoria/include

memoria: memoria_begin $(CODEDIR)/libmemoria.$(SUFFIX) memoria_end

memoria_begin:
	@echo "Building module 'memoria'..."

memoria_end:
	@echo "Module 'memoria' successfully built."

$(CODEDIR)/libmemoria.$(SUFFIX): $(MEMORIA_OBJECTS)
	$(CC) $(LFLAGS) $(DFLAGS) $(MEMORIA_LIBS) $(MEMORIA_OBJECTS) -o $(CODEDIR)/libmemoria.$(SUFFIX)

$(CODEDIR)/memoria/src/%.o: $(CONFIG) $(MEMORIA_TINCLUDES)
	$(CC) $(CFLAGS) $(MEMORIA_INCPATH) $(CODEDIR)/memoria/src/$*.c -o $(CODEDIR)/memoria/src/$*.o

