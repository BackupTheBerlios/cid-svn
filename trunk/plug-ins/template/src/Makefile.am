 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
    libcd-CID_APPLET_NAME.la


libcd_CID_LIB_NAME_la_SOURCES = \
    applet-init.c\
    applet-init.h\
    applet-struct.h


cid_version = `pkg-config --modversion cid`

libcd_CID_LIB_NAME_la_CFLAGS = \
    -I$(top_srcdir)/src\
    $(PACKAGE_CFLAGS)\
    -DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
    -DMY_APPLET_USER_DATA_DIR=\""CID_APPLET_NAME"\"\
    -DMY_APPLET_VERSION=\""$(VERSION_CID_PKG)"\"\
    -DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_CID_PKG)"\"\
    -DMY_APPLET_CID_VERSION=\""$(cid_version)"\"\
    -std=c99\
    -Werror-implicit-function-declaration\
    -O3
#   -g -ggdb -W -Wall


libcd_CID_LIB_NAME_la_LIBADD = \
    $(PACKAGE_LIBS) -lm


libcd_CID_LIB_NAME_la_LDFLAGS =\
    -avoid-version -module
