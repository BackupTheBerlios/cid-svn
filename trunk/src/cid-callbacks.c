/*
   *
   *                            cid-callbacks.c
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/

//#include "cid.h"
#include "cid-callbacks.h"
#include "cid-panel-callbacks.h"
#include "cid-messages.h"
#include "cid-struct.h"
#include "cid-conf-panel-factory.h"
#include "cid-amazon.h"
#include "cid-config.h"
#include "cid-utilities.h"
#include "cid-asynchrone.h"

extern CidMainContainer *cid;
extern gboolean bCurrentlyDownloading, bCurrentlyDownloadingXML, bCurrentlyFocused;

gboolean bFlyingButton;

static gchar *cImageURL = NULL;
static CidMeasure *pMeasureTimer = NULL;

void cid_interrupt (int signal) {
    _cid_quit(NULL,NULL);
}

void _cid_quit (GtkWidget *p_widget, gpointer user_data) {

    cid_disconnect_player();

    cid_save_data ();

    gchar *cCommand = g_strdup_printf ("rm %s", DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    if (!system (cCommand)) return;
    g_free (cCommand);

    g_print ("Bye !\n");

    cid_sortie (CID_EXIT_SUCCESS);

    /* Parametres inutilises */
    (void)p_widget;
    (void)user_data;
}

gpointer _cid_launch_threaded (gchar *cCommand) {
    if (!system (cCommand)) return NULL;
    g_free (cCommand);
    return NULL;
}

void on_clic (GtkWidget *p_widget, GdkEventButton* pButton) {

    // si on a un clic gauche, que le clic est maintenu, et qu'on se trouve dans la zone permettant le deplacement
    if (pButton->button == 1 && pButton->type == GDK_BUTTON_PRESS && pButton->x > cid->iWidth-cid->iExtraSize && pButton->y < cid->iExtraSize) { // clic gauche
        // si on ne verouille pas la position
        if (!cid->bLockPosition) {
            if (cid->bShowAbove)
                gtk_window_set_keep_below(GTK_WINDOW (cid->cWindow), TRUE);
            gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (p_widget)),
                                        pButton->button,
                                        pButton->x_root,
                                        pButton->y_root,
                                        pButton->time); // alors on se deplace
        
        }
    } else if (pButton->button == 1 && pButton->type == GDK_BUTTON_RELEASE) {
        // on relache un clic gauche, donc on lance la fonction 'play/pause'/'next'/'previous'
        if (cid->bMonitorPlayer && cid->iPlayer != PLAYER_NONE) {
            if (cid->bDisplayControl) {
                if (pButton->x < cid->iPrevNextSize &&
                    pButton->y < (cid->iHeight + cid->iPrevNextSize)/2 &&
                    pButton->y > (cid->iHeight - cid->iPrevNextSize)/2) {
                    
                    cid->pMonitorList->p_fPrevious();
                    
                } else if  (pButton->x < (cid->iWidth + cid->iPlayPauseSize)/2 &&
                            pButton->x > (cid->iWidth - cid->iPlayPauseSize)/2 &&
                            pButton->y < (cid->iHeight + cid->iPlayPauseSize)/2 &&
                            pButton->y > (cid->iHeight - cid->iPlayPauseSize)/2) {
                
                    cid->pMonitorList->p_fPlayPause();
                
                } else if  (pButton->x > cid->iWidth - cid->iPrevNextSize &&
                            pButton->y < (cid->iHeight + cid->iPrevNextSize)/2 &&
                            pButton->y > (cid->iHeight - cid->iPrevNextSize)/2) {
                
                    cid->pMonitorList->p_fNext();
                
                }
            } else {
                cid->pMonitorList->p_fPlayPause();
            }
        }
    } else if (pButton->button == 2 && pButton->type == GDK_BUTTON_RELEASE) { // clic milieu
        // on relache un clic du milieu, donc on lance la fonction 'Next'
        if (cid->bMonitorPlayer && cid->iPlayer != PLAYER_NONE && !cid->bDisplayControl)
            cid->pMonitorList->p_fNext();
    } else if (pButton->button == 3 && pButton->type == GDK_BUTTON_RELEASE){ //clic droit
        // On relache un clic droit, donc on affiche le menu contextuel
        
        GtkWidget *menu = (GtkWidget*) cid_build_menu (cid);  

        gtk_widget_show_all (menu);

        gtk_menu_popup (GTK_MENU (menu),
                NULL,
                NULL,
                NULL,
                NULL,
                1,
                gtk_get_current_event_time ());
    }
    
    (void)p_widget;
}

void on_dragNdrop_data_received (GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata) {
    gchar *cReceivedData = (gchar *) seldata->data;
    g_return_if_fail (cReceivedData != NULL);
    int length = strlen (cReceivedData);
    if (cReceivedData[length-1] == '\n')
        cReceivedData[--length] = '\0';  // on vire le retour chariot final.
    if (cReceivedData[length-1] == '\r')
        cReceivedData[--length] = '\0';
        
    gchar **cReceivedDataList = g_strsplit(cReceivedData,"\n",0);
    //g_free (cReceivedData);
    
    
    while (*cReceivedDataList != NULL) {
        g_print (">>> %s\n",cid_toupper(*cReceivedDataList));
        cReceivedDataList++;
    }
        
    //g_print("d&d >>> %s\n",cReceivedData);
    //system (g_strdup_printf("nautilus %s &",cReceivedData));
    //g_free (cReceivedDataList);
    //g_free (cReceivedData);
}


void on_motion (GtkWidget *widget, GdkEventMotion *event) {
    
    cid->iCursorX = event->x;
    cid->iCursorY = event->y;
    
    
    if ((cid->iCursorX < cid->iPrevNextSize &&
        cid->iCursorY < (cid->iHeight + cid->iPrevNextSize)/2 &&
        cid->iCursorY > (cid->iHeight - cid->iPrevNextSize)/2)
    ||
        (cid->iCursorX > cid->iWidth - cid->iPrevNextSize &&
        cid->iCursorY < (cid->iHeight + cid->iPrevNextSize)/2 &&
        cid->iCursorY > (cid->iHeight - cid->iPrevNextSize)/2)
    ||
        (cid->iCursorX < (cid->iWidth + cid->iPlayPauseSize)/2 &&
        cid->iCursorX > (cid->iWidth - cid->iPlayPauseSize)/2 &&
        cid->iCursorY < (cid->iHeight + cid->iPlayPauseSize)/2 &&
        cid->iCursorY > (cid->iHeight - cid->iPlayPauseSize)/2)) {
        
        gtk_widget_queue_draw(cid->cWindow);
        bFlyingButton = TRUE;
    } else if (bFlyingButton) {
        bFlyingButton = FALSE;
        gtk_widget_queue_draw(cid->cWindow);
    }
}

void _cid_web_button_clicked (GtkLinkButton *button, const gchar *link_, gpointer *user_data) {
    cid_launch_web_browser(link_);
}

void _cid_about (GtkMenuItem *pMenuItem, gpointer *data) {
    
    GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (cid->cWindow),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_CLOSE,
        "\nConky Images Display (2008-2009)\n version %s",CID_VERSION);
    
#if GTK_MINOR_VERSION >= 12
    GtkWidget *pLink = gtk_link_button_new("http://cid.freezee.org/");
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pLink);
    gtk_link_button_set_uri_hook ((GtkLinkButtonUriFunc) _cid_web_button_clicked, NULL, NULL);
#endif
    
    gchar *cImagePath = g_strdup (CID_DEFAULT_IMAGE);
    GtkWidget *pImage = gtk_image_new_from_file (cImagePath);
    g_free (cImagePath);
#if GTK_MINOR_VERSION >= 12
    gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (pDialog), pImage);
#endif
    GtkWidget *pNoteBook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pNoteBook);
    
    _cid_add_about_page (pNoteBook,
        "Development",
        _("<b>Main developers :</b>\n  Benjamin SANS\n  Charlie MERLAND\n\
<b>Original idea/first development :</b>\n  Charlie MERLAND\n\
<b>Translations :</b>\n  Benjamin SANS\n\
<b>Source references :</b>\n  some part of the code is inspired by Cairo-Dock\n\
<b>Special Thanks :</b>\n  Adrien Pilleboue\n  Fabrice Rey\n"));
    
    _cid_add_about_page (pNoteBook,
        "Support",
        _("<b>Installation script :</b>\n  Benjamin SANS\n\
<b>Site (http://cid.freezee.org/) :</b>\n  Charlie MERLAND\n\
<b>Suggestions/Comments/Beta-Testers :</b>\n  Les forumeurs de ubuntu-fr\n  Les forumeurs de jeuxvideo.com\n\
\n\
<b>Any suggestion? Leave it on :</b>\n  http://cid.freezee.org/\n"));
    
    gtk_widget_show_all (pDialog);
    gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
    gtk_dialog_run (GTK_DIALOG (pDialog));
    gtk_widget_destroy (pDialog);
}

void _cid_add_about_page (GtkWidget *pNoteBook, const gchar *cPageLabel, const gchar *cAboutText) {
    GtkWidget *pVBox, *pScrolledWindow;
    GtkWidget *pPageLabel, *pAboutLabel;
    
    pPageLabel = gtk_label_new (cPageLabel);
    pVBox = gtk_vbox_new (FALSE, 0);
    pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);
    gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, pPageLabel);
    
    pAboutLabel = gtk_label_new (NULL);
    gtk_label_set_use_markup (GTK_LABEL (pAboutLabel), TRUE);
    gtk_box_pack_start (GTK_BOX (pVBox),
        pAboutLabel,
        FALSE,
        FALSE,
        0);
    gtk_label_set_markup (GTK_LABEL (pAboutLabel), cAboutText);
}

gboolean _cid_proceed_download_cover (gpointer p) {

    // Si on ne télécharge pas, on arrête la boucle direct
    if (!cid->bDownload) {
        cid_stop_measure_timer (pMeasureTimer);
        return FALSE;
    }

    // Avant tout, on dl le xml
    if (!bCurrentlyDownloadingXML && !bCurrentlyDownloading) 
        cid_get_xml_file(musicData.playing_artist,musicData.playing_album);

    // Quand on a le xml, on dl la pochette
    if (g_file_test (DEFAULT_XML_LOCATION, G_FILE_TEST_EXISTS) && !bCurrentlyDownloading) {
        if (cImageURL)
            g_free(cImageURL);
        cImageURL = NULL;
        cid_stream_file(DEFAULT_XML_LOCATION,&cImageURL);
        cid_debug ("URL : %s",cImageURL);
        if (cImageURL) {
            cid_download_missing_cover(cImageURL,DEFAULT_DOWNLOADED_IMAGE_LOCATION);
            bCurrentlyDownloadingXML = FALSE;
        } else {
            bCurrentlyDownloadingXML = FALSE;
            bCurrentlyDownloading = FALSE;
            cid_debug ("Téléchargement impossible\n");
            cid_stop_measure_timer (pMeasureTimer);
            return FALSE;
        }
    }

    // Quand on a la pochette, on l'affiche et on stoppe la boucle
    if (g_file_test (DEFAULT_DOWNLOADED_IMAGE_LOCATION, G_FILE_TEST_EXISTS)) {
        bCurrentlyDownloadingXML = FALSE;
        bCurrentlyDownloading = FALSE;
        cid_display_image(DEFAULT_DOWNLOADED_IMAGE_LOCATION);
        cid_stop_measure_timer (pMeasureTimer);
        return FALSE;
    }
    
    return TRUE;
}

gboolean _check_cover_is_present (gpointer data) {
    cid->iCheckIter++;
    if (g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS)) {
        cid_display_image(musicData.playing_cover);
        musicData.cover_exist = TRUE;
        musicData.iSidCheckCover = 0;
        return FALSE;
    } else if (cid->iCheckIter > cid->iTimeToWait && cid->bDownload) {
        if (pMeasureTimer) {
            if (cid_measure_is_running(pMeasureTimer))
                cid_stop_measure_timer(pMeasureTimer);
            if (cid_measure_is_active(pMeasureTimer))
                cid_free_measure_timer(pMeasureTimer);
        }
        pMeasureTimer = cid_new_measure_timer (2 SECONDES, NULL, NULL, (CidUpdateTimerFunc) _cid_proceed_download_cover, NULL);
        
        cid_launch_measure (pMeasureTimer);
        
        return FALSE;
    } else {
        return TRUE;
    }
}

void _cid_conf_panel (GtkMenuItem *pItemMenu, gpointer *data) {
    if (!cid->bConfFilePanel) {
        cid_save_data();
    
        cid_edit_conf_file_with_panel (GTK_WINDOW(cid->cWindow), cid->pConfFile, cid->bSafeMode ? _(" < Maintenance Mode > ") : _("CID Configuration Panel") , 750, 480, '\0', NULL, cid->bSafeMode ? (CidReadConfigFunc) cid_read_config : (CidReadConfigFunc) cid_read_config_after_update, CID_GETTEXT_PACKAGE);
    }
}
