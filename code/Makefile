#
#  (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
#  This file is part of ONS, see COPYING for details.
#

#
# File information:
# - Created: 17. January 2009
# - Lead-Dev: - David Herrmann
# - Contributors: /
# - Last-Change: 27. May 2009
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

# Hint: (ADDMOD)
# Libraries we link against.
SUNDRY_LIBS=-lpthread
MEMORIA_LIBS=
ASYNCHIO_LIBS=

# Prefix where to install the libraries and includes.
# Include files are copied to $(PREFIX)/include
# Libraries are copied to $(PREFIX)/lib
PREFIX=/usr

# Hint: (ADDMOD)
# Phony targets.
.PHONY=none clean install uninstall \
 sundry sundry_begin sundry_end \
 memoria memoria_begin memoria_end \
 asynchio asynchio_begin asynchio_end


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
	@rm -fv $(CODEDIR)/asynchio/src/*.o
	@rm -fv $(CODEDIR)/libsundry.*
	@rm -fv $(CODEDIR)/libmemoria.*
	@rm -fv $(CODEDIR)/libasynchio.*
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

	@cd $(CODEDIR) && for file in libasynchio.* ; do \
		if test ! $$file = "libasynchio.*" ; then \
			INST="yes" ; \
			cp -v $$file $(PREFIX)/lib/ ; \
			chmod 755 $(PREFIX)/lib/$$file ; \
		fi ; \
	done ; \
	if test "x$$INST" = "xyes" ; then \
		cp -Rv asynchio/include/asynchio $(PREFIX)/include/ ; \
		chmod 755 $(PREFIX)/include/asynchio ; \
		chmod 644 $(PREFIX)/include/asynchio/*.h ; \
	fi ;

# Hint: (ADDMOD)
uninstall:
	@rm -vRf $(PREFIX)/include/sundry $(PREFIX)/include/memoria $(PREFIX)/include/asynchio \
 $(PREFIX)/lib/libsundry.* $(PREFIX)/lib/libmemoria.* $(PREFIX)/lib/libasynchio.*


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
	$(CC) $(LFLAGS) $(DFLAGS) $(SUNDRY_OBJECTS) -o $(CODEDIR)/libsundry.$(SUFFIX) $(SUNDRY_LIBS)

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

memoria: sundry memoria_begin $(CODEDIR)/libmemoria.$(SUFFIX) memoria_end

memoria_begin:
	@echo "Building module 'memoria'..."

memoria_end:
	@echo "Module 'memoria' successfully built."

$(CODEDIR)/libmemoria.$(SUFFIX): $(MEMORIA_OBJECTS)
	$(CC) $(LFLAGS) $(DFLAGS) $(CODEDIR)/libsundry.$(SUFFIX) $(MEMORIA_OBJECTS) -o $(CODEDIR)/libmemoria.$(SUFFIX) $(MEMORIA_LIBS) 

# TODO:
# We currently fix the build with a force "-no-pedantic" (and similar) here!
$(CODEDIR)/memoria/src/%.o: $(CONFIG) $(MEMORIA_TINCLUDES)
	$(CC) $(CFLAGS) -no-pedantic -std=gnu99 $(MEMORIA_INCPATH) $(CODEDIR)/memoria/src/$*.c -o $(CODEDIR)/memoria/src/$*.o


#
# Module: Asynchio
# Metatargets to build asynchio.
#

ASYNCHIO_SOURCES=obj.c
ASYNCHIO_INCLUDES=asynchio.h

ASYNCHIO_TSOURCES=$(foreach file,$(ASYNCHIO_SOURCES),$(CODEDIR)/asynchio/src/$(file))
ASYNCHIO_OBJECTS=$(ASYNCHIO_TSOURCES:%.c=%.o)
ASYNCHIO_TINCLUDES=$(foreach file,$(ASYNCHIO_INCLUDES),$(CODEDIR)/asynchio/include/asynchio/$(file))
ASYNCHIO_INCPATH=-I$(CODEDIR)/sundry/include -I$(CODEDIR)/memoria/include -I$(CODEDIR)/asynchio/include

asynchio: sundry memoria asynchio_begin $(CODEDIR)/libasynchio.$(SUFFIX) asynchio_end

asynchio_begin:
	@echo "Building module 'asynchio'..."

asynchio_end:
	@echo "Module 'asynchio' successfully built."

$(CODEDIR)/libasynchio.$(SUFFIX): $(ASYNCHIO_OBJECTS)
	$(CC) $(LFLAGS) $(DFLAGS) $(CODEDIR)/libsundry.$(SUFFIX) $(CODEDIR)/libmemoria.$(SUFFIX) $(ASYNCHIO_OBJECTS) -o $(CODEDIR)/libasynchio.$(SUFFIX) $(ASYNCHIO_LIBS)

$(CODEDIR)/asynchio/src/%.o: $(CONFIG) $(ASYNCHIO_TINCLUDES)
	$(CC) $(CFLAGS) $(ASYNCHIO_INCPATH) $(CODEDIR)/asynchio/src/$*.c -o $(CODEDIR)/asynchio/src/$*.o

