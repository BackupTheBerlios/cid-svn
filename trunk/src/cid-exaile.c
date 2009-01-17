/*
   *
   *                     cid-exaile.c
   *                       -------
   *                 Conky Images Display
   *              17/01/2009 - SANS Benjamin
   *             ----------------------------
   *
   *
*/
#include "cid.h"

static DBusGProxy *dbus_proxy_player = NULL;

gboolean cont;
gboolean bSongChanged;
gboolean bPreviousState;
gboolean bFirstLoop;

gboolean cid_exaile_cover() {
	if (dbus_detect_exaile()) {
		if (exaile_getPlaying ()) {
			getExaileSongInfos();
			if (!bSongChanged)
				return cont;
		    if (musicData.playing_cover != NULL) 
				cid_display_image (musicData.playing_cover);
			else
				cid_display_image (DEFAULT_IMAGE);
		} else {
			cid_display_image (DEFAULT_IMAGE);
		}
	} else {
		cid_display_image (DEFAULT_IMAGE);
	}
	return cont;
}

gboolean exaile_dbus_connect_to_bus (void) {
	g_type_init ();
	if (dbus_is_enabled ()) {
		
		dbus_proxy_player = create_new_session_proxy (
			"org.exaile.DBusInterface",
			"/DBusInterfaceObject",
			"org.exaile.DBusInterface"
		);
		
		return TRUE;
	}
	return FALSE;
}

void exaile_dbus_disconnect_from_bus (void) {
	
	if (dbus_proxy_player != NULL) {
		g_object_unref (dbus_proxy_player);
		dbus_proxy_player = NULL;
	}
}

gboolean dbus_detect_exaile(void) {
	musicData.opening = dbus_detect_application ("org.exaile.DBusInterface");
	return musicData.opening;
}


//*********************************************************************************
// exaile_getPlaying() : Test si exaile joue de la musique ou non
//*********************************************************************************
gboolean exaile_getPlaying (void) {
	gchar *status = dbus_get_string (dbus_proxy_player, "query");
	gchar **cSplitedQuery = g_strsplit (status," ",0);
	bPreviousState = musicData.playing;
	if (! g_ascii_strcasecmp(cSplitedQuery[1], "playing")) 
		musicData.playing = TRUE;
	else
		musicData.playing = FALSE;
	if (bPreviousState != musicData.playing)
		cid_set_state_icon();
	g_free (cSplitedQuery);
	g_free (status);
	return musicData.playing;
}

gchar *cid_check_exaile_cover_exists (gchar *cURI) {
	gint cpt=0;
	gchar **cCleanURI = g_strsplit (cURI,"/",0);
	while (cCleanURI[cpt]!=NULL) 
		cpt++;
	gchar **cSplitedURI = g_strsplit (cCleanURI[cpt-1],".",0);
	if (g_strcasecmp(cSplitedURI[0],"nocover")==0) {
		g_free (cCleanURI);
		g_free (cSplitedURI);
		
		return g_strdup(DEFAULT_IMAGE);
	}
	g_free (cCleanURI);
	g_free (cSplitedURI);
	return cURI;
}

void getExaileSongInfos(void) {	
	gchar *cOldArtist = musicData.playing_artist;
	gchar *cOldTitle  = musicData.playing_title;
	musicData.playing_album  = dbus_get_string (dbus_proxy_player, "get_album");
	musicData.playing_artist = dbus_get_string (dbus_proxy_player, "get_artist");
	musicData.playing_title  = dbus_get_string (dbus_proxy_player, "get_title");
	musicData.playing_duration = atoi (dbus_get_string (dbus_proxy_player, "get_length"));
	musicData.playing_cover  = cid_check_exaile_cover_exists(dbus_get_string (dbus_proxy_player, "get_cover_path"));
	
	musicData.playing_uri = NULL;
	musicData.playing_track = 0;
	
	if ((bFirstLoop && cOldArtist==NULL && cOldTitle==NULL) || (g_strcasecmp(cOldArtist,musicData.playing_artist)!=0 && g_strcasecmp(cOldTitle,musicData.playing_title)!=0))
		bSongChanged = TRUE;
	else
		bSongChanged = FALSE;
		
	g_free (cOldArtist);
	g_free (cOldTitle);
	bFirstLoop = FALSE;
}

void cid_exaile_pipe (gint iInter) {
	g_timeout_add_full (G_PRIORITY_HIGH, iInter,(gpointer) cid_exaile_cover, NULL, NULL);
}

void cid_disconnect_from_exaile () {
	cont = FALSE;
	exaile_dbus_disconnect_from_bus();
}

void cid_connect_to_exaile(gint iInter) {
	cont = TRUE;
	bFirstLoop = TRUE;
	exaile_dbus_connect_to_bus ();
	cid_exaile_cover();
	cid_exaile_pipe (iInter);	
}

void _playPause_exaile (void) {
	dbus_call (dbus_proxy_player,"play_pause");
}

void _next_exaile (void) {
	dbus_call (dbus_proxy_player,"next_track");
}

void _previous_exaile (void) {
	dbus_call (dbus_proxy_player,"prev_track");
}

void cid_build_exaile_menu (void) {
	cid->pMonitorList->p_fPlayPause = (CidControlFunction) _playPause_exaile;
	cid->pMonitorList->p_fNext = (CidControlFunction) _next_exaile;
	cid->pMonitorList->p_fPrevious = (CidControlFunction) _previous_exaile;
}
