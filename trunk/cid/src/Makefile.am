
SUBDIRS = .

bin_PROGRAMS = \
    cid

#if WITH_OPENGL
#have_opengl=-DHAVE_OPENGL="1"
#endif

#    $(OPENGL_CFLAGS) \
#    $(have_opengl)

if E17_ENABLE
have_e17=-DHAVE_E17="1"
endif

if HAVE_LIBCRYPT
have_libcrypt=-DHAVE_LIBCRYPT="1"
endif

cid_CFLAGS = \
    $(PACKAGE_CFLAGS) \
    $(have_e17) \
    $(have_libcrypt) \
    -DCID_DEFAULT_IMAGE=\""$(pkgdatadir)/default.svg"\"\
    -DCID_CONFIG_FILE=\""cid.conf"\"\
    -DCID_DATA_DIR=\""$(pkgdatadir)"\"\
    -DCID_MODULES_DIR=\""$(libdir)/$(PACKAGE)"\"\
    -DCID_VERSION=\""$(VERSION)"\"\
    -DCID_LOCALE_DIR=\""$(datadir)/locale"\"\
    -DCID_GETTEXT_PACKAGE=\""$(GETTEXT_PACKAGE)"\"\
    -W \
    -Werror \
    -fno-builtin \
    -g  

cid_LDFLAGS = \
    -Wl,--export-dynamic

cid_LDADD = \
    $(PACKAGE_LIBS) \
    $(LIBCRYPT_LIBS)

cid_SOURCES = \
    cid.c \
    cid.h \
    cid-amarok.c \
    cid-amarok.h \
    cid-amarok2.c \
    cid-amarok2.h \
    cid-animation.c \
    cid-animation.h \
    cid-applet-canvas.h \
    cid-asynchrone.c \
    cid-asynchrone.h \
    cid-callbacks.c \
    cid-callbacks.h \
    cid-conf-panel-factory.c \
    cid-conf-panel-factory.h \
    cid-config.c \
    cid-config.h \
    cid-console-call.c \
    cid-console-call.h \
    cid-constantes.h \
    cid-cover.c \
    cid-cover.h \
    cid-dbus.c \
    cid-dbus.h \
    cid-draw.c \
    cid-draw.h \
    cid-exaile.c \
    cid-exaile.h \
    cid-menu-factory.c \
    cid-menu-factory.h \
    cid-messages.c \
    cid-messages.h \
    cid-modules.c \
    cid-modules.h \
    cid-mpd.c \
    cid-panel-callbacks.c \
    cid-panel-callbacks.h \
    cid-rhythmbox.c \
    cid-rhythmbox.h \
    cid-utilities.c \
    cid-utilities.h \
    libmpdclient.c \
    libmpdclient.h
    
    
    
    
    



cidincludedir =\
    $(includedir)/cid

cidinclude_HEADERS =\
    cid.h

cidsubincludedir =\
    $(includedir)/cid/cid

cidsubinclude_HEADERS = \
    cid-amarok.h \
    cid-amarok2.h \
    cid-cover.h \
    cid-animation.h \
    cid-applet-canvas.h \
    cid-asynchrone.h \
    cid-callbacks.h \
    cid-conf-panel-factory.h \
    cid-config.h \
    cid-console-call.h \
    cid-constantes.h \
    cid-dbus.h \
    cid-draw.h \
    cid-exaile.h \
    cid-menu-factory.h \
    cid-messages.h \
    cid-modules.h \
    cid-mpd.h \
    cid-panel-callbacks.h \
    cid-rhythmbox.h \
    cid-struct.h \
    cid-utilities.h \
    libmpdclient.h

