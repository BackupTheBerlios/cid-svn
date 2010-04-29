/*
   *
   *                              amarok.c
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/

//#include "cid.h"
#include "cid-amarok.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-callbacks.h"
#include "cid-console-call.h"
#include "cid-constantes.h"
#include "cid-asynchrone.h"

extern CidMainContainer *cid;

static gboolean cont;
gboolean run = FALSE;
static CidMeasure *pMeasureTimer = NULL;
extern gboolean bCurrentlyDownloading, bCurrentlyDownloadingXML;

///dcop amarok playlist addMediaList [ "file:///home/benjamin/Music/aboutagirl.mp3" ]

gboolean 
get_amarock_musicData () 
{
    CIDError *error = NULL;
    gint state = cid_console_get_int_with_error_full("dcop amarok player status",-1,&error);
    
    musicData.opening = TRUE;
    
    if (error) 
    {
        cid_warning (error->message);
        
        musicData.opening = FALSE;
        musicData.playing = FALSE;
    
        if (run) 
        {
            cid_display_image(DEFAULT_IMAGE);
            cid_set_state_icon();
            run = FALSE;
        }
        
        cid_free_error (error);
        return FALSE;
    }
    
    gboolean bOldState = musicData.playing;
    
    if (state == 1)
        musicData.playing = FALSE;
    else if (state == 2)
        musicData.playing = TRUE;
        
    if (bOldState!=musicData.playing)
        cid_set_state_icon();
    
    
    gchar *gArtist, *gAlbum, *gTitle, *gPlayingURI, *gCoverURI;
    
    
    gArtist = cid_console_get_string ("dcop amarok player artist");
    gAlbum = cid_console_get_string ("dcop amarok player album");
    gTitle = cid_console_get_string ("dcop amarok player title");
    gPlayingURI = cid_console_get_string ("dcop amarok player encodedURL");
    gCoverURI = cid_console_get_string ("dcop amarok player coverImage");
    
    g_free (musicData.playing_cover);
    if (gCoverURI == NULL)
        musicData.playing_cover = g_strdup (DEFAULT_IMAGE);
    else 
        musicData.playing_cover = g_strdup (gCoverURI);
    
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
    
    
    run = TRUE;
    cid_info ("\nartist : %s\nalbum : %s\ntitle : %s\nsong uri : %s\ncover uri : %s\n",gArtist,gAlbum,gTitle,gPlayingURI,gCoverURI);
    return TRUE;
}

gboolean 
cid_download_amarok_cover (gpointer data) 
{
    cid->iCheckIter++;
    if (cid->iCheckIter > cid->iTimeToWait) 
    {
        if (pMeasureTimer) 
        {
            if (cid_measure_is_running(pMeasureTimer))
                cid_stop_measure_timer(pMeasureTimer);
            if (cid_measure_is_active(pMeasureTimer))
                cid_free_measure_timer(pMeasureTimer);
        }
        pMeasureTimer = cid_new_measure_timer (2 SECONDES, NULL, NULL, (CidUpdateTimerFunc) _cid_proceed_download_cover, NULL);
        
        cid_launch_measure (pMeasureTimer);
        return FALSE;
    }
    return TRUE;
}

gchar *
cid_check_amarok_cover_exists (gchar *cURI) 
{
    gchar **cCleanURI = g_strsplit (cURI,"@",0);
    gchar **cSplitedURI = g_strsplit (cCleanURI[1],".",0);
    if (g_strcasecmp(cSplitedURI[0],"nocover")==0) 
    {
        g_free (cCleanURI);
        g_free (cSplitedURI);
        cid->iCheckIter = 0;
        g_timeout_add (1000, (GSourceFunc) cid_download_amarok_cover, (gpointer) NULL);
        return g_strdup(DEFAULT_IMAGE);
    }
    g_strfreev (cCleanURI);
    g_strfreev (cSplitedURI);
    return cURI;
}

gboolean 
cid_amarok_cover() 
{
    /* On vérifie l'état d'Amarok */
    if (get_amarock_musicData()) 
    {
        /* Si Amarok ne joue pas, on affiche l'image par défaut. */
        if (!musicData.opening) 
        {
            cid_debug ("Amarok isn't playing\n");
            cid_display_image (DEFAULT_IMAGE);
        } 
        else 
        {
            if (musicData.playing && musicData.playing_cover != NULL)
                cid_display_image (cid_check_amarok_cover_exists(musicData.playing_cover));
            else
                cid_display_image (DEFAULT_IMAGE);
            cid_animation(cid->iAnimationType);
        } 
    }
        
    return cont;
}

void 
cid_amarok_pipe (gint iInter) 
{
    cid->iPipe = g_timeout_add_full (G_PRIORITY_HIGH, iInter,(gpointer) cid_amarok_cover, NULL, NULL);
}

void 
cid_disconnect_from_amarok () 
{
    cont = FALSE;
    if (cid->bPipeRunning)
        g_source_remove (cid->iPipe);
    cid->bPipeRunning = FALSE;
}

void 
cid_connect_to_amarok(gint iInter) 
{
    cont = TRUE;
    cid->bPipeRunning = TRUE;
    cid_display_image(DEFAULT_IMAGE);
    cid_amarok_pipe (iInter);   
}

void 
_playPause_amarok (void) 
{
    cid_launch_command ("dcop amarok player playPause >/dev/null 2>&1");
    cid_amarok_cover ();
}

void 
_next_amarok (void) 
{
    cid_launch_command ("dcop amarok player next >/dev/null 2>&1");
    cid_amarok_cover ();
}

void 
_previous_amarok (void) 
{
    cid_launch_command ("dcop amarok player prev >/dev/null 2>&1");
    cid_amarok_cover ();
}

void 
cid_build_amarok_menu (void) 
{
    cid->pMonitorList->p_fPlayPause = _playPause_amarok;
    cid->pMonitorList->p_fNext = _next_amarok;
    cid->pMonitorList->p_fPrevious = _previous_amarok;
    cid->pMonitorList->p_fAddToQueue = NULL;
}
