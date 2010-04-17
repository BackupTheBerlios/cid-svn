/*
   *
   *                               cid-mpd.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid-mpd.h"
#include "cid-struct.h"
#include "cid-asynchrone.h"
#include "cid-constantes.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-callbacks.h"

extern CidMainContainer *cid;

static gboolean cont;
static mpd_Connection *conn = NULL;

gboolean
cid_mpd_cover ()
{
    mpd_Status *status;
    mpd_InfoEntity *entity;
    // On ouvre une connexion s'il n'en existe pas deja une
    if (!conn)
    {
        cid_debug ("Connecting to MPD");
        conn = mpd_newConnection (cid->mpd_host,cid->mpd_port, 10);
        
        if (cid->mpd_pass != NULL && strcmp (cid->mpd_pass, "") != 0)
        {
            mpd_sendPasswordCommand (conn,cid->mpd_pass);
            mpd_finishCommand (conn); 
        }
    }
    
    // On verifie qu'il n'y a pas d'erreurs
    if (conn->error)
    {
        cid_warning ("MPD error: %s",conn->errorStr);
        mpd_closeConnection (conn);
        conn = NULL;
        musicData.opening = FALSE;
        musicData.playing = FALSE;
        cid_set_state_icon ();
        cid_display_image (DEFAULT_IMAGE);
        return cont;
    }
    
    mpd_sendStatusCommand (conn);
    
    if ((status = mpd_getStatus (conn)) == NULL)
    {
        cid_warning ("MPD error: %s",conn->errorStr);
        mpd_closeConnection(conn);
        conn = NULL;
        musicData.opening = FALSE;
        musicData.playing = FALSE;
        cid_set_state_icon ();
        cid_display_image (DEFAULT_IMAGE);
        return cont;
    }
    
    mpd_finishCommand (conn);
    
    if (conn->error)
    {
        mpd_closeConnection (conn);
        conn = NULL;
        return cont;
    }
    
    musicData.opening = TRUE;
    
    gboolean bOldState = musicData.playing;
    musicData.playing = (status->state == MPD_STATUS_STATE_PLAY);
    
    mpd_freeStatus(status);
    
    if (bOldState!=musicData.playing)
        cid_set_state_icon();
    
    if (conn->error)
    {
        mpd_closeConnection (conn);
        conn = NULL;
        return cont;
    }
    
    mpd_sendCurrentSongCommand (conn);
    while ((entity = mpd_getNextInfoEntity(conn)))
    {
        mpd_Song *song = entity->info.song;

        if (entity->type != MPD_INFO_ENTITY_TYPE_SONG) {
            mpd_freeInfoEntity(entity);
            continue;
        }
        
        if (musicData.playing_uri != NULL && strcmp (musicData.playing_uri,song->file) == 0)
            return cont;
        
        g_free (musicData.playing_artist);
        if (song->artist)
            musicData.playing_artist = g_strdup (song->artist);
        else
            musicData.playing_artist = NULL;
        
        g_free (musicData.playing_album);
        if (song->album)
            musicData.playing_album = g_strdup (song->album);
        else
            musicData.playing_album = NULL;
        
        g_free (musicData.playing_title);
        if (song->title)
            musicData.playing_title = g_strdup (song->title);
        else
            musicData.playing_title = NULL;
        
        g_free (musicData.playing_uri);
        if (song->file)
            musicData.playing_uri = g_strdup (song->file);
        else
            musicData.playing_uri = NULL;
        
        if (entity != NULL) 
        {
            mpd_freeInfoEntity(entity);
            entity = NULL;
        }
    }
    if (entity != NULL) 
    {
        mpd_freeInfoEntity (entity);
        entity = NULL;
    }
    mpd_finishCommand (conn);
    
    /*
    mpd_sendLsInfoCommand (conn,"");
    while ((entity = mpd_getNextInfoEntity(conn)))
    {
        if (entity->type != MPD_INFO_ENTITY_TYPE_DIRECTORY)
        {
            mpd_freeInfoEntity (entity);
            continue;
        }
        
        fprintf (stdout, "Directory: %s\n", entity->info.directory->path);
        
        if (entity != NULL) 
        {
            mpd_freeInfoEntity(entity);
            entity = NULL;
        }
    }
    if (entity != NULL) 
    {
        mpd_freeInfoEntity (entity);
        entity = NULL;
    }
    mpd_finishCommand (conn);
    */
    
    gchar *cSongPath = NULL;
    
    if (cid->mpd_dir != NULL)
    {
        gchar *tmp = g_strdup_printf ("%s%s/%s", strncmp (cid->mpd_dir, "file://", 7) == 0 ? "" : "file://", 
                                  cid->mpd_dir, musicData.playing_uri);
        cSongPath = g_filename_from_uri (tmp, NULL, NULL);
        g_free (tmp);
    }
    
    if (cSongPath != NULL)
    {
        CidDataTable *p_tabFiles = cid_create_datatable(G_TYPE_STRING,"cover","album","albumart",
                                                    ".folder",".cover","folder","Cover","Folder",
                                                    G_TYPE_INVALID);
        CidDataCase *p_temp = p_tabFiles->head;
        gchar *cSongDir = g_path_get_dirname (cSongPath);
        g_free (cSongPath);
        musicData.playing_cover = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, musicData.playing_artist, musicData.playing_album);
        cid_debug ("   test de %s\n", musicData.playing_cover);
        while (p_temp != NULL && !g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS))
        {
            g_free (musicData.playing_cover);
            musicData.playing_cover = g_strdup_printf ("%s/%s.jpg", cSongDir, p_temp->content->string);
            cid_debug ("   test de %s\n", musicData.playing_cover);
            p_temp = p_temp->next;
        }
        cid_free_datatable(&p_tabFiles);
        g_free (cSongDir);
        if (! g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS))
        {
            cid->iCheckIter = 0;
            if (musicData.iSidCheckCover == 0 && cid->iPlayer != PLAYER_NONE) 
            {
                cid_debug ("l'image n'existe pas encore => on boucle.\n");
                musicData.iSidCheckCover = g_timeout_add (1 SECONDES, (GSourceFunc) _check_cover_is_present, (gpointer) NULL);
            }
        }
    }
    
    cid_info ("\nartist : %s\nalbum : %s\ntitle : %s\nsong uri : %s\ncover uri : %s",
              musicData.playing_artist,
              musicData.playing_album,
              musicData.playing_title,
              musicData.playing_uri,
              musicData.playing_cover);
    
    cid_display_image (musicData.playing_cover);
    cid_animation(cid->iAnimationType);
    
    return cont;
}

void 
cid_mpd_pipe (gint iInter) 
{
    cid->iPipe = g_timeout_add_full (G_PRIORITY_HIGH, iInter,(gpointer) cid_mpd_cover, NULL, NULL);
}

void 
cid_connect_to_mpd (gint iInter)
{
    cont = TRUE;
    cid->bPipeRunning = TRUE;
    cid_mpd_cover ();
    cid_mpd_pipe (iInter);   
}

void 
cid_disconnect_from_mpd ()
{
    cont = FALSE;
    if (cid->bPipeRunning)
        g_source_remove (cid->iPipe);
    cid->bPipeRunning = FALSE;
}

void 
_playPause_mpd (void) 
{
    if (conn)
    {
        mpd_sendPauseCommand (conn,musicData.playing);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
_next_mpd (void) 
{
    if (conn)
    {
        mpd_sendNextCommand (conn);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
_previous_mpd (void) 
{
    if (conn)
    {
        mpd_sendPrevCommand (conn);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
cid_build_mpd_menu (void)
{
    cid->pMonitorList->p_fPlayPause = _playPause_mpd;
    cid->pMonitorList->p_fNext = _next_mpd;
    cid->pMonitorList->p_fPrevious = _previous_mpd;
    cid->pMonitorList->p_fAddToQueue = NULL;
}
