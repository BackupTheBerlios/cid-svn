/*
   *
   *                              amarok.c
   *                               -------
   *                         Conky Images Display
   *             05/10/2008 - Charlie MERLAND / Benjamin SANS
   *             --------------------------------------------
   *
*/

#include "cid.h"

gboolean cont;
gboolean run = FALSE;

gboolean get_amarock_musicData () {
	FILE *status = popen ("dcop amarok player status","r");
	gchar gStatus[128];
	musicData.opening = TRUE;
	if (!status) {
		cid_warning ("Couldn't get status");
		return FALSE;
	}
	if (!fgets (gStatus,128,status)) {
		cid_warning ("Couldn't get status");
		musicData.opening = FALSE;
		pclose (status);
		if (run) {
			cid_display_image(DEFAULT_IMAGE);
			run = FALSE;
		}
		return FALSE;
	}
	
	FILE *artist = popen ("dcop amarok player artist","r"), 
		 *album = popen ("dcop amarok player album","r"),
	 	 *title = popen ("dcop amarok player title","r"),
	 	 *uri = popen ("dcop amarok player encodedURL","r"),
	 	 *coverURI = popen ("dcop amarok player coverImage","r");
	gchar gArtist[128], gAlbum[128], gTitle[128], gPlayingURI[256], gCoverURI[256];
	if (!artist) {
		cid_warning ("Couldn't get artist name");
		return FALSE;
	}
	if (!album) {
		cid_warning ("Couldn't get album name");
		return FALSE;
	}
	if (!title) {
		cid_warning ("Couldn't get title");
		return FALSE;
	}
	if (!uri) {
		cid_warning ("Couldn't get song URI");
		return FALSE;
	}
	if (!coverURI) {
		cid_warning ("Couldn't get cover URI");
		return FALSE;
	}
	
	
	if (!fgets (gArtist,128,artist)) return FALSE;
	if (!fgets (gAlbum,128,album)) return FALSE;
	if (!fgets (gTitle,128,title)) return FALSE;
	if (!fgets (gPlayingURI,256,uri)) return FALSE;
	
	g_free (musicData.playing_cover);
	if (!fgets (gCoverURI,256,coverURI))
		musicData.playing_cover = g_strdup (DEFAULT_IMAGE);
	else {
		strtok (gCoverURI,"\n");
		musicData.playing_cover = g_strdup (gCoverURI);
	}

	strtok (gArtist,"\n");
	strtok (gAlbum,"\n");
	strtok (gTitle,"\n");
	strtok (gPlayingURI,"\n");
	strtok (gStatus,"\n");
	
	pclose (artist);
	pclose (album);
	pclose (title);
	pclose (uri);
	pclose (coverURI);
	
	if (musicData.playing_uri != NULL && strcmp (musicData.playing_uri,gPlayingURI)==0)
		return FALSE;
		
	g_free (musicData.playing_uri);
	musicData.playing_uri = g_strdup(gPlayingURI);
	g_free (musicData.playing_title);
	musicData.playing_title = g_strdup (gTitle);
	g_free (musicData.playing_album);
	musicData.playing_album = g_strdup (gAlbum);
	g_free (musicData.playing_artist);
	musicData.playing_artist = g_strdup (gArtist);
	
	
	
	if (gStatus != NULL && (strcmp (gStatus,"1")==0 || strcmp (gStatus,"2")==0))
		musicData.playing = TRUE;
	else
		musicData.playing = FALSE;
	
	run = TRUE;
	cid_info ("\nartist : %s\nalbum : %s\ntitle : %s\nsong uri : %s\ncover uri : %s\n",gArtist,gAlbum,gTitle,gPlayingURI,gCoverURI);
	return TRUE;
}

gboolean cid_amarok_cover() {
	/* On vérifie l'état d'Amarok */
	if (get_amarock_musicData()) {
		/* Si Amarok ne joue pas, on affiche l'image par défaut. */
		if (!musicData.opening) {
			cid_debug ("Amarok isn't playing\n");
			cid_display_image (DEFAULT_IMAGE);
		} else {
			if (musicData.playing && musicData.playing_cover != NULL)
				cid_display_image (musicData.playing_cover);
			else
				cid_display_image (DEFAULT_IMAGE);
			cid_animation(cid->iAnimationType);
		} 
	}
		
	return cont;
}

void cid_amarok_pipe (gint iInter) {
	g_timeout_add (iInter,(gpointer) cid_amarok_cover, NULL);
}

void cid_disconnect_from_amarok () {
	cont = FALSE;
}

void cid_connect_to_amarok(gint iInter) {
	cont = TRUE;
	cid_display_image(DEFAULT_IMAGE);
	cid_amarok_pipe (iInter);	
}

void _playPause_amarok () {
	if (!system ("dcop amarok player playPause")) return;
}

void _next_amarok () {
	if (!system ("dcop amarok player next")) return;
}

void _previous_amarok () {
	if (!system ("dcop amarok player prev")) return;
}

void cid_build_amarok_menu (GtkWidget *pMenuItem, GtkWidget *menu) {
	static gpointer *data = NULL;
	GtkWidget *image;
	pMenuItem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
	
	_add_entry_in_menu (_("Play/Pause"), NULL, _playPause_amarok, menu);
	_add_entry_in_menu (_("Next"), NULL, _next_amarok, menu);
	_add_entry_in_menu (_("Previous"), NULL, _previous_amarok, menu);
}
