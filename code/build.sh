#!/bin/sh

#
#  (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
#  This file is part of ONS, see COPYING for details.
#

#
# File information:
# - Created: 15. March 2009
# - Lead-Dev: - David Herrmann
# - Contributors: /
# - Last-Change: 25. May 2009
#

#
# This script builds the ONS libraries on systems supporting
# an SH like shell and the GNU autotools.
#


#
# Remove "./" from the progname.
#
PROGNAME=$(echo $0 | sed -e 's/^\.\///g')


#
# Print help.
#
echo "                                                "
echo "           #####################                "
echo "           ##    ONS build    ##                "
echo "           #####################                "
echo "                                                "
echo "This script creates the ONS libraries on a POSIX"
echo "    system which supports the GNU autotools.    "
echo "                                                "
echo "Invoke the script with ./$PROGNAME [parameters] "
echo "          Where [parameters] can be:            "
echo "       - 'build': Builds the libraries.         "
echo "       - 'install': Installs the libraries.     "
echo "       - 'uninstall': Uninstalls the libraries. "
echo "       - 'rebuild': Cleans the caches and builds"
echo "                    the libraries again.        "
echo "                                                "
echo "                                                "


#
# Check all parameters and print usage if an error occurred.
#
REBUILD="no"
BUILD="no"
INSTALL="no"
CONFIGURE=""
for i in "$@" ; do
    case "$i" in
        rebuild)
            REBUILD="yes"
            ;;
        install)
            INSTALL="yes"
            ;;
        build)
            BUILD="yes"
            ;;
        *)
            echo "(Re)set configure options to: '$i'"
            CONFIGURE=$i
            REBUILD="yes"
            ;;
    esac
done
if test $REBUILD = "no" -a $BUILD = "no" -a $INSTALL = "no" ; then
    echo "Error: Please specify at least one parameter of: REBUILD BUILD INSTALL"
    exit 1
fi


#
# Helper function.
# If the parameter is not 0, we exit.
#
check_ret() {
    if test "x$1" = "x0" ; then
        # Do nothing.
        echo "nothing" >/dev/null
    else
        echo "Operation failed."
        exit 1
    fi
}


#
# Check that we are in the right directory.
#
if test -f ./build.sh ; then
    CREATION_TIME=$(grep Created ./build.sh | grep -v TIME)
    if test "$CREATION_TIME" != "# - Created: 15. March 2009" ; then
        echo "Error: Please change into the directory of the ./build.sh script."
        exit 1;
    else
        echo "Changing into directory './build/'."
        cd build
        check_ret $?
    fi
else
    echo "Error: Please change into the directory of the ./build.sh script."
fi


#
# Create the autotools only if they are not yet created.
#
if test $REBUILD = "yes" -o ! -f ./configure ; then
    echo "Copying 'install-sh'."
    cp /usr/share/automake-1.10/install-sh .
    check_ret $?
    echo "Copying 'config.sub'."
    cp /usr/share/automake-1.10/config.sub .
    check_ret $?
    echo "Copying 'config.guess'."
    cp /usr/share/automake-1.10/config.guess .
    check_ret $?
    echo "Invoking 'autoreconf'."
    autoreconf
    check_ret $?
    if test "x$CONFIGURE" = "x" ; then
        echo "CONF: ./configure --enable-warnings --enable-debug"
        ./configure --enable-warnings --enable-debug
    else
        echo "CONF: ./configure $CONFIGURE"
        ./configure $CONFIGURE
    fi
    check_ret $?
fi


#
# Build the source.
#
if test $BUILD = "yes" ; then
    echo "BUILD: make"
    make
    check_ret $?
fi


#
# Install the source
#
if test $INSTALL = "yes" ; then
    echo "INSTALL: make install"
    make install
    check_ret $?
fi


#
# Helper function which creates a makefile depending on which modules
# have to be compiled.
#
create_makefile() {
    if test "x$1" = "x" ; then
        MODULES="sundry"
    else
        MODULES="$1"
    fi
}
