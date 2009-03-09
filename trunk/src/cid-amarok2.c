/*
   *
   *                    cid-amarok2.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
//#include "cid.h"
#include "cid-amarok2.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"

extern CidMainContainer *cid;

static DBusGProxy *dbus_proxy_player = NULL;
static GHashTable *change_song = NULL;

gchar *cid_amarok_2_cover() {
	if (dbus_detect_amarok_2()) {
		if (amarok_2_getPlaying ()) {
			//amarok_2_getPlayingUri();
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
			dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID),
			G_TYPE_INVALID);
		//dbus_g_proxy_add_signal(dbus_proxy_player, "CapsChange",
		//	G_TYPE_UINT,
		//	G_TYPE_INVALID);
		
		// Puis on connecte les signaux a l'application pour les traiter un par un
		dbus_g_proxy_connect_signal(dbus_proxy_player, "TrackChange",
			G_CALLBACK(am_onChangeSong), NULL, NULL);
			
		dbus_g_proxy_connect_signal(dbus_proxy_player, "StatusChange",
			G_CALLBACK(am_onChangeState), NULL, NULL);
		
		//dbus_g_proxy_connect_signal(dbus_proxy_player, "CapsChange",
		//	G_CALLBACK(am_onCovertArtChanged), NULL, NULL);
		
		return TRUE;
	}
	return FALSE;
}

void amarok_2_dbus_disconnect_from_bus (void) {
	if (dbus_proxy_player != NULL) {
		// On se desabonne de tous les signaux
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "TrackChange",
			G_CALLBACK(am_onChangeSong), NULL);
		cid_debug ("TrackChange deconnecte\n");
		
		dbus_g_proxy_disconnect_signal(dbus_proxy_player, "StatusChange",
			G_CALLBACK(am_onChangeState), NULL);
		cid_debug ("StatusChange deconnecte\n");
		
		//dbus_g_proxy_disconnect_signal(dbus_proxy_player, "CapsChange",
		//	G_CALLBACK(am_onCovertArtChanged), NULL);
		//cid_debug ("CapsChange deconnecte\n");
		
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
// amarok_2_getPlaying() : Test si amarok2 joue de la musique ou non
//*********************************************************************************
gboolean amarok_2_getPlaying (void) {
	GValueArray *s = 0;
	GValue *v;
	gint status;
	dbus_g_proxy_call (dbus_proxy_player, "GetStatus", NULL,
	G_TYPE_INVALID,
	dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID),
	&s,
	G_TYPE_INVALID);
	
	v = g_value_array_get_nth(s, 0);
	status = g_value_get_int(v);
	
	cid_debug("Status : %i",status);
	//switch (status) {
	//	case 0:
			/// PLAY
	//		break;
	//	case 1:
			/// PAUSE
	//		break;
	//	case 2: 
			/// STOP
	//		break;
	//	default:
	//		break;
	//}
	musicData.playing = FALSE;
	if (status == 0)
		musicData.playing = TRUE;
	return musicData.playing;
}


//*********************************************************************************
// amarok2_getPlayingUri() : Retourne l'adresse de la musique jouée
//*********************************************************************************
gchar *amarok_2_getPlayingUri(void) {
	///\______ je vois pas ou recuperer cette info...
	g_free (musicData.playing_uri);
	musicData.playing_uri = NULL;
	
	
	return musicData.playing_uri;
}


void am_getSongInfos(void) {	
	GHashTable *data_list = NULL;
	GValue *value;

	if(dbus_g_proxy_call (dbus_proxy_player, "GetMetadata", NULL,
							G_TYPE_INVALID,
							dbus_g_type_get_map("GHashTable",G_TYPE_STRING, G_TYPE_VALUE),
							&data_list,
							G_TYPE_INVALID)) {
	
		// Tester si la table de hachage n'est pas vide
		g_free (musicData.playing_artist);
		value = (GValue *) g_hash_table_lookup(data_list, "artist");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_artist = g_strdup (g_value_get_string(value));
		else musicData.playing_artist = NULL;
		cid_message ("playing_artist <- %s", musicData.playing_artist);
		
		g_free (musicData.playing_album);
		value = (GValue *) g_hash_table_lookup(data_list, "album");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_album = g_strdup (g_value_get_string(value));
		else musicData.playing_album = NULL;
		cid_message ("playing_album <- %s", musicData.playing_album);
		
		g_free (musicData.playing_title);
		value = (GValue *) g_hash_table_lookup(data_list, "title");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_title = g_strdup (g_value_get_string(value));
		else musicData.playing_title = NULL;
		cid_message ("playing_title <- %s", musicData.playing_title);
		
		value = (GValue *) g_hash_table_lookup(data_list, "tracknumber");
		if (value != NULL && G_VALUE_HOLDS_UINT(value)) musicData.playing_track = g_value_get_uint(value);
		else musicData.playing_track = 0;
		cid_message ("playing_track <- %d", musicData.playing_track);
		
		value = (GValue *) g_hash_table_lookup(data_list, "time");
		if (value != NULL && G_VALUE_HOLDS_INT(value)) musicData.playing_duration = (g_value_get_int(value));
		else musicData.playing_duration = 0;
		cid_message ("playing_duration <- %ds", musicData.playing_duration);
		
		g_free (musicData.playing_cover);
		value = (GValue *) g_hash_table_lookup(data_list, "arturl");
		if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_cover = g_strdup (g_value_get_string(value));
		else musicData.playing_cover = NULL;
		cid_message ("playing_cover <- %s", musicData.playing_cover);
		
		g_hash_table_destroy (data_list);
	} else {
		cid_warning ("  can't get song properties");
		g_free (musicData.playing_uri);
		musicData.playing_uri = NULL;
		g_free (musicData.playing_cover);
		musicData.playing_cover = NULL;
	}
}


//*********************************************************************************
// am_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void am_onChangeSong(DBusGProxy *player_proxy,GHashTable *data_list, gpointer data) {
	/*
	GValue *value;
	
	// Tester si la table de hachage n'est pas vide
	g_free (musicData.playing_artist);
	value = (GValue *) g_hash_table_lookup(data_list, "artist");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_artist = g_strdup (g_value_get_string(value));
	else musicData.playing_artist = NULL;
	cid_message ("playing_artist <- %s", musicData.playing_artist);
	
	g_free (musicData.playing_album);
	value = (GValue *) g_hash_table_lookup(data_list, "album");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_album = g_strdup (g_value_get_string(value));
	else musicData.playing_album = NULL;
	cid_message ("playing_album <- %s", musicData.playing_album);
	
	g_free (musicData.playing_title);
	value = (GValue *) g_hash_table_lookup(data_list, "title");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_title = g_strdup (g_value_get_string(value));
	else musicData.playing_title = NULL;
	cid_message ("playing_title <- %s", musicData.playing_title);
	
	value = (GValue *) g_hash_table_lookup(data_list, "tracknumber");
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) musicData.playing_track = g_value_get_uint(value);
	else musicData.playing_track = 0;
	cid_message ("playing_track <- %d", musicData.playing_track);
	
	value = (GValue *) g_hash_table_lookup(data_list, "time");
	if (value != NULL && G_VALUE_HOLDS_INT(value)) musicData.playing_duration = (g_value_get_int(value));
	else musicData.playing_duration = 0;
	cid_message ("playing_duration <- %ds", musicData.playing_duration);
	
	g_free (musicData.playing_cover);
	value = (GValue *) g_hash_table_lookup(data_list, "arturl");
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) musicData.playing_cover = g_strdup (g_value_get_string(value));
	else musicData.playing_cover = NULL;
	cid_message ("playing_cover <- %s", musicData.playing_cover);
	*/
	cid_display_image(cid_amarok_2_cover());
}

//*********************************************************************************
// am_onChangeState() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void am_onChangeState(DBusGProxy *player_proxy, GValueArray *status, gpointer data) {
	amarok_2_getPlaying();
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
