/*
   *
   *                    cid-rhythmbox.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
   *    Source originale: applet rhythmbox pour Cairo-dock
   *    Auteur origial: Adrien Pilleboue
*/
//#include "cid.h"
#include "cid-rhythmbox.h"
#include "cid-dbus.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-struct.h"

extern CidMainContainer *cid;

static DBusGProxy *dbus_proxy_player = NULL;
static DBusGProxy *dbus_proxy_shell = NULL;

gchar *cid_rhythmbox_cover() {
	if (dbus_detect_rhythmbox()) {
		if (rhythmbox_getPlaying ()) {
			rhythmbox_getPlayingUri();
			getSongInfos();
		    if (rhythmboxData.playing_cover != NULL) 
		    	cid_set_state_icon();
				return rhythmboxData.playing_cover;
			return DEFAULT_IMAGE;
		} else {
			return DEFAULT_IMAGE;
		}
	} else {
		return DEFAULT_IMAGE;
	}
}

gboolean rhythmbox_dbus_connect_to_bus (void) {
	g_type_init ();
	if (dbus_is_enabled ()) {
		dbus_proxy_player = create_new_session_proxy (
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Player",
			"org.gnome.Rhythmbox.Player"
		);
		
		dbus_proxy_shell = create_new_session_proxy (
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Shell",
			"org.gnome.Rhythmbox.Shell"
		);
		
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingChanged",
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "playingUriChanged",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "elapsedChanged",
			G_TYPE_UINT,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(rb_onChangeState), NULL, NULL);
			
		dbus_g_proxy_connect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(rb_onChangeSong), NULL, NULL);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(rb_onElapsedChanged), NULL, NULL);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(rb_onCovertArtChanged), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

void rhythmbox_dbus_disconnect_from_bus (void) {
	if (dbus_proxy_player != NULL) {
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingChanged",
			G_CALLBACK(rb_onChangeState), NULL);
		cid_debug ("playingChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "playingUriChanged",
			G_CALLBACK(rb_onChangeSong), NULL);
		cid_debug ("playingUriChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "elapsedChanged",
			G_CALLBACK(rb_onElapsedChanged), NULL);
		cid_debug ("elapsedChanged deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "rb:CovertArt-uri",
			G_CALLBACK(rb_onCovertArtChanged), NULL);
		cid_debug ("onCovertArtChanged deconnecte\n");
		
		g_object_unref (dbus_proxy_player);
		dbus_proxy_player = NULL;
	}
	if (dbus_proxy_shell != NULL) {
		g_object_unref (dbus_proxy_shell);
		dbus_proxy_shell = NULL;
	}
}

gboolean dbus_detect_rhythmbox(void) {
	rhythmboxData.opening = dbus_detect_application ("org.gnome.Rhythmbox");
	return rhythmboxData.opening;
}


//*********************************************************************************
// rhythmbox_getPlaying() : Test si Rhythmbox joue de la musique ou non
//*********************************************************************************
gboolean rhythmbox_getPlaying (void) {
	rhythmboxData.playing = dbus_get_boolean (dbus_proxy_player, "getPlaying");
	return rhythmboxData.playing;
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
gchar *rhythmbox_getPlayingUri(void) {
	
	g_free (rhythmboxData.playing_uri);
	rhythmboxData.playing_uri = NULL;
	
	rhythmboxData.playing_uri = dbus_get_string (dbus_proxy_player, "getPlayingUri");
	return rhythmboxData.playing_uri;
}


void getSongInfos(void) {	
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
	
	if(dbus_g_proxy_call (dbus_proxy_shell, "getSongProperties", NULL,
							G_TYPE_STRING, rhythmboxData.playing_uri,
							G_TYPE_INVALID,
							dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
							&data_list,
							G_TYPE_INVALID)) {
		g_free (rhythmboxData.playing_artist);
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) rhythmboxData.playing_artist = g_strdup (g_value_get_string(value));
		else rhythmboxData.playing_artist = NULL;
		cid_message ("  playing_artist <- %s\n", rhythmboxData.playing_artist);
		
		g_free (rhythmboxData.playing_album);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) rhythmboxData.playing_album = g_strdup (g_value_get_string(value));
		else rhythmboxData.playing_album = NULL;
		cid_message ("  playing_album <- %s\n", rhythmboxData.playing_album);
		
		g_free (rhythmboxData.playing_title);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) rhythmboxData.playing_title = g_strdup (g_value_get_string(value));
		else rhythmboxData.playing_title = NULL;
		cid_message ("  playing_title <- %s\n", rhythmboxData.playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) rhythmboxData.playing_track = g_value_get_uint(value);
		else rhythmboxData.playing_track = 0;
		cid_message ("  playing_track <- %d\n", rhythmboxData.playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) rhythmboxData.playing_duration = g_value_get_uint(value);
		else rhythmboxData.playing_duration = 0;
		cid_message ("  playing_duration <- %ds\n", rhythmboxData.playing_duration);
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		g_free (rhythmboxData.playing_cover);
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
			GError *erreur = NULL;
			const gchar *cString = g_value_get_string(value);
			if (cString != NULL && strncmp (cString, "file://", 7) == 0) {
				rhythmboxData.playing_cover = g_filename_from_uri (cString, NULL, &erreur);
				if (erreur != NULL) {
					cid_warning ("Attention : %s\n", erreur->message);
					g_error_free (erreur);
				}
			} else {
				rhythmboxData.playing_cover = g_strdup (cString);
			}
		} else {
			gchar *tabFiles[]={"cover", "album", "albumart", ".folder", "folder", "Cover", "Folder"};
			gchar *cSongPath = g_filename_from_uri (rhythmboxData.playing_uri, NULL, NULL);  // on teste d'abord dans le repertoire de la chanson.
			int i=0;
			if (cSongPath != NULL)
			{
				gchar *cSongDir = g_path_get_dirname (cSongPath);
				g_free (cSongPath);
				rhythmboxData.playing_cover = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, rhythmboxData.playing_artist, rhythmboxData.playing_album);
				cid_debug ("   test de %s\n", rhythmboxData.playing_cover);
				while (! g_file_test (rhythmboxData.playing_cover, G_FILE_TEST_EXISTS) && tabFiles[i]!=NULL && i<7)
				{
					g_free (rhythmboxData.playing_cover);
					rhythmboxData.playing_cover = g_strdup_printf ("%s/%s.jpg", cSongDir, tabFiles[i]);
					cid_debug ("   test de %s (%d)\n", rhythmboxData.playing_cover,i);
					i++;
				}
				if (! g_file_test (rhythmboxData.playing_cover, G_FILE_TEST_EXISTS))
				{
					cid_debug ("   test de %s (if .gnome2)\n", rhythmboxData.playing_cover);
					g_free (rhythmboxData.playing_cover);
					rhythmboxData.playing_cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv("HOME"),rhythmboxData.playing_artist, rhythmboxData.playing_album);
				}
				g_free (cSongDir);
			}
		}
		cid_message ("  playing_cover <- %s\n", rhythmboxData.playing_cover);
		
		g_hash_table_destroy (data_list);
	} else {
		cid_error ("  can't get song properties");
		g_free (rhythmboxData.playing_uri);
		rhythmboxData.playing_uri = NULL;
		g_free (rhythmboxData.playing_cover);
		rhythmboxData.playing_cover = NULL;
	}
}


//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void rb_onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data) {
	cid_message ("-> %s (%s)\n",__func__,uri);
	
	g_free (rhythmboxData.playing_uri);
	if(uri != NULL && *uri != '\0') {
		rhythmboxData.playing_uri = g_strdup (uri);
		rhythmboxData.opening = TRUE;
		getSongInfos();
		cid_display_image(cid_rhythmbox_cover());
		cid_animation(cid->iAnimationType);
	} else {
		rhythmboxData.playing_uri = NULL;
		rhythmboxData.cover_exist = FALSE;
		
		g_free (rhythmboxData.playing_artist);
		rhythmboxData.playing_artist = NULL;
		g_free (rhythmboxData.playing_album);
		rhythmboxData.playing_album = NULL;
		g_free (rhythmboxData.playing_title);
		rhythmboxData.playing_title = NULL;
		g_free (rhythmboxData.playing_cover);
		rhythmboxData.playing_cover = NULL;
		rhythmboxData.playing_duration = 0;
		rhythmboxData.playing_track = 0;
		
		cid_display_image(cid_rhythmbox_cover());
		cid_animation(cid->iAnimationType);

		dbus_detect_rhythmbox();
	}
}

//*********************************************************************************
// rhythmbox_onChangeState() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void rb_onChangeState(DBusGProxy *player_proxy, gboolean a_playing, gpointer data) {
	cid_message ("-> %s () : %s\n",__func__,a_playing ? "PLAY" : "PAUSE");
	rhythmboxData.playing = a_playing;
	if(! rhythmboxData.cover_exist && rhythmboxData.playing_uri != NULL) {
//		g_printerr ("  playing_uri : %s\n", rhythmboxData.playing_uri);
		if(rhythmboxData.playing) {
			cid->bCurrentlyPlaying = TRUE;
		//	rhythmbox_set_surface (PLAYER_PLAYING);
		} else {
			cid->bCurrentlyPlaying = FALSE;
		//	rhythmbox_set_surface (PLAYER_PAUSED);
		}
	}
	cid_set_state_icon();
}

//*********************************************************************************
// rhythmbox_elapsedChanged() : Fonction executée à chaque changement de temps joué
//*********************************************************************************
void rb_onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data) {
	if(elapsed > 0) {
		//g_print ("%s () : %ds\n", __func__, elapsed);
/*
		if(myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED)
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed)
			CD_APPLET_REDRAW_MY_ICON
		}
		else if(myConfig.quickInfoType == MY_APPLET_TIME_LEFT)  // avec un '-' devant.
		{
			CD_APPLET_SET_MINUTES_SECONDES_AS_QUICK_INFO (elapsed - rhythmboxData.playing_duration)
			CD_APPLET_REDRAW_MY_ICON
		}
		else if(myConfig.quickInfoType == MY_APPLET_PERCENTAGE)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", (int) (100.*elapsed/rhythmboxData.playing_duration))
			CD_APPLET_REDRAW_MY_ICON
		}
*/
	}
}


void rb_onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data) {
	cid_debug ("%s (%s)",__func__,cImageURI);
	g_free (rhythmboxData.playing_cover);
	rhythmboxData.playing_cover = g_strdup (cImageURI);
	cid_display_image(rhythmboxData.playing_cover);
}

void _playPause_rhythmbox (void) {
	dbus_call_boolean (dbus_proxy_player,"playPause", !rhythmboxData.playing);
}

void _next_rhythmbox (void) {
	dbus_call (dbus_proxy_player,"next");
}

void _previous_rhythmbox (void) {
	dbus_call (dbus_proxy_player,"previous");
}

void cid_build_rhythmbox_menu (void) {
	cid->pMonitorList->p_fPlayPause = (CidControlFunction) _playPause_rhythmbox;
	cid->pMonitorList->p_fNext = (CidControlFunction) _next_rhythmbox;
	cid->pMonitorList->p_fPrevious = (CidControlFunction) _previous_rhythmbox;
}
