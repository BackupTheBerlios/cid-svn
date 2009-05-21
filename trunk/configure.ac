AC_PREREQ(2.59)
AC_INIT([CID], [1.1.2], [Benjamin SANS (sans_ben@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(foreign)
AC_PROG_LIBTOOL

IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cid
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

# Checks for programs.
# gcc
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC

# Checks for libraries.
PKG_CHECK_MODULES(PACKAGE, "gtk+-2.0 dbus-1 dbus-glib-1 libxml-2.0 gthread-2.0")
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

AC_ARG_ENABLE(opengl,
        [ --enable-opengl  :  Enable OpenGL],
        [case "${enableval}" in
            yes) useopengl=true ;;
            no)  useopengl=false ;;
            *)   echo "bad value ${enableval} for --enable-opengl" ;;
        esac],[useopengl=false])

if test "$useopengl" = "true" ; then
        PKG_CHECK_MODULES(OPENGL, "gtkglext-1.0", [opengl_detected=true], [opengl_detected=false; AC_MSG_WARN(Could not find dtkdlext-1.0; C.I.D. won't be built with OpenGL support.)])
        AC_SUBST(OPENGL_CFLAGS)
        AC_SUBST(OPENGL_LIBS)

        if test ! "$opengl_detected" = "false" ; then
            AC_DEFINE(HAVE_OPENGL,"1",[Defined if we can use OpenGL.])
        fi
fi

if test "$opengl_detected" = "true" ; then
        opengl_required="gtkglext-1.0"
else
        opengl_required=""
fi

AC_SUBST(opengl_required)

AM_CONDITIONAL( [WITH_OPENGL], [test "$opengl_detected" = "true" ] )

AC_CONFIG_FILES([
		Makefile
		cid.pc
		src/Makefile
		data/Makefile
		data/cid.conf
		po/Makefile.in
		doc/dox.config
])
AC_OUTPUT

echo "C.I.D. will be compiled with the following options :"
if test "$opengl_detected" = "true" ; then
    echo " * Use OpenGL : yes"
else
    echo " * Use OpenGL : no"
fi
