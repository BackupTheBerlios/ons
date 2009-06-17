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
# - Last-Change: 26. May 2009
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
echo "       system which supports SH and MAKE.       "
echo "                                                "
echo "Invoke the script with ./$PROGNAME [parameters] "
echo "          Where [parameters] can be:            "
echo "                                                "
echo "       - 'build': Builds the libraries.         "
echo "       - 'install': Installs the libraries.     "
echo "       - 'uninstall': Uninstalls the libraries. "
echo "       - 'clean': Cleans the caches.            "
echo "                                                "
echo "       - 'path=/install/path': Points to the    "
echo "                  path where to install ONS to  "
echo "                  or where to remove ONS from.  "
echo "                  It defaults to '/usr'.        "
echo "       - 'pedantic': Compiles in strict mode.   "
echo "       - 'noclean': Disable implicit 'clean'.   "
echo "       - 'mods=list of mods' Compiles only      "
echo "                             those modules.     "
echo "                                                "
echo "       - 'autodetect': Autodetects the          "
echo "                       environment (default).   "
echo "       - 'macos': Forces Mac OS environment.    "
echo "       - 'linux': Forces Linux environment.     "
echo "       - 'windows': Forces Windows environment. "
echo "       - 'generic': Forces generic environment. "
echo "                                                "
echo "  The order of the parameters is not important. "
echo "     They are sorted by the script itself.      "
echo "                                                "
echo "                                                "


#
# Check all parameters and print usage if an error occurred.
#
CLEAN="no"
BUILD="no"
INSTALL="no"
UNINSTALL="no"
AUTODETECT="yes"
MACOS="no"
LINUX="no"
WINDOWS="no"
GENERIC="no"
IPATH="/usr"
PEDANTIC="no"
NOCLEAN="no"
MODULES="sundry memoria asynchio"
for i in "$@" ; do
    case "$i" in
        clean)
            CLEAN="yes"
            ;;
        install)
            INSTALL="yes"
            ;;
        uninstall)
            UNINSTALL="yes"
            ;;
        build)
            BUILD="yes"
            ;;
        autodetect)
            AUTODETECT="yes"
            ;;
        pedantic)
            PEDANTIC="yes"
            ;;
        noclean)
            NOCLEAN="yes"
            ;;
        macos)
            MACOS="yes"
            AUTODETECT="no"
            ;;
        linux)
            LINUX="yes"
            AUTODETECT="no"
            ;;
        windows)
            WINDOWS="yes"
            AUTODETECT="no"
            ;;
        generic)
            GENERIC="yes"
            AUTODETECT="no"
            ;;
        mods=*)
            IMODS=$(echo "$i" | sed -e 's/^mods=//g')
            if test "x$IMODS" = "x" ; then
                echo "Invalid mods argument: '$i'"
                exit 1
            fi
            MODULES=$IMODS
            ;;
        path=*)
            IPATH=$(echo "$i" | sed -e 's/^path=//g')
            if test "x$IPATH" = "x" ; then
                echo "Invalid path argument: '$i'"
                exit 1
            fi
            ;;
        *)
            echo "Invalid parameter: '$i'"
            exit 1
            ;;
    esac
done
if test $CLEAN = "no" -a $BUILD = "no" -a $INSTALL = "no" ; then
    echo "Error: Please specify at least one parameter of: CLEAN BUILD INSTALL"
    exit 1
fi
if test $NOCLEAN = "no" -a $BUILD = "yes" ; then
    CLEAN="yes"
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
# System detection.
# This sets $SYSTEM to the detected system.
# Valid values are:
#  - macos
#  - linux
#  - windows
#  - generic
#
SYSTEM=""
if test $MACOS = "yes" ; then
    SYSTEM="macos"
elif test $LINUX = "yes" ; then
    SYSTEM="linux"
elif test $WINDOWS = "yes" ; then
    SYSTEM="windows"
elif test $GENERIC = "yes" ; then
    SYSTEM="generic"
else
    # Autodetection.
    TMP=`uname -s`
    if test $TMP = "Darwin" ; then
        SYSTEM="macos"
    elif test $TMP = "Linux" ; then
        SYSTEM="linux"
    else
        SYSTEM="generic"
        echo "Warning: Could not autodetect your system; now using generic configuration."
    fi
fi
echo "Compiling for system: '$SYSTEM'"


#
# Check that we are in the right directory.
#
if test -f ./build.sh ; then
    CREATION_TIME=$(grep Created ./build.sh | grep -v TIME)
    if test "$CREATION_TIME" != "# - Created: 15. March 2009" ; then
        echo "Error: Please change into the directory of the ./build.sh script."
        exit 1;
    fi
else
    echo "Error: Please change into the directory of the ./build.sh script."
fi


#
# Create the autotools only if they are not yet created.
#
if test $CLEAN = "yes" ; then
    echo "CLEAN: make clean"
    make clean
    check_ret $?
fi


#
# Build the source.
#
if test $BUILD = "yes" ; then
    echo "BUILD: make"

    if test $PEDANTIC = "yes" ; then
        CEXTRA="-pedantic -std=c89 -ansi -Wno-unused-function -Wshadow"
    else
        CEXTRA="-Wno-unused-function"
    fi

    if test $SYSTEM = "macos" ; then
        make $MODULES DFLAGS=-dynamiclib SUFFIX=dylib LIBS=-lpthread CONFDEF=ONS_SYS_MACOS "CEXTRA=$CEXTRA"
    elif test $SYSTEM = "linux" ; then
        make $MODULES DFLAGS=-shared SUFFIX=so LIBS=-lpthread CONFDEF=ONS_SYS_LINUX "CEXTRA=$CEXTRA"
    else
        make $MODULES "CEXTRA=$CEXTRA"
    fi
    check_ret $?
fi


#
# Install the source
#
if test $INSTALL = "yes" ; then
    echo "INSTALL: make install"
    make install PREFIX=$IPATH
    check_ret $?
fi


#
# Uninstall the source
#
if test $UNINSTALL = "yes" ; then
    echo "UNINSTALL: make uninstall"
    make uninstall PREFIX=$IPATH
    check_ret $?
fi

