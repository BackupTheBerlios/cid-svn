
SUBDIRS = .

bin_PROGRAMS = \
	cid

cid_CFLAGS = \
	$(PACKAGE_CFLAGS) \
	-DCID_DEFAULT_IMAGE=\""$(pkgdatadir)/default.svg"\"\
	-DCID_CONFIG_FILE=\""cid.conf"\"\
	-DCID_DATA_DIR=\""$(pkgdatadir)"\"\
	-DCID_MODULES_DIR=\""$(libdir)/$(PACKAGE)"\"\
	-DCID_VERSION=\""$(VERSION)"\"\
	-DCID_LOCALE_DIR=\""$(datadir)/locale"\"\
	-DCID_GETTEXT_PACKAGE=\""$(GETTEXT_PACKAGE)"\"\
	-W \
	-fno-builtin \
	-g	

cid_LDFLAGS = \
	-Wl,--export-dynamic \
	$(PACKAGE_LIBS)

cid_SOURCES = \
	cid.c \
	cid.h \
	cid-amarok.c \
	cid-amarok.h \ 
	cid-amarok2.c \
	cid-amarok2.h \
	cid-amazon.c \
	cid-amazon.h \
	cid-animation.c \
	cid-animation.h \
	cid-callbacks.c \
	cid-callbacks.h \
	cid-conf-panel-factory.c \
	cid-conf-panel-factory.h \
	cid-config.c \
	cid-config.h \
	cid-dbus.c \
	cid-dbus.h \
	cid-exaile.c \
	cid-exaile.h \
	cid-main.c \
	cid-main.h \
	cid-menu-factory.c \
	cid-menu-factory.h \
	cid-messages.c \
	cid-messages.h \
	cid-rhythmbox.c \
	cid-rhythmbox.h \
	cid-utilities.c \
	cid-utilities.h 
	
	
	
	
	



cidincludedir =\
	$(includedir)/cid

cidinclude_HEADERS =\
	cid.h

cidsubincludedir =\
	$(includedir)/cid/cid

cidsubinclude_HEADERS = \
	cid-amarok.h \
	cid-amarok2.h \
	cid-amazon.h \
	cid-animation.h \
	cid-callbacks.h \
	cid-conf-panel-factory.h \
	cid-config.h \
	cid-dbus.h \
	cid-exaile.h \
	cid-main.h \
	cid-menu-factory.h \
	cid-messages.h \
	cid-rhythmbox.h \
	cid-utilities.h 

