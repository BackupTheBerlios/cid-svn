/*
   *
   *                    cid-rhythmbox.c
   *                       -------
   *                 Conky Images Display
   *                     SANS Benjamin
   *             ----------------------------
   *
   *
*/
#include "cid.h"

static DBusGProxy *dbus_proxy_player = NULL;
static GHashTable *change_song = NULL;

gchar *cid_amarok_2_cover() {
	if (dbus_detect_amarok_2()) {
		if (amarok_2_getPlaying ()) {
			amarok_2_getPlayingUri();
			am_getSongInfos();
		    if (musicData.playing_cover != NULL) 
		    	cid_set_state_icon();
				return musicData.playing_cover;
			return DEFAULT_IMAGE;
		} else {
			return DEFAULT_IMAGE;
		}
	} else {
		return DEFAULT_IMAGE;
	}
}

gboolean amarok_2_dbus_connect_to_bus (void) {
	g_type_init ();
	if (dbus_is_enabled ()) {
		// On se connecte au bus org.kde.amarok /Player org.freedesktop.MediaPlayer
		dbus_proxy_player = create_new_session_proxy (
			"org.kde.amarok",
			"/Player",
			"org.freedesktop.MediaPlayer"
		);
		
		// On s'abonne aux signaux
		dbus_g_proxy_add_signal(dbus_proxy_player, "TrackChange",
			dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "StatusChange",
			G_TYPE_UINT,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_player, "CapsChange",
			G_TYPE_UINT,
			G_TYPE_INVALID);
		
		// Puis on connecte les signaux a l'application pour les traiter un par un
		dbus_g_proxy_connect_signal(dbus_proxy_player, "TrackChange",
			G_CALLBACK(am_onChangeState), NULL, NULL);
			
		dbus_g_proxy_connect_signal(dbus_proxy_player, "StatusChange",
			G_CALLBACK(am_onChangeSong), NULL, NULL);
		
		dbus_g_proxy_connect_signal(dbus_proxy_player, "CapsChange",
			G_CALLBACK(am_onCovertArtChanged), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

void amarok_2_dbus_disconnect_from_bus (void) {
	if (dbus_proxy_player != NULL) {
		// On se desabonne de tous les signaux
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "TrackChange",
			G_CALLBACK(am_onChangeState), NULL);
		cid_debug ("TrackChange deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "StatusChange",
			G_CALLBACK(am_onChangeSong), NULL);
		cid_debug ("StatusChange deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "CapsChange",
			G_CALLBACK(am_onCovertArtChanged), NULL);
		cid_debug ("CapsChange deconnecte\n");
		
		g_object_unref (dbus_proxy_player);
		dbus_proxy_player = NULL;
	}
}

gboolean dbus_detect_amarok_2(void) {
	// On verifie qu'on trouve bien amarok dans la liste des bus
	musicData.opening = dbus_detect_application ("org.kde.amarok");
	return musicData.opening;
}


//*********************************************************************************
// rhythmbox_getPlaying() : Test si Rhythmbox joue de la musique ou non
//*********************************************************************************
gboolean amarok_2_getPlaying (void) {
	///\______ ICI ON DOIT RECUPERER LE STATUS ET DONNER UNE VALEUR AU BOOLEAN 'playing'
	///        j'arrive pas a determiner le type (iii)... tableau ?
	//gint status = dbus_get_uinteger(dbus_proxy_player, "GetStatus");
	g_print ("%d\n",status);
	return musicData.playing;
}


//*********************************************************************************
// rhythmbox_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
gchar *amarok_2_getPlayingUri(void) {
	///\______ ICI JE PENSE QU'ON DOIT SE SERVIR DU 'GetMetadata'
	///        faut juste savoir comment se compose la GHashTable obtenue
	g_free (musicData.playing_uri);
	musicData.playing_uri = NULL;
	
	
	return musicData.playing_uri;
}


void am_getSongInfos(void) {	
	GHashTable *data_list = NULL;
	GValue *value;
	const gchar *data;
	///\_____ PAREIL QU'AU DESSUS, ON DOIT RECUPERER LES DONNEES CONTENUES DANS 'GetMetadata'
	///       mais j'ai aucune idee de ce a quoi ca ressemble
/*	
	if(dbus_g_proxy_call (dbus_proxy_player, "GetMetadata", NULL,
							G_TYPE_STRING, musicData.playing_uri,
							G_TYPE_INVALID,
							dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
							&data_list,
							G_TYPE_INVALID)) {
		g_free (musicData.playing_artist);
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_artist = g_strdup (g_value_get_string(value));
		else musicData.playing_artist = NULL;
		cid_message ("  playing_artist <- %s\n", musicData.playing_artist);
		
		g_free (musicData.playing_album);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_album = g_strdup (g_value_get_string(value));
		else musicData.playing_album = NULL;
		cid_message ("  playing_album <- %s\n", musicData.playing_album);
		
		g_free (musicData.playing_title);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_title = g_strdup (g_value_get_string(value));
		else musicData.playing_title = NULL;
		cid_message ("  playing_title <- %s\n", musicData.playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "track-number");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) musicData.playing_track = g_value_get_uint(value);
		else musicData.playing_track = 0;
		cid_message ("  playing_track <- %d\n", musicData.playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "duration");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) musicData.playing_duration = g_value_get_uint(value);
		else musicData.playing_duration = 0;
		cid_message ("  playing_duration <- %ds\n", musicData.playing_duration);
		
		value = (GValue *) g_hash_table_lookup(data_list, "rb:coverArt-uri");
		g_free (musicData.playing_cover);
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
			GError *erreur = NULL;
			const gchar *cString = g_value_get_string(value);
			if (cString != NULL && strncmp (cString, "file://", 7) == 0) {
				musicData.playing_cover = g_filename_from_uri (cString, NULL, &erreur);
				if (erreur != NULL) {
					cid_warning ("Attention : %s\n", erreur->message);
					g_error_free (erreur);
				}
			} else {
				musicData.playing_cover = g_strdup (cString);
			}
		} else {
			gchar *tabFiles[]={"cover", "album", "albumart", ".folder", "folder", "Cover", "Folder"};
			gchar *cSongPath = g_filename_from_uri (musicData.playing_uri, NULL, NULL);  // on teste d'abord dans le repertoire de la chanson.
			int i=0;
			if (cSongPath != NULL)
			{
				gchar *cSongDir = g_path_get_dirname (cSongPath);
				g_free (cSongPath);
				musicData.playing_cover = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, musicData.playing_artist, musicData.playing_album);
				cid_debug ("   test de %s\n", musicData.playing_cover);
				while (! g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS) && tabFiles[i]!=NULL && i<7)
				{
					g_free (musicData.playing_cover);
					musicData.playing_cover = g_strdup_printf ("%s/%s.jpg", cSongDir, tabFiles[i]);
					cid_debug ("   test de %s (%d)\n", musicData.playing_cover,i);
					i++;
				}
				if (! g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS))
				{
					cid_debug ("   test de %s (if .gnome2)\n", musicData.playing_cover);
					g_free (musicData.playing_cover);
					musicData.playing_cover = g_strdup_printf("%s/.gnome2/rhythmbox/covers/%s - %s.jpg", g_getenv("HOME"),musicData.playing_artist, musicData.playing_album);
				}
				g_free (cSongDir);
			}
		}
		cid_message ("  playing_cover <- %s\n", musicData.playing_cover);
		
		g_hash_table_destroy (data_list);
	} else {
		cid_error ("  can't get song properties");
		g_free (musicData.playing_uri);
		musicData.playing_uri = NULL;
		g_free (musicData.playing_cover);
		musicData.playing_cover = NULL;
	}
*/
}


//*********************************************************************************
// am_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void am_onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data) {
	cid_message ("-> %s (%s)\n",__func__,uri);
	
	g_free (musicData.playing_uri);
	if(uri != NULL && *uri != '\0') {
		musicData.playing_uri = g_strdup (uri);
		musicData.opening = TRUE;
		getSongInfos();
		cid_display_image(cid_amarok_2_cover());
		cid_animation(cid->iAnimationType);
	} else {
		musicData.playing_uri = NULL;
		musicData.cover_exist = FALSE;
		
		g_free (musicData.playing_artist);
		musicData.playing_artist = NULL;
		g_free (musicData.playing_album);
		musicData.playing_album = NULL;
		g_free (musicData.playing_title);
		musicData.playing_title = NULL;
		g_free (musicData.playing_cover);
		musicData.playing_cover = NULL;
		musicData.playing_duration = 0;
		musicData.playing_track = 0;
		
		cid_display_image(cid_amarok_2_cover());
		cid_animation(cid->iAnimationType);

		dbus_detect_amarok_2();
	}
}

//*********************************************************************************
// am_onChangeState() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void am_onChangeState(DBusGProxy *player_proxy, gboolean a_playing, gpointer data) {
	cid_message ("-> %s () : %s\n",__func__,a_playing ? "PLAY" : "PAUSE");
	musicData.playing = a_playing;
	if(! musicData.cover_exist && musicData.playing_uri != NULL) {
//		g_printerr ("  playing_uri : %s\n", musicData.playing_uri);
		if(musicData.playing) {
			cid->bCurrentlyPlaying = TRUE;
		//	rhythmbox_set_surface (PLAYER_PLAYING);
		} else {
			cid->bCurrentlyPlaying = FALSE;
		//	rhythmbox_set_surface (PLAYER_PAUSED);
		}
	}
	cid_set_state_icon();
}

void am_onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data) {
	cid_debug ("%s (%s)",__func__,cImageURI);
	g_free (musicData.playing_cover);
	musicData.playing_cover = g_strdup (cImageURI);
	cid_display_image(musicData.playing_cover);
}

void _playPause_amarok_2 (void) {
	dbus_call (dbus_proxy_player,musicData.playing ? "Pause" : "Play");
}

void _next_amarok_2 (void) {
	dbus_call (dbus_proxy_player,"Next");
}

void _previous_amarok_2 (void) {
	dbus_call (dbus_proxy_player,"Prev");
}

void cid_build_amarok_2_menu (void) {
	cid->pMonitorList->p_fPlayPause = (CidControlFunction) _playPause_amarok_2;
	cid->pMonitorList->p_fNext = (CidControlFunction) _next_amarok_2;
	cid->pMonitorList->p_fPrevious = (CidControlFunction) _previous_amarok_2;
}
