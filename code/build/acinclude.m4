dnl
dnl (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
dnl This file is part of ONS, see COPYING for details.
dnl

dnl
dnl File information:
dnl - Created: 22. December 2008
dnl - Lead-Dev: - David Herrmann
dnl - Contributors: /
dnl - Last-Change: 15. March 2009
dnl

dnl
dnl This file contains outsourced autoconf macros.
dnl Each macro is documented in it's comment.
dnl


dnl
dnl Verifies that the compiler actually halts after the
dnl compilation phase without generating any object
dnl file, when the source compiles with errors.
dnl

AC_DEFUN([ONS_C_HALT_ON_ERROR],
[
    AC_MSG_CHECKING([the C compiler to halt on compilation errors])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[ this cant compile ]],[[force compilation error]])],
        [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([compiler does not halt on compilation errors.])
        ],
        [AC_MSG_RESULT([yes])]
    )
])


dnl
dnl Verifies that the precompiler supports indented macros.
dnl

AC_DEFUN([ONS_C_INDENTED_MACROS],
[
    AC_MSG_CHECKING([the precompiler to support indented macros])
    AC_TRY_RUN([
#define NOT_USED_MACRO
    #define TEST_MACRO
int main() {
#ifdef TEST_MACRO
    return 0;
#else
    return 1;
#endif
}
        ],
        [AC_MSG_RESULT([yes])],
        [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([Your precompiler does not support indented macros.])
        ]
    )
])


dnl
dnl Checks whether 1 byte == 8 bit, otherwise terminate the process.
dnl

AC_DEFUN([ONS_C_EIGHT_BIT_SYSTEM],
[
    AC_MSG_CHECKING([whether 1 byte is equal to 8 bits])
    AC_TRY_RUN([
                #include <limits.h>
                #include <stdlib.h>
                int main() {
                    if(CHAR_BIT == 8) return EXIT_SUCCESS;
                    return EXIT_FAILURE;
                }
                ],
                [AC_MSG_RESULT([yes])],
                [
                 AC_MSG_RESULT([no])
                 AC_MSG_ERROR([1 byte is not equal to 8 bits on your system.])
                ]
    )
])


dnl
dnl Verifies C99 support.
dnl

AC_DEFUN([ONS_C_C99],
[
    AC_PROG_CC_C99
    AC_MSG_CHECKING([for C99 support])
    if test x$ac_cv_prog_cc_c99 = "xno" ; then
        AC_MSG_RESULT([no])
    else
        AC_MSG_RESULT([yes])
    fi
])


dnl
dnl Search an endian.h similar header and define an include directive.
dnl $ons_include_endian_header is used in later AC_RUN_IFELSE directives.
dnl The asm headers are separated because they can conflict with the other ones.
dnl
dnl The following headers are checked and have the C definition set:
dnl   sys/param.h endian.h machine/endian.h byteorder.h machine/byteorder.h arch/byteorder.h
dnl   arch/endian.h asm/endian.h asm/byteorder.h
dnl
dnl Defines either BIG_ENDIAN or LITTLE_ENDIAN. $ons_cv_endianness contains either "LittleEndian"
dnl or "BigEndian".
dnl

AC_DEFUN([ONS_C_ENDIANNESS], [
    AC_CHECK_HEADERS(sys/param.h endian.h machine/endian.h byteorder.h machine/byteorder.h arch/byteorder.h arch/endian.h)
    AC_CHECK_HEADERS(asm/endian.h asm/byteorder.h)
    ons_include_endian_header="
        #ifdef HAVE_SYS_PARAM_H
            #include <sys/param.h>
        #endif
        #ifdef HAVE_ENDIAN_H
            #include <endian.h>
        #endif
        #ifdef HAVE_MACHINE_ENDIAN_H
            #include <machine/endian.h>
        #endif
        #ifdef HAVE_BYTEORDER_H
            #include <byteorder.h>
        #endif
        #ifdef HAVE_MACHINE_BYTEORDER_H
            #include <machine/byteorder.h>
        #endif
        #ifdef HAVE_ARCH_BYTEORDER_H
            #include <arch/byteorder.h>
        #endif
        #ifdef HAVE_ARCH_ENDIAN_H
            #include <arch/endian.h>
        #endif
    "
    ons_include_endian_header_asm="
        #ifdef HAVE_ASM_BYTEORDER_H
            #include <asm/byteorder.h>
        #endif
        #ifdef HAVE_ASM_ENDIAN_H
            #include <asm/endian.h>
        #endif
    "
    dnl
    dnl Let's check the endianness of the system.
    dnl
    AC_MSG_CHECKING([the endianness of your system])
    ons_cv_endianness="Unknown"
    ons_cv_endian_force_big="no";
    ons_cv_endian_force_little="no";
    AC_ARG_ENABLE([bigendian],
        [  --enable-bigendian      Set the endianness of your system to bigendian.],
        [ons_cv_endian_force_big=$enable_bigendian],
    )
    AC_ARG_ENABLE([littleendian],
        [  --enable-littleendian   Set the endianness of your system to littleendian.],
        [ons_cv_endian_force_little=$enable_littleendian],
    )
    AC_TRY_RUN([
                #include <stdlib.h>
                $ons_include_endian_header
                $ons_include_endian_header_asm

                int main() {
                #ifndef LITTLE_ENDIAN
                    #define LITTLE_ENDIAN __LITTLE_ENDIAN
                #endif
                #ifndef BYTE_ORDER
                    #define BYTE_ORDER __BYTE_ORDER
                #endif
                    if(LITTLE_ENDIAN == BYTE_ORDER) return EXIT_SUCCESS;
                    return EXIT_FAILURE;
                }
                ],[ons_cv_endian_little="yes"
                   ons_cv_endianness="LittleEndian"],[]
    )
    if test x"$ons_cv_endianness" = xUnknown; then
        AC_TRY_RUN([
                    #include <stdlib.h>
                    $ons_include_endian_header
                    $ons_include_endian_header_asm

                    int main() {
                    #ifndef BIG_ENDIAN
                        #define BIG_ENDIAN __BIG_ENDIAN
                    #endif
                    #ifndef BYTE_ORDER
                        #define BYTE_ORDER __BYTE_ORDER
                    #endif
                        if(BIG_ENDIAN == BYTE_ORDER) return EXIT_SUCCESS;
                        return EXIT_FAILURE;
                    }
                    ],[ons_cv_endian_big="yes"
                       ons_cv_endianness="BigEndian"],[]
        )
    fi
    if test x"$ons_cv_endianness" = xUnknown; then
        AC_TRY_RUN([
            main () {
                union {
                    long l;
                    char c[sizeof(long)];
                } u;
                u.l = 1;
                if(sizeof(long) < 4) exit(1);
                exit(!(u.c[sizeof (long) - 1] == 1));
            }], [ons_cv_endian_big="yes"
                 ons_cv_endianness="BigEndian"]
        )
        if test x"$ons_cv_endianness" = xUnknown; then
            AC_TRY_RUN([
                main () {
                    union {
                        long l;
                        char c[sizeof(long)];
                    } u;
                    u.l = 1;
                    if(sizeof(long) < 4) exit(1);
                    exit(!(u.c[0] == 1));
                }], [ons_cv_endian_little="yes"
                     ons_cv_endianness="LittleEndian"]
            )
        fi
    fi
    if test x"$ons_cv_endian_force_big" = xyes; then
        if test x"$ons_cv_endian_force_little" = xyes; then
            AC_MSG_ERROR([You enabled --enable-bigendian and --enable-littleendian. Please specify only one of them.])
        fi
        ons_cv_endian_big="yes"
        ons_cv_endianness="BigEndian"
    elif test x"$ons_cv_endian_force_little" = xyes; then
        ons_cv_endian_little="yes"
        ons_cv_endianness="LittleEndian"
    fi
    if test x"$ons_cv_endianness" = xBigEndian; then
        AC_DEFINE([BIG_ENDIAN], [1], [Define to 1 if system is bigendian.])
    else
        AC_DEFINE([LITTLE_ENDIAN], [1], [Define to 1 if system is littleendian.])
    fi
    AC_MSG_RESULT([$ons_cv_endianness])
])


dnl
dnl Now we check for compiler features to disabled structure padding.
dnl There are several mechanisms and we check for all and enable only the best way.
dnl The checks may be overpowered but we really need to know whether these mechanisms
dnl work, also at least the #pragma compiles, whether or not it is supported.
dnl
dnl First check whether the compiler supports the '__attribute__((__packed__))' feature.
dnl

AC_DEFUN([ONS_C_NO_PADDING], [
    AC_MSG_CHECKING(whether '__attribute__((__packed__))' is supported)
    AC_RUN_IFELSE(
        [[
            #include <stddef.h>
            struct test_struct {
                char test_a;
                int test_b;
                char test_c;
                char test_d;
                long test_e;
                short test_f;
            } __attribute__((__packed__));
            int main() {
                struct test_struct test_object;
                return ((unsigned int)!(sizeof(test_object) == (sizeof(char) +
                                                 sizeof(int) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(long) +
                                                 sizeof(short)) &&
                        offsetof(struct test_struct, test_a) == 0 &&
                        offsetof(struct test_struct, test_b) == sizeof(char) &&
                        offsetof(struct test_struct, test_c) == (offsetof(struct test_struct, test_b) + sizeof(int)) &&
                        offsetof(struct test_struct, test_d) == (offsetof(struct test_struct, test_c) + sizeof(char)) &&
                        offsetof(struct test_struct, test_e) == (offsetof(struct test_struct, test_d) + sizeof(char)) &&
                        offsetof(struct test_struct, test_f) == (offsetof(struct test_struct, test_e) + sizeof(long))
                        ));
            }
        ]],
        [ons_cv_pack_attr="yes"],
        [ons_cv_pack_attr="no"]
    )
    AC_MSG_RESULT([$ons_cv_pack_attr])
    dnl
    dnl Now check whether the compiler supports the '#pragma pack' feature.
    dnl
    AC_MSG_CHECKING([[whether '#pragma pack' is supported]])
    AC_RUN_IFELSE(
        [[
            #include <stddef.h>
            #pragma pack(push)
            #pragma pack(1)
            struct test_struct {
                char test_a;
                int test_b;
                char test_c;
                char test_d;
                long test_e;
                short test_f;
            };
            #pragma pack(pop)
            int main() {
                struct test_struct test_object;
                return ((unsigned int)!(sizeof(test_object) == (sizeof(char) +
                                                 sizeof(int) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(long) +
                                                 sizeof(short)) &&
                        offsetof(struct test_struct, test_a) == 0 &&
                        offsetof(struct test_struct, test_b) == sizeof(char) &&
                        offsetof(struct test_struct, test_c) == (offsetof(struct test_struct, test_b) + sizeof(int)) &&
                        offsetof(struct test_struct, test_d) == (offsetof(struct test_struct, test_c) + sizeof(char)) &&
                        offsetof(struct test_struct, test_e) == (offsetof(struct test_struct, test_d) + sizeof(char)) &&
                        offsetof(struct test_struct, test_f) == (offsetof(struct test_struct, test_e) + sizeof(long))
                        ));
            }
        ]],
        [ons_cv_pack_pragma="yes"],
        [ons_cv_pack_pragma="no"]
    )
    AC_MSG_RESULT([$ons_cv_pack_pragma])
    dnl
    dnl Select the best method to remove padding.
    dnl
    if test $ons_cv_pack_attr = yes ; then
        AC_DEFINE([HAVE_ATTR_PACKED], [1], [Define if '__attribute__((__packed__))' is supported.])
    fi
    if test $ons_cv_pack_pragma = yes ; then
        AC_DEFINE([HAVE_PRAGMA_PACK], [1], [Define if '#pragma pack' is supported.])
    elif test $ons_cv_pack_attr = no ; then
        AC_MSG_ERROR([No method found to pack a structure.])
    fi
])


dnl
dnl Written by John Hawkinson <jhawk@mit.edu>. This code is in the Public
dnl Domain.
dnl
dnl This test is for network applications that need socket() and
dnl gethostbyname() -ish functions.  Under Solaris, those applications need to
dnl link with "-lsocket -lnsl".  Under IRIX, they should *not* link with
dnl "-lsocket" because libsocket.a breaks a number of things (for instance:
dnl gethostbyname() under IRIX 5.2, and snoop sockets under most versions of
dnl IRIX).
dnl 
dnl Unfortunately, many application developers are not aware of this, and
dnl mistakenly write tests that cause -lsocket to be used under IRIX.  It is
dnl also easy to write tests that cause -lnsl to be used under operating
dnl systems where neither are necessary (or useful), such as SunOS 4.1.4, which
dnl uses -lnsl for TLI.
dnl 
dnl This test exists so that every application developer does not test this in
dnl a different, and subtly broken fashion.
dnl 
dnl It has been argued that this test should be broken up into two seperate
dnl tests, one for the resolver libraries, and one for the libraries necessary
dnl for using Sockets API. Unfortunately, the two are carefully intertwined and
dnl allowing the autoconf user to use them independantly potentially results in
dnl unfortunate ordering dependancies -- as such, such component macros would
dnl have to carefully use indirection and be aware if the other components were
dnl executed. Since other autoconf macros do not go to this trouble, and almost
dnl no applications use sockets without the resolver, this complexity has not
dnl been implemented.
dnl
dnl The check for libresolv is in case you are attempting to link statically
dnl and happen to have a libresolv.a lying around (and no libnsl.a).
dnl
AC_DEFUN([ONS_LIBRARY_NET], [
   # Most operating systems have gethostbyname() in the default searched
   # libraries (i.e. libc):
   AC_CHECK_FUNC(gethostbyname, ,
     # Some OSes (eg. Solaris) place it in libnsl:
     AC_CHECK_LIB(nsl, gethostbyname, , 
       # Some strange OSes (SINIX) have it in libsocket:
       AC_CHECK_LIB(socket, gethostbyname, ,
          # Unfortunately libsocket sometimes depends on libnsl.
          # AC_CHECK_LIB's API is essentially broken so the following
          # ugliness is necessary:
          AC_CHECK_LIB(socket, gethostbyname,
             LIBS="-lsocket -lnsl $LIBS",
               AC_CHECK_LIB(resolv, gethostbyname),
             -lnsl)
       )
     )
   )
  AC_CHECK_FUNC(socket, , AC_CHECK_LIB(socket, socket, ,
    AC_CHECK_LIB(socket, socket, LIBS="-lsocket -lnsl $LIBS", , -lnsl)))
])


dnl
dnl Checks whether the socket() call allows binary concatenation of the
dnl "type" parameter with SOCK_NONBLOCK and SOCK_CLOEXEC.
dnl

AC_DEFUN([ONS_EXT_SOCKET], [
    AC_MSG_CHECKING([for extended 'type' parameter of socket()])
    AC_TRY_RUN(
    [
        #include <sys/types.h>
        #include <sys/socket.h>
        main() {
            signed int fd, type;
            size_t size = sizeof(signed int);
            if((fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) < 0)
                exit(1);
            if(getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &size) != 0) return 0;
            if(type != SOCK_STREAM) exit(1);
            return 0;
        }
    ],
    [ AC_MSG_RESULT(yes)
      AC_DEFINE([HAVE_EXT_SOCKET], [1], [Define if the "type" parameter of socket() allows binary options.])
      ons_cv_ext_socket="yes" ],
    [ AC_MSG_RESULT(no)
      ons_cv_ext_socket="no" ]
    )
])


dnl
dnl Checks for accept4()
dnl

AC_DEFUN([ONS_CHECK_ACCEPT4], [
    AC_MSG_CHECKING([for accept4() syscall])
    AC_TRY_RUN(
    [
        #include <sys/types.h>
        #include <sys/socket.h>
        main() {
            accept4(0, NULL, NULL, 0);
            return 0;
        }
    ],
    [ AC_MSG_RESULT(yes)
      AC_DEFINE([HAVE_ACCEPT4], [1], [Define if the accept4() syscall is available.])
      ons_cv_accept4="yes" ],
    [ AC_MSG_RESULT(no)
      ons_cv_accept4="no" ]
    )
])

