#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cid-CID_APPLET_NAME],[0.0.1],[CID_MY_NAME (CID_MY_MAIL)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CID_PKG=cid-CID_APPLET_NAME
AC_DEFINE_UNQUOTED(GETTEXT_CID_PKG,"$GETTEXT_CID_PKG", [Gettext package.])
AC_SUBST(GETTEXT_CID_PKG)

AC_SUBST(VERSION_CID_PKG, "$VERSION")
AC_SUBST(PACKAGE_CID_PKG, "$PACKAGE")

pkguserdirname="CID_APPLET_NAME"
AC_SUBST(pkguserdirname)
pkgdatadir=[`pkg-config cid --variable=pluginsdatadir`/CID_APPLET_NAME]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cid --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cid)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in])
AC_OUTPUT
