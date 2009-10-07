#dnl Process this file with automake to produce Makefile.in

if WITH_OLD_GNOME_SUPPORT
gnome_old_dir=gnome-integration-old
endif

if WITH_WEBLETS
weblets_dir=weblets
endif

SUBDIRS = . po\
	${gnome_old_dir}\
	${weblets_dir}

EXTRA_DIST = \
	intltool-extract.in\
	intltool-merge.in\
	intltool-update.in\
	LICENSE

