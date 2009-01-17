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
		cid_warning ("Couldn't reach amarok");
		pclose (status);
		return FALSE;
	}
	if (!fgets (gStatus,128,status)) {
		cid_warning ("Couldn't get status");
		musicData.opening = FALSE;
		cid_set_state_icon();
		pclose (status);
		if (run) {
			cid_display_image(DEFAULT_IMAGE);
			run = FALSE;
		}
		return FALSE;
	}
	
	gint state = atoi(gStatus);
	if (state == 1)
		musicData.playing = FALSE;
	else if (state == 2)
		musicData.playing = TRUE;
		
	cid_set_state_icon();
	
	
	FILE *artist = popen ("dcop amarok player artist","r"), 
		 *album = popen ("dcop amarok player album","r"),
	 	 *title = popen ("dcop amarok player title","r"),
	 	 *uri = popen ("dcop amarok player encodedURL","r"),
	 	 *coverURI = popen ("dcop amarok player coverImage","r");
	gchar gArtist[128], gAlbum[128], gTitle[128], gPlayingURI[256], gCoverURI[256];
	if (!artist || !album || !title || !uri || !coverURI) {
		pclose (artist);
		pclose (album);
		pclose (title);
		pclose (uri);
		pclose (coverURI);
		cid_warning ("Couldn't get data");
		return FALSE;
	}
	
	
	if (!fgets (gArtist,128,artist) || !fgets (gAlbum,128,album) || !fgets (gTitle,128,title) || !fgets (gPlayingURI,256,uri)) {
		pclose (artist);
		pclose (album);
		pclose (title);
		pclose (uri);
		pclose (coverURI);
		return FALSE;
	}
	
	g_free (musicData.playing_cover);
	if (!fgets (gCoverURI,256,coverURI))
		musicData.playing_cover = g_strdup (DEFAULT_IMAGE);
	else {
		strtok (gCoverURI,"\n");
		musicData.playing_cover = g_strdup (gCoverURI);
	}
	
	pclose (artist);
	pclose (album);
	pclose (title);
	pclose (uri);
	pclose (coverURI);

	strtok (gArtist,"\n");
	strtok (gAlbum,"\n");
	strtok (gTitle,"\n");
	strtok (gPlayingURI,"\n");
	strtok (gStatus,"\n");
	
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

gboolean cid_download_amarok_cover (gpointer data) {
	cid->iCheckIter++;
	if (cid->iCheckIter > cid->iTimeToWait) {
		//GError *erreur = NULL;
		//GThread* pThread = g_thread_create ((GThreadFunc) _cid_proceed_download_cover, NULL, FALSE, &erreur);
		//if (erreur != NULL)	{
		//	cid_warning ("couldn't launch this command (%s)", erreur->message);
		//	g_error_free (erreur);
		//	return FALSE;
		//}
		g_timeout_add (0,(GSourceFunc) _cid_proceed_download_cover, NULL);
		return FALSE;
	}
	return TRUE;
}

gchar *cid_check_amarok_cover_exists (gchar *cURI) {
	gchar **cCleanURI = g_strsplit (cURI,"@",0);
	gchar **cSplitedURI = g_strsplit (cCleanURI[1],".",0);
	if (g_strcasecmp(cSplitedURI[0],"nocover")==0) {
		g_free (cCleanURI);
		g_free (cSplitedURI);
		cid->iCheckIter = 0;
		g_timeout_add (1000, (GSourceFunc) cid_download_amarok_cover, (gpointer) NULL);
		return g_strdup(DEFAULT_IMAGE);
	}
	g_free (cCleanURI);
	g_free (cSplitedURI);
	return cURI;
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
				cid_display_image (cid_check_amarok_cover_exists(musicData.playing_cover));
			else
				cid_display_image (DEFAULT_IMAGE);
			cid_animation(cid->iAnimationType);
		} 
	}
		
	return cont;
}

void cid_amarok_pipe (gint iInter) {
	g_timeout_add_full (G_PRIORITY_HIGH, iInter,(gpointer) cid_amarok_cover, NULL, NULL);
}

void cid_disconnect_from_amarok () {
	cont = FALSE;
}

void cid_connect_to_amarok(gint iInter) {
	cont = TRUE;
	cid_display_image(DEFAULT_IMAGE);
	cid_amarok_pipe (iInter);	
}

void _playPause_amarok (void) {
	if (!system ("dcop amarok player playPause")) return;
}

void _next_amarok (void) {
	if (!system ("dcop amarok player next")) return;
}

void _previous_amarok (void) {
	if (!system ("dcop amarok player prev")) return;
}

void cid_build_amarok_menu (void) {
	cid->pMonitorList->p_fPlayPause = (CidControlFunction) _playPause_amarok;
	cid->pMonitorList->p_fNext = (CidControlFunction) _next_amarok;
	cid->pMonitorList->p_fPrevious = (CidControlFunction) _previous_amarok;
}
