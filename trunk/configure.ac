AC_PREREQ(2.59)
AC_INIT([CID], [1.1.1_beta1], [Benjamin SANS (sans_ben@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(foreign)
AC_PROG_LIBTOOL

IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cid
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC

# Checks for libraries.
PKG_CHECK_MODULES(PACKAGE, "gtk+-2.0 dbus-1 dbus-glib-1 libxml-2.0 gthread-2.0")
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)


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
