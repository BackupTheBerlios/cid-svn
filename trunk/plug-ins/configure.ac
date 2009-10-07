#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cid-plugins],[1.1.4],[Benjamin SANS (sans_ben@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT


############# PACKAGE CAIRO-DOCK #################
GETTEXT_PACKAGE=cid-plugins
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

pkgdatadir=[`pkg-config cid --variable=pluginsdatadir`]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cid --variable=pluginsdir`]
AC_SUBST(libdir)

srcdir="."
AC_SUBST(srcdir)
top_srcdir=".."
AC_SUBST(top_srcdir)
top_builddir=".."
AC_SUBST(top_builddir)

PKG_CHECK_MODULES(PACKAGE, "cid gtk+-2.0 dbus-1 dbus-glib-1 libxml-2.0 gthread-2.0")


############# COMPILATION CONDITIONNELLE #################
AC_ARG_ENABLE([old-gnome-integration],
	[  --enable-old-gnome-integration : Enable Gnome<2.22 integration (default=yes) ],
	[ with_old_gnome_support=${enableval} ],
	[ with_old_gnome_support=yes ])

############# PACKAGES ADDITIONNELS #################
if test "$with_old_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(OLD_GNOME_INTEGRATION,gnome-vfs-2.0 libgnomeui-2.0, ,[ with_old_gnome_support=false; AC_MSG_WARN(Could not find gnome-vfs and/or gnomeui; Cairo-Dock won't be built with Gnome<2.22 support.) ])
	AC_SUBST(OLD_GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(OLD_GNOME_INTEGRATION_LIBS)
fi
if test "$with_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(GNOME_INTEGRATION,gio-2.0, ,[ with_gnome_support=false; AC_MSG_WARN(Could not find gio; Cairo-Dock won't be built with Gnome>=2.22 support.) ])
	AC_SUBST(GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(GNOME_INTEGRATION_LIBS)
fi

AC_PROG_CXX

AM_CONDITIONAL( [WITH_OLD_GNOME_SUPPORT], [test "$with_old_gnome_support" = "yes"] )

############# GNOME-INTEGRATION-OLD #################
GNOME_INTEGRATION_OLD_PACKAGE=cd-gnome-integration-old

AC_SUBST(VERSION_GNOME_INTEGRATION_OLD, "1.0.2")
AC_SUBST(PACKAGE_GNOME_INTEGRATION_OLD, "$GNOME_INTEGRATION_OLD_PACKAGE")

AC_SUBST(gnome_integration_olddatadir, "${pkgdatadir}/gnome-integration-old")

############# XMMS #################
XMMS_PACKAGE="cd-xmms"
AC_SUBST(GETTEXT_XMMS, "$XMMS_PACKAGE")

AC_SUBST(VERSION_XMMS, "0.2.6")
AC_SUBST(PACKAGE_XMMS, "$XMMS_PACKAGE")

AC_SUBST(xmmsdatadir, "${pkgdatadir}/xmms")

############# WEBLETS #################
WEBLETS_PACKAGE="cd-weblets"

AC_DEFINE(HAVE_WEBKIT,1,[Defined if we want to use webkit.])

AC_SUBST(GETTEXT_WEBLETS, "$WEBLETS_PACKAGE")

AC_SUBST(VERSION_WEBLETS, "0.3.1")
AC_SUBST(PACKAGE_WEBLETS, "$WEBLETS_PACKAGE")

AC_SUBST(webletsdatadir, "${pkgdatadir}/weblets")


AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	
	gnome-integration-old/Makefile
	gnome-integration-old/src/Makefile
	gnome-integration-old/data/Makefile
	
	weblets/Makefile
	weblets/src/Makefile
	weblets/po/Makefile.in
	weblets/data/Makefile
	weblets/data/weblets.conf
	
	xmms/Makefile
	xmms/src/Makefile
	xmms/po/Makefile.in
	xmms/data/Makefile
	xmms/data/xmms.conf
	])

AC_OUTPUT([po/Makefile
	weblets/po/Makefile
	xmms/po/Makefile
	])

echo "cid-plugins will be compiled with the following options :"
echo " * Compil gnome-integration-old plug-in : "${with_old_gnome_support}
echo " * Compil Weblets applet : "${with_weblets}

