/*
   *
   *                         cid-conf-panel-factory.c
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   * Sources: Cairo-Dock
   * Author:  Fabrice Rey
*/
//#include "cid.h"
#include "cid-conf-panel-factory.h"
#include "cid-messages.h"
#include "cid-callbacks.h"
#include "cid-utilities.h"

extern CidMainContainer *cid;

#define _allocate_new_buffer\
    data = g_new (gpointer, 3); \
    g_ptr_array_add (pDataGarbage, data);
    
#define _allocate_new_model\
    modele = gtk_list_store_new (CID_MODEL_NB_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF);

static int iNbConfigDialogs = 0;
static GtkListStore *s_pRendererListStore = NULL;
static GtkListStore *s_pDecorationsListStore = NULL;
static GtkListStore *s_pDecorationsListStore2 = NULL;
static GtkWidget *s_pDialog = NULL;

void cid_config_panel_destroyed (void) {
    iNbConfigDialogs --;
    if (iNbConfigDialogs <= 0) {
        cid_debug ("plus de panneaux de config\n");
        iNbConfigDialogs = 0;
        cid->bConfFilePanel = FALSE;
    }
}

gboolean cid_edit_conf_file_with_panel (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain) {
    return cid_edit_conf_file_core (pWindow, g_strdup (cConfFilePath), g_strdup (cTitle), iWindowWidth, iWindowHeight, iIdentifier, cPresentedGroup, pConfigFunc, cGettextDomain);
}

gboolean on_delete_main_gui (GtkWidget *pWidget, GdkEvent *event, GMainLoop *pBlockingLoop) {
    g_print ("%s (%x)\n", __func__, pBlockingLoop);
    if (pBlockingLoop != NULL) {
        g_print ("dialogue detruit, on sort de la boucle");
        if (g_main_loop_is_running (pBlockingLoop))
            g_main_loop_quit (pBlockingLoop);
    }
    return FALSE;
}

gboolean cid_edit_conf_file_core (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain) {
    if (s_pDialog != NULL && cid->bConfFilePanel) {
        return FALSE;
    }
    cid_message ("%s (%s)", __func__, cConfFilePath);
    GSList *pWidgetList = NULL;
    GtkTextBuffer *pTextBuffer = NULL;  // le buffer est lie au widget, donc au pDialog.
    GKeyFile *pKeyFile = g_key_file_new ();
    
    GError *erreur = NULL;
    g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
    if (erreur != NULL) {
        cid_warning (erreur->message);
        g_error_free (erreur);
        return FALSE;
    }
    g_key_file_remove_key (pKeyFile, "Desktop Entry", "X-Ubuntu-Gettext-Domain", NULL);  // salete de traducteur automatique.
    
    GPtrArray *pDataGarbage = g_ptr_array_new ();
    s_pDialog = cid_generate_ihm_from_keyfile (pKeyFile, cTitle, pWindow, &pWidgetList, (pConfigFunc != NULL)&&!cid->bSafeMode, iIdentifier, cPresentedGroup, cGettextDomain, pDataGarbage);

    g_return_val_if_fail (s_pDialog != NULL, FALSE);
    cid->bConfFilePanel = TRUE;
    
    if (iWindowWidth != 0 && iWindowHeight != 0)
        gtk_window_resize (GTK_WINDOW (s_pDialog), iWindowWidth, iWindowHeight);
        
    gtk_window_present (GTK_WINDOW (s_pDialog));
    
    gpointer *user_data = g_new (gpointer, 11);
    user_data[0] = pKeyFile;
    user_data[1] = pWidgetList;
    user_data[2] = cConfFilePath;
    user_data[3] = pTextBuffer;
    user_data[4] = pConfigFunc;
    user_data[5] = pWindow;
    user_data[6] = (gchar *)cTitle;
    user_data[7] = GINT_TO_POINTER (iWindowWidth);
    user_data[8] = GINT_TO_POINTER (iWindowHeight);
    user_data[9] = GINT_TO_POINTER ((int) iIdentifier);
    user_data[10] = pDataGarbage;
        
    g_signal_connect (s_pDialog, "response", G_CALLBACK (_cid_user_action_on_config), user_data);
    g_signal_connect (s_pDialog, "delete-event", G_CALLBACK (_cid_user_action_on_config), user_data);
        
    if (cid->bSafeMode)
    {
        cid->bBlockedWidowActive = TRUE;
        gtk_window_set_title (GTK_WINDOW (s_pDialog), _("< Maintenance mode >"));
        GMainLoop *pBlockingLoop = g_main_loop_new (NULL, FALSE);
        g_object_set_data (G_OBJECT (s_pDialog), "loop", pBlockingLoop);
        g_signal_connect (s_pDialog,
            "delete-event",
            G_CALLBACK (on_delete_main_gui),
            pBlockingLoop);
        
        g_print ("debut de boucle bloquante ...\n");
        GDK_THREADS_LEAVE ();
        g_main_loop_run (pBlockingLoop);
        GDK_THREADS_ENTER ();
        g_print ("fin de boucle bloquante\n");
        
        g_main_loop_unref (pBlockingLoop);
    }   
        
    return FALSE;
}

void cid_config_panel_created (void) {
    iNbConfigDialogs ++;
    cid_debug ("nbre de panneaux de config <- %d\n", iNbConfigDialogs);
}

GtkWidget *cid_generate_ihm_from_keyfile (GKeyFile *pKeyFile, const gchar *cTitle, GtkWindow *pParentWindow, GSList **pWidgetList, gboolean bApplyButtonPresent, gchar iIdentifier, gchar *cPresentedGroup, gchar *cGettextDomain, GPtrArray *pDataGarbage) {
    
    gpointer *data;
    int iNbBuffers = 0;
    gsize length = 0;
    gchar **pKeyList;
    gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
    
    GtkWidget *pOneWidget;
    GSList * pSubWidgetList;
    GtkWidget *pLabel, *pLabelContainer;
    GtkWidget *pVBox, *pHBox, *pSmallVBox, *pEventBox, *pRightHBox;
    GtkWidget *pEntry;
    GtkWidget *pTable;
    GtkWidget *pButtonAdd, *pButtonRemove;
    GtkWidget *pButtonDown, *pButtonUp, *pButtonConfig;
    GtkWidget *pButtonFileChooser, *pButtonPlay;
    GtkWidget *pFrame, *pFrameVBox;
    GtkWidget *pScrolledWindow;
    GtkWidget *pColorButton;
    GtkWidget *pFontButton;
    GtkWidget *pDescriptionLabel;
    GtkWidget *pPreviewImage;
    GtkWidget *pButtonConfigRenderer;
    GtkWidget *pBackButton;
    GtkCellRenderer *rend;
    GtkTreeIter iter;
    gchar *cGroupName, *cGroupComment , *cKeyName, *cKeyComment, *cUsefulComment, *cAuthorizedValuesChain, *pTipString, **pAuthorizedValuesList, *cSmallGroupIcon;
    gpointer *pGroupKeyWidget;
    int i, j, k, iNbElements;
    int iNumPage=0, iPresentedNumPage=0;
    char iElementType, iHiddenType;
    gboolean bIsAligned;
    gboolean bValue, *bValueList;
    gboolean bAddBackButton;
    int iValue, iMinValue, iMaxValue, *iValueList;
    double fValue, fMinValue, fMaxValue, *fValueList;
    gchar *cValue, **cValueList, *cSmallIcon=NULL;
    GdkColor gdkColor;
    GtkListStore *modele;
    
    gchar *cOriginalConfFilePath = g_strdup_printf ("%s/%s", CID_DATA_DIR, CID_CONFIG_FILE);
    
    GtkWidget *pDialog; 
    if (bApplyButtonPresent) {
        pParentWindow = NULL;  // evite de la rendre modale, et ainsi la fait apparaitre dans la barre des taches.
        pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
            (pParentWindow != NULL ? GTK_WINDOW (pParentWindow) : NULL),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_STOCK_APPLY,
            GTK_RESPONSE_APPLY,
            GTK_STOCK_OK,
            GTK_RESPONSE_ACCEPT,
            GTK_STOCK_QUIT,
            GTK_RESPONSE_REJECT,
            NULL);      
    } else {
        pDialog = gtk_dialog_new_with_buttons ((cTitle != NULL ? cTitle : ""),
            NULL,
            GTK_DIALOG_MODAL, 
            GTK_STOCK_OK,
            GTK_RESPONSE_ACCEPT,
            NULL);
    }
    
    gchar *cIconPath = g_strdup (CID_DEFAULT_IMAGE);
    gtk_window_set_icon_from_file (GTK_WINDOW (pDialog), cIconPath, NULL);
    g_free (cIconPath);
    
// border size
    gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), 1);
    
    GtkTooltips *pToolTipsGroup = gtk_tooltips_new ();
    
    GtkWidget *pNoteBook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
    g_object_set (G_OBJECT (pNoteBook), "tab-pos", GTK_POS_LEFT, NULL);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pNoteBook);
    
    i = 0;
    while (pGroupList[i] != NULL) {
        pVBox = NULL;
        pFrame = NULL;
        pFrameVBox = NULL;
        cGroupName = pGroupList[i];
        cGroupComment  = g_key_file_get_comment (pKeyFile, cGroupName, NULL, NULL);
        cSmallGroupIcon = NULL;
        if (cGroupComment != NULL) {
            cGroupComment[strlen(cGroupComment)-1] = (gchar)'\0';
            gchar *str = (gchar *)strrchr (cGroupComment, '[');
            if (str != NULL) {
                cSmallGroupIcon = str+1;
                str = (gchar *)strrchr (cSmallGroupIcon, ']');
                if (str != NULL)
                    *str = '\0';
            }
        }
        

        pKeyList = g_key_file_get_keys (pKeyFile, cGroupName, NULL, NULL);

        j = 0;
        while (pKeyList[j] != NULL) {
            cKeyName = pKeyList[j];

            cKeyComment =  g_key_file_get_comment (pKeyFile, cGroupName, cKeyName, NULL);
            //g_print ("%s -> %s\n", cKeyName, cKeyComment);
            if (cKeyComment != NULL && strcmp (cKeyComment, "") != 0) {
                cUsefulComment = cKeyComment;
                while (*cUsefulComment == '#' || *cUsefulComment == ' ')  // on saute les # et les espaces.
                    cUsefulComment ++;

                iElementType = *cUsefulComment;
                if (iElementType == 't') {
                    cUsefulComment ++;
                    iHiddenType = *cUsefulComment;
                }
                cUsefulComment ++;

                if (! g_ascii_isdigit (*cUsefulComment) && *cUsefulComment != '[') {
                    if (iIdentifier != 0 && *cUsefulComment != iIdentifier) {
                        g_free (cKeyComment);
                        j ++;
                        continue;
                    }
                    cUsefulComment ++;
                }

                if (pVBox == NULL) { // maintenant qu'on a au moins un element dans ce groupe, on cree sa page dans le notebook.
                    pLabel = gtk_label_new (dgettext (cGettextDomain, cGroupName));
                    
                    pLabelContainer = NULL;
                    GtkWidget *pAlign = NULL;
                    if (cSmallGroupIcon != NULL && *cSmallGroupIcon != '\0') {
// marge des box
                        pLabelContainer = gtk_hbox_new (FALSE, 5);
                        pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
                        gtk_container_add (GTK_CONTAINER (pAlign), pLabelContainer);
                        
                        GtkWidget *pImage = gtk_image_new ();
                        GdkPixbuf *pixbuf;
                        if (*cSmallGroupIcon != '/')
                            pixbuf = gtk_widget_render_icon (pImage,
                                cSmallGroupIcon ,
                                GTK_ICON_SIZE_BUTTON,
                                NULL);
                        else
// taille des icones
                            pixbuf = gdk_pixbuf_new_from_file_at_size (cSmallGroupIcon, 16, 16, NULL);
                        if (pixbuf != NULL) {
                            gtk_image_set_from_pixbuf (GTK_IMAGE (pImage), pixbuf);
                            gdk_pixbuf_unref (pixbuf);
                            gtk_container_add (GTK_CONTAINER (pLabelContainer),
                                pImage);
                        }
                        gtk_container_add (GTK_CONTAINER (pLabelContainer),
                            pLabel);
                        gtk_widget_show_all (pLabelContainer);
                    }
                    
                    pVBox = gtk_vbox_new (FALSE, 1);

                    pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
                    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);

                    gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, (pAlign != NULL ? pAlign : pLabel));
                    if (cPresentedGroup != NULL && strcmp (cPresentedGroup, cGroupName) == 0)
                        iPresentedNumPage = iNumPage;
                    iNumPage ++;
                }

                if (g_ascii_isdigit (*cUsefulComment)) {
                    iNbElements = atoi (cUsefulComment);
                    g_return_val_if_fail (iNbElements > 0, NULL);
                    while (g_ascii_isdigit (*cUsefulComment))
                        cUsefulComment ++;
                } else {
                    iNbElements = 1;
                }
                //g_print ("%d element(s)\n", iNbElements);

                while (*cUsefulComment == ' ')  // on saute les espaces.
                    cUsefulComment ++;

                if (*cUsefulComment == '[') {
                    cUsefulComment ++;
                    cAuthorizedValuesChain = cUsefulComment;

                    while (*cUsefulComment != '\0' && *cUsefulComment != ']')
                        cUsefulComment ++;
                    g_return_val_if_fail (*cUsefulComment != '\0', NULL);
                    *cUsefulComment = '\0';
                    cUsefulComment ++;
                    while (*cUsefulComment == ' ')  // on saute les espaces.
                        cUsefulComment ++;

                    pAuthorizedValuesList = g_strsplit (cAuthorizedValuesChain, ";", 0);
                } else {
                    pAuthorizedValuesList = NULL;
                }
                if (cUsefulComment[strlen (cUsefulComment) - 1] == '\n')
                    cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
                if (cUsefulComment[strlen (cUsefulComment) - 1] == '/') {
                    bIsAligned = FALSE;
                    cUsefulComment[strlen (cUsefulComment) - 1] = '\0';
                } else {
                    bIsAligned = TRUE;
                }
                //g_print ("cUsefulComment : %s\n", cUsefulComment);

                pTipString = (gchar *)strchr (cUsefulComment, '{');
                if (pTipString != NULL) {
                    if (*(pTipString-1) == '\n')
                        *(pTipString-1) ='\0';
                    else
                        *pTipString = '\0';

                    pTipString ++;

                    gchar *pTipEnd = (gchar *)strrchr (pTipString, '}');
                    if (pTipEnd != NULL)
                        *pTipEnd = '\0';
                }
// border size
                pHBox = gtk_hbox_new (FALSE, 1);
                if (pTipString != NULL) {
                    //g_print ("pTipString : '%s'\n", pTipString);
                    pEventBox = gtk_event_box_new ();
                    gtk_container_add (GTK_CONTAINER (pEventBox), pHBox);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (pToolTipsGroup),
                        pEventBox,
                        dgettext (cGettextDomain, pTipString),
                        "pouet");
                } else
                    pEventBox = NULL;

                if (*cUsefulComment != '\0' && strcmp (cUsefulComment, "...") != 0 && iElementType != 'F' && iElementType != 'X' && (iElementType != 't' || (iElementType == 't' && cid->bTesting))) {
                        pLabel = gtk_label_new (dgettext (cGettextDomain, cUsefulComment));
                        GtkWidget *pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
                        gtk_container_add (GTK_CONTAINER (pAlign), pLabel);
                        gtk_box_pack_start ((bIsAligned ? GTK_BOX (pHBox) : (pFrameVBox == NULL ? GTK_BOX (pVBox) : GTK_BOX (pFrameVBox))),
                            pAlign,
                            FALSE,
                            FALSE,
                            0);
                }

                gtk_box_pack_start (pFrameVBox == NULL ? GTK_BOX (pVBox) : GTK_BOX (pFrameVBox),
                    (pEventBox != NULL ? pEventBox : pHBox),
                    FALSE,
                    FALSE,
                    0);

                if (bIsAligned) {
// border size
                    pRightHBox = gtk_hbox_new (FALSE, 1);
                    gtk_box_pack_end (GTK_BOX (pHBox),
                        pRightHBox,
                        FALSE,
                        FALSE,
                        0);
                    pHBox = pRightHBox;
                }

                pSubWidgetList = NULL;
                bAddBackButton = FALSE;

                if (iElementType=='t' && cid->bTesting)
                    iElementType = iHiddenType;

                switch (iElementType) {
                    case 't' :  // option cachee 
                    break;
                    case 'b' :  // boolean
                        //g_print ("  + boolean\n");
                        length = 0;
                        bValueList = g_key_file_get_boolean_list (pKeyFile, cGroupName, cKeyName, &length, NULL);

                        for (k = 0; k < iNbElements; k ++) {
                            bValue =  (k < (int)length ? bValueList[k] : FALSE);
                            pOneWidget = gtk_check_button_new ();
                            gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), bValue);

                            pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pOneWidget,
                                FALSE,
                                FALSE,
                                0);
                        }
                        g_free (bValueList);
                    break;

                    case 'i' :  // integer
                    case 'I' :  // integer dans un HScale
                        //g_print ("  + integer\n");
                        length = 0;
                        iValueList = g_key_file_get_integer_list (pKeyFile, cGroupName, cKeyName, &length, NULL);
                        for (k = 0; k < iNbElements; k ++) {
                            iValue =  (k < (int)length ? iValueList[k] : 0);
                            if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[0] != NULL)
                                iMinValue = g_ascii_strtod (pAuthorizedValuesList[0], NULL);
                            else
                                iMinValue = 0;
                            if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[1] != NULL)
                                iMaxValue = g_ascii_strtod (pAuthorizedValuesList[1], NULL);
                            else
                                iMaxValue = 9999;

                            GtkObject *pAdjustment = gtk_adjustment_new (iValue,
                                0,
                                1,
                                1,
                                MAX (1, (iMaxValue - iMinValue) / 20),
                                0);
                            bAddBackButton = TRUE;
                            if (iElementType == 'I') {
                                pOneWidget = gtk_hscale_new (GTK_ADJUSTMENT (pAdjustment));
                                gtk_scale_set_digits (GTK_SCALE (pOneWidget), 0);
                                gtk_widget_set (pOneWidget, "width-request", 150, NULL);
                            } else {
                                pOneWidget = gtk_spin_button_new (GTK_ADJUSTMENT (pAdjustment),
                                    1.,
                                    0);
                            }
                            g_object_set (pAdjustment, "lower", (double) iMinValue, "upper", (double) iMaxValue, NULL); // le 'width-request' sur un GtkHScale avec 'fMinValue' non nul plante ! Donc on les met apres...
                            gtk_adjustment_set_value (GTK_ADJUSTMENT (pAdjustment), iValue);

                            pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                            gtk_box_pack_start(GTK_BOX (pHBox),
                                pOneWidget,
                                FALSE,
                                FALSE,
                                0);
                        }
                        g_free (iValueList);
                    break;

                    case 'f' :  // float.
                    case 'c' :  // float avec un bouton de choix de couleur.
                    case 'e' :  // float dans un HScale.
                        //g_print ("  + float\n");
                        length = 0;
                        fValueList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, NULL);
                        for (k = 0; k < iNbElements; k ++) {
                            fValue =  (k < (int)length ? fValueList[k] : 0);
                            if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[0] != NULL)
                                fMinValue = g_ascii_strtod (pAuthorizedValuesList[0], NULL);
                            else
                                fMinValue = 0;
                            if (pAuthorizedValuesList != NULL && pAuthorizedValuesList[1] != NULL)
                                fMaxValue = g_ascii_strtod (pAuthorizedValuesList[1], NULL);
                            else
                                fMaxValue = 9999;

                            GtkObject *pAdjustment = gtk_adjustment_new (fValue,
                                0,
                                1,
                                (fMaxValue - fMinValue) / 20.,
                                (fMaxValue - fMinValue) / 10.,
                                0);

                            if (iElementType == 'e') {
                                bAddBackButton = TRUE;
                                pOneWidget = gtk_hscale_new (GTK_ADJUSTMENT (pAdjustment));
                                gtk_scale_set_digits (GTK_SCALE (pOneWidget), 3);
                                gtk_widget_set (pOneWidget, "width-request", 150, NULL);
                            } else {
                                pOneWidget = gtk_spin_button_new (GTK_ADJUSTMENT (pAdjustment),
                                    1.,
                                    3);
                            }
                            g_object_set (pAdjustment, "lower", fMinValue, "upper", fMaxValue, NULL); // le 'width-request' sur un GtkHScale avec 'fMinValue' non nul plante ! Donc on les met apres...
                            gtk_adjustment_set_value (GTK_ADJUSTMENT (pAdjustment), fValue);

                            pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                            gtk_box_pack_start(GTK_BOX (pHBox),
                                pOneWidget,
                                FALSE,
                                FALSE,
                                0);
                        }
                        if (iElementType == 'c' && length > 2) {
                            gdkColor.red = fValueList[0] * 65535;
                            gdkColor.green = fValueList[1] * 65535;
                            gdkColor.blue = fValueList[2] * 65535;
                            pColorButton = gtk_color_button_new_with_color (&gdkColor);
                            if (length > 3) {
                                gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (pColorButton), TRUE);
                                gtk_color_button_set_alpha (GTK_COLOR_BUTTON (pColorButton), fValueList[3] * 65535);
                            } else
                                gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (pColorButton), FALSE);

                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pColorButton,
                                FALSE,
                                FALSE,
                                0);
                            g_signal_connect (G_OBJECT (pColorButton), "color-set", G_CALLBACK(_cid_set_color), pSubWidgetList);
                            g_signal_connect (G_OBJECT (pColorButton), "clicked", G_CALLBACK(_cid_get_current_color), pSubWidgetList);
                        }
                        g_free (fValueList);
                    break;

                    case 'n' :
                        cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);
                        modele = s_pRendererListStore;
                        pOneWidget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (modele));
                        rend = gtk_cell_renderer_text_new ();
                        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (pOneWidget), rend, FALSE);
// model name
                        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (pOneWidget), rend, "text", CID_MODEL_NAME, NULL);
                        
                        pDescriptionLabel = gtk_label_new (NULL);
                        gtk_label_set_use_markup  (GTK_LABEL (pDescriptionLabel), TRUE);
                        pPreviewImage = gtk_image_new_from_pixbuf (NULL);
                        _allocate_new_buffer;
                        data[0] = pDescriptionLabel;
                        data[1] = pPreviewImage;
                        g_signal_connect (G_OBJECT (pOneWidget), "changed", G_CALLBACK (_cid_select_one_item_in_combo), data);
// box margin
                        GtkWidget *pPreviewBox = gtk_vbox_new (FALSE, 5);
                        gtk_box_pack_start (GTK_BOX (pHBox),
                            pPreviewBox,
                            FALSE,
                            FALSE,
                            0);
                        gtk_box_pack_start (GTK_BOX (pPreviewBox),
                            pDescriptionLabel,
                            FALSE,
                            FALSE,
                            0);
                        gtk_box_pack_start (GTK_BOX (pPreviewBox),
                            pPreviewImage,
                            FALSE,
                            FALSE,
                            0);
                        
                        if (_cid_find_iter_from_name (modele, cValue, &iter))
                            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (pOneWidget), &iter);
                        
                        pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                        gtk_box_pack_start (GTK_BOX (pHBox),
                            pOneWidget,
                            FALSE,
                            FALSE,
                            0);
                        g_free (cValue);
                        
                        pButtonConfigRenderer = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
                        _allocate_new_buffer;
                        data[0] = pOneWidget;
                        data[1] = pParentWindow;
                        g_signal_connect (G_OBJECT (pButtonConfigRenderer),
                            "clicked",
                            G_CALLBACK (_cid_configure_renderer),
                            data);
                        gtk_box_pack_start (GTK_BOX (pHBox),
                            pButtonConfigRenderer,
                            FALSE,
                            FALSE,
                            0);
                    break ;
                    
                    case 'd' :
                    case 'o' :
                        cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);
                        modele = (iElementType == 'd' ? s_pDecorationsListStore : s_pDecorationsListStore2);
                        pOneWidget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (modele));
                        
                        rend = gtk_cell_renderer_text_new ();
                        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (pOneWidget), rend, FALSE);
// Model Name
                        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (pOneWidget), rend, "text", CID_MODEL_NAME, NULL);
                        
                        if (_cid_find_iter_from_name (modele, cValue, &iter))
                            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (pOneWidget), &iter);
                        pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                        gtk_box_pack_start (GTK_BOX (pHBox),
                            pOneWidget,
                            FALSE,
                            FALSE,
                            0);
                        g_free (cValue);
                    break ;
                    
                    case 's' :  // string
                    case 'S' :  // string avec un selecteur de fichier a cote du GtkEntry.
                    case 'u' :  // string avec un selecteur de fichier a cote du GtkEntry et un boutton play.
                    case 'D' :  // string avec un selecteur de repertoire a cote du GtkEntry.
                    case 'T' :  // string, mais sans pouvoir decochez les cases.
                    case 'E' :  // string, mais avec un GtkComboBoxEntry pour le choix unique.
                    case 'R' :  // string, avec un label pour la description.
                    case 'P' :  // string avec un selecteur de font a cote du GtkEntry.
                    case 'r' :  // string representee par son numero dans une liste de choix.
                    case 'M' :  // string, avec un label pour la description et un bouton configurer (specialement fait pour les modules).
                    case 'k' :  // string avec un selecteur de touche clavier (Merci Ctaf !)
                        //g_print ("  + string (%s)\n", cUsefulComment);
                        pEntry = NULL;
                        pDescriptionLabel = NULL;
                        pPreviewImage = NULL;
                        length = 0;
                        GdkPixbuf *pixbuf;
                        cValueList = g_key_file_get_locale_string_list (pKeyFile, cGroupName, cKeyName, NULL, &length, NULL);
                        if (iNbElements == 1) {
                            cValue =  (0 < length ? cValueList[0] : "");
                            if (pAuthorizedValuesList == NULL || pAuthorizedValuesList[0] == NULL) {
                                pOneWidget = gtk_entry_new ();
                                pEntry = pOneWidget;
                                gtk_entry_set_text (GTK_ENTRY (pOneWidget), cValue);
                            } else {
                                _allocate_new_model
                                if (iElementType == 'E') {
// Model Name
                                    pOneWidget = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (modele), CID_MODEL_NAME);
                                } else {
                                    pOneWidget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (modele));
                                    GtkCellRenderer *rend = gtk_cell_renderer_text_new ();
                                    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (pOneWidget), rend, FALSE);
// Model Name
                                    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (pOneWidget), rend, "text", CID_MODEL_NAME, NULL);
                                }

                                k = 0;
                                int iSelectedItem = -1;
                                if (iElementType == 'r')
                                    iSelectedItem = atoi (cValue);
                                gchar *cResult = (iElementType == 'r' ? g_new0 (gchar , 10) : NULL);
                                int ii, iNbElementsByItem = (iElementType == 'R' ? 3 : (iElementType == 'M' ? 4 : 1));
                                while (pAuthorizedValuesList[k] != NULL) {
                                    for (ii=0;ii<iNbElementsByItem;ii++) {
                                        if (pAuthorizedValuesList[k+ii] == NULL) {
                                            cid_warning ("bad conf file format, you can try to delete it and restart cid");
                                            break;
                                        }
                                    }
                                    if (ii != iNbElementsByItem)
                                        break;
                                    //g_print ("%d) %s\n", k, pAuthorizedValuesList[k]);
                                    GtkTreeIter iter;
                                    gtk_list_store_append (GTK_LIST_STORE (modele), &iter);
                                    if (iSelectedItem == -1 && strcmp (cValue, pAuthorizedValuesList[k]) == 0)
                                        iSelectedItem = k / iNbElementsByItem;

                                    if (cResult != NULL) {
                                        snprintf (cResult, 10, "%d", k);
                                    }
                                    if (iElementType == 'M')
// Icon Size
                                        pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], 16, 16, NULL);
                                        
                                    gtk_list_store_set (GTK_LIST_STORE (modele), &iter,
                                        CID_MODEL_NAME, (iElementType == 'r' ? dgettext (cGettextDomain, pAuthorizedValuesList[k]) : pAuthorizedValuesList[k]),
                                        CID_MODEL_RESULT, (cResult != NULL ? cResult : pAuthorizedValuesList[k]),
                                        CID_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
                                        CID_MODEL_IMAGE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL),
                                        CID_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);

                                    k += iNbElementsByItem;
                                    if (iElementType == 'R' || iElementType == 'M') {
                                        if (pAuthorizedValuesList[k-2] == NULL)  // ne devrait pas arriver si le fichier de conf est bien rempli.
                                            break;
                                    }
                                }
                                g_free (cResult);
                                if (k == 0) { // rien dans le gtktree => plantage.
                                    j ++;
                                    continue;
                                }
                                if (iElementType == 'R' || iElementType == 'M') {
                                    pDescriptionLabel = gtk_label_new (NULL);
                                    gtk_label_set_use_markup  (GTK_LABEL (pDescriptionLabel), TRUE);
                                    pPreviewImage = gtk_image_new_from_pixbuf (NULL);
                                    _allocate_new_buffer;
                                    data[0] = pDescriptionLabel;
                                    data[1] = pPreviewImage;
                                    g_signal_connect (G_OBJECT (pOneWidget), "changed", G_CALLBACK (_cid_select_one_item_in_combo), data);
                                }

                                if (iElementType != 'E' && iSelectedItem == -1)
                                    iSelectedItem = 0;
                                gtk_combo_box_set_active (GTK_COMBO_BOX (pOneWidget), iSelectedItem);
                            }
                            pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pOneWidget,
                                FALSE,
                                FALSE,
                                0);
                        } else {
                            pOneWidget = gtk_tree_view_new ();
                            _allocate_new_model;
                            gtk_tree_view_set_model (GTK_TREE_VIEW (pOneWidget), GTK_TREE_MODEL (modele));
                            gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (modele), CID_MODEL_ORDER, GTK_SORT_ASCENDING);
                            gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pOneWidget), FALSE);
                            
                            GtkCellRenderer *rend;
                            if (pAuthorizedValuesList != NULL && iElementType != 'T') { // && pAuthorizedValuesList[0] != NULL
                                rend = gtk_cell_renderer_toggle_new ();
                                gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "active", CID_MODEL_ACTIVE, NULL);
                                //\___________g_signal_connect (G_OBJECT (rend), "toggled", (GCallback) (iElementType == 'M' ? _cairo_dock_activate_one_module : _cairo_dock_activate_one_element), modele);
                            }
                            
                            if (iElementType == 'M') {
                                rend = gtk_cell_renderer_pixbuf_new ();
                                gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "pixbuf", CID_MODEL_ICON, NULL);
                            }

                            rend = gtk_cell_renderer_text_new ();
                            gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (pOneWidget), -1, NULL, rend, "text", CID_MODEL_NAME, NULL);
                            GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pOneWidget));
                            gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

                            pSubWidgetList = g_slist_append (pSubWidgetList, pOneWidget);
                            pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
                            gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
                            gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pOneWidget);
                            if (iElementType == 'M')
// Preview Height
                                gtk_widget_set (pScrolledWindow, "height-request", (int) (150 + 0), NULL);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pScrolledWindow,
                                FALSE,
                                FALSE,
                                0);

                            pSmallVBox = gtk_vbox_new (FALSE, 3);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pSmallVBox,
                                FALSE,
                                FALSE,
                                0);

                            if (iElementType != 'M') {
                                pButtonUp = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
                                g_signal_connect (G_OBJECT (pButtonUp),
                                    "clicked",
                                    G_CALLBACK (_cid_go_up),
                                    pOneWidget);
                                gtk_box_pack_start (GTK_BOX (pSmallVBox),
                                    pButtonUp,
                                    FALSE,
                                    FALSE,
                                    0);
    
                                pButtonDown = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
                                g_signal_connect (G_OBJECT (pButtonDown),
                                    "clicked",
                                    G_CALLBACK (_cid_go_down),
                                    pOneWidget);
                                gtk_box_pack_start (GTK_BOX (pSmallVBox),
                                    pButtonDown,
                                    FALSE,
                                    FALSE,
                                    0);
                            } else {
                                _allocate_new_buffer;
                                data[0] = pOneWidget;
                                data[1] = pDialog;
                                pButtonConfig = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
//\_________ Configuration applet
                                g_signal_connect (G_OBJECT (pButtonConfig),
                                    "clicked",
                                    G_CALLBACK (_cid_configure),
                                    data);
                                gtk_box_pack_start (GTK_BOX (pSmallVBox),
                                    pButtonConfig,
                                    FALSE,
                                    FALSE,
                                    0);
                            }

                            GtkTreeIter iter;
                            int iNbElementsByItem = (iElementType == 'R' ? 3 : (iElementType == 'M' ? 4 : 1));
                            if (pAuthorizedValuesList != NULL) { //  && pAuthorizedValuesList[0] != NULL
                                int l, iOrder = 0;
                                for (l = 0; l < (int)length; l ++) {
                                    cValue = cValueList[l];
                                    k = 0;
                                    while (pAuthorizedValuesList[k] != NULL) {
                                        if (strcmp (cValue, pAuthorizedValuesList[k]) == 0) {
                                            break;
                                        }
                                        k += iNbElementsByItem;
                                    }

                                    if (pAuthorizedValuesList[k] != NULL) { // c'etait bien une valeur autorisee.
                                        memset (&iter, 0, sizeof (GtkTreeIter));
                                        gtk_list_store_append (modele, &iter);
                                        if (iElementType == 'M')
// Icon Size
                                            pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], 16, 16, NULL);
                                        gtk_list_store_set (modele, &iter,
                                            CID_MODEL_ACTIVE, TRUE,
                                            CID_MODEL_NAME, cValue,
                                            CID_MODEL_RESULT, cValue,
                                            CID_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
                                            CID_MODEL_ORDER, iOrder ++,
                                            CID_MODEL_IMAGE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL), 
                                            CID_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);
                                    }
                                }
                                k = 0;
                                while (pAuthorizedValuesList[k] != NULL) {
                                    cValue =  pAuthorizedValuesList[k];
                                    for (l = 0; l < (int)length; l ++) {
                                        if (strcmp (cValue, cValueList[l]) == 0) {
                                            break;
                                        }
                                    }

                                    if (l == (int)length) { // elle n'a pas encore ete inseree.
                                        memset (&iter, 0, sizeof (GtkTreeIter));
                                        gtk_list_store_append (modele, &iter);
                                        if (iElementType == 'M')
// Icon Size
                                            pixbuf = gdk_pixbuf_new_from_file_at_size (pAuthorizedValuesList[k+3], 16, 16, NULL);
                                        gtk_list_store_set (modele, &iter,
                                            CID_MODEL_ACTIVE, FALSE,
                                            CID_MODEL_NAME, cValue,
                                            CID_MODEL_RESULT, cValue,
                                            CID_MODEL_DESCRIPTION_FILE, (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+1] : NULL),
                                            CID_MODEL_ORDER, iOrder ++,
                                            CID_MODEL_IMAGE,
                                            (iElementType == 'R' || iElementType == 'M' ? pAuthorizedValuesList[k+2] : NULL), 
                                            CID_MODEL_ICON, (iElementType == 'M' ? pixbuf : NULL), -1);
                                    }
                                    k += iNbElementsByItem;
                                }

                                if (iElementType == 'R' || iElementType == 'M') {
                                    pDescriptionLabel = gtk_label_new (NULL);
                                    gtk_label_set_use_markup (GTK_LABEL (pDescriptionLabel), TRUE);
                                    pPreviewImage = gtk_image_new_from_pixbuf (NULL);
                                    _allocate_new_buffer;
                                    data[0] = pDescriptionLabel;
                                    data[1] = pPreviewImage;
                                    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pOneWidget));
                                    gtk_tree_selection_set_select_function (selection, (GtkTreeSelectionFunc) _cid_select_one_item_in_tree, data, NULL);
                                }
                            } else { // pas de valeurs autorisees.
                                for (k = 0; k < iNbElements; k ++) {
                                    cValue =  (k < (int)length ? cValueList[k] : NULL);
                                    if (cValue != NULL) {
                                        memset (&iter, 0, sizeof (GtkTreeIter));
                                        gtk_list_store_append (modele, &iter);
                                        gtk_list_store_set (modele, &iter,
                                            CID_MODEL_ACTIVE, TRUE,
                                            CID_MODEL_NAME, cValue,
                                            CID_MODEL_RESULT, cValue,
                                            CID_MODEL_ORDER, k, -1);
                                    }
                                }
                                pTable = gtk_table_new (2, 2, FALSE);
                                gtk_box_pack_start (GTK_BOX (pHBox),
                                    pTable,
                                    FALSE,
                                    FALSE,
                                    0);
                                    
                                _allocate_new_buffer;
                                
                                pButtonAdd = gtk_button_new_from_stock (GTK_STOCK_ADD);
                                g_signal_connect (G_OBJECT (pButtonAdd),
                                    "clicked",
                                    G_CALLBACK (_cid_add),
                                    data);
                                gtk_table_attach (GTK_TABLE (pTable),
                                    pButtonAdd,
                                    0,
                                    1,
                                    0,
                                    1,
                                    GTK_SHRINK,
                                    GTK_SHRINK,
                                    0,
                                    0);
                                pButtonRemove = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
                                g_signal_connect (G_OBJECT (pButtonRemove),
                                    "clicked",
                                    G_CALLBACK (_cid_remove),
                                    data);
                                gtk_table_attach (GTK_TABLE (pTable),
                                    pButtonRemove,
                                    0,
                                    1,
                                    1,
                                    2,
                                    GTK_SHRINK,
                                    GTK_SHRINK,
                                    0,
                                    0);
                                pEntry = gtk_entry_new ();
                                gtk_table_attach (GTK_TABLE (pTable),
                                    pEntry,
                                    1,
                                    2,
                                    0,
                                    2,
                                    GTK_SHRINK,
                                    GTK_SHRINK,
                                    0,
                                    0);
                                
                                data[0] = pOneWidget;
                                data[1] = pEntry;
                            }
                        }

                        if (iElementType == 'S' || iElementType == 'D' || iElementType == 'u') {
                            if (pEntry != NULL) {
                                _allocate_new_buffer;
                                data[0] = pEntry;
                                data[1] = GINT_TO_POINTER (iElementType != 'u' ? (iElementType == 'S' ? 0 : 1) : 0);
                                data[2] = GTK_WINDOW (pDialog);
                                pButtonFileChooser = gtk_button_new_from_stock (GTK_STOCK_OPEN);
                                g_signal_connect (G_OBJECT (pButtonFileChooser),
                                    "clicked",
                                    G_CALLBACK (_cid_pick_a_file),
                                    data);
                                gtk_box_pack_start (GTK_BOX (pHBox),
                                    pButtonFileChooser,
                                    FALSE,
                                    FALSE,
                                    0);
                                if (iElementType == 'u') { //Sound Play Button
                                    pButtonPlay = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY); //Outch
                                    g_signal_connect (G_OBJECT (pButtonPlay),
                                        "clicked",
                                        G_CALLBACK (_cid_play_a_sound),
                                        data);
                                    gtk_box_pack_start (GTK_BOX (pHBox),
                                        pButtonPlay,
                                        FALSE,
                                        FALSE,
                                        0);
                                }
                            }
                        } else if (iElementType == 'R' || iElementType == 'M') {
// Marge
                            GtkWidget *pPreviewBox = gtk_vbox_new (FALSE, 1);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pPreviewBox,
                                FALSE,
                                FALSE,
                                0);
                            if (pDescriptionLabel != NULL)
                                gtk_box_pack_start (GTK_BOX (pPreviewBox),
                                    pDescriptionLabel,
                                    FALSE,
                                    FALSE,
                                    0);
                            if (pPreviewImage != NULL)
                                gtk_box_pack_start (GTK_BOX (pPreviewBox),
                                    pPreviewImage,
                                    FALSE,
                                    FALSE,
                                    0);
                        } else if (iElementType == 'P' && pEntry != NULL) {
                            pFontButton = gtk_font_button_new_with_font (gtk_entry_get_text (GTK_ENTRY (pEntry)));
                            gtk_font_button_set_show_style (GTK_FONT_BUTTON (pFontButton), FALSE);
                            gtk_font_button_set_show_size (GTK_FONT_BUTTON (pFontButton), FALSE);
                            g_signal_connect (G_OBJECT (pFontButton),
                                "font-set",
                                G_CALLBACK (_cid_set_font),
                                pEntry);
                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pFontButton,
                                FALSE,
                                FALSE,
                                0);
                        } else if (iElementType == 'k' && pEntry != NULL) {
                            GtkWidget *pGrabKeyButton = gtk_button_new_with_label(_("grab"));

                            _allocate_new_buffer;
                            data[0] = pOneWidget;
                            data[1] = pDialog;
                            gtk_widget_add_events(pDialog, GDK_KEY_PRESS_MASK);

                            g_signal_connect (G_OBJECT (pGrabKeyButton),
                                "clicked",
                                G_CALLBACK (_cid_key_grab_clicked),
                                data);

                            gtk_box_pack_start (GTK_BOX (pHBox),
                                pGrabKeyButton,
                                FALSE,
                                FALSE,
                                0);
                        }
                        g_strfreev (cValueList);
                    break;

                    case 'F' :
                    case 'X' :
                        //g_print ("  + frame\n");
                        if (pAuthorizedValuesList == NULL) {
                            pFrame = NULL;
                            pFrameVBox = NULL;
                        } else {
                            if (pAuthorizedValuesList[0] == NULL || *pAuthorizedValuesList[0] == '\0')
                                cValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, NULL);  // utile ?
                            else {
                                cValue = pAuthorizedValuesList[0];
                                cSmallIcon = pAuthorizedValuesList[1];
                            }
                            gchar *cFrameTitle;

                            
                            cFrameTitle = g_strdup_printf ("<b>%s</b>", dgettext (cGettextDomain, cValue));
                            pLabel= gtk_label_new (NULL);
                            gtk_label_set_markup (GTK_LABEL (pLabel), cFrameTitle);
                            
                            pLabelContainer = NULL;
                            if (cSmallIcon != NULL) {
// Marge Box
                                pLabelContainer = gtk_hbox_new (FALSE, 5/2);
                                GtkWidget *pImage = gtk_image_new ();
                                GdkPixbuf *pixbuf;
                                if (*cSmallIcon != '/')
                                    pixbuf = gtk_widget_render_icon (pImage,
                                        cSmallIcon ,
                                        GTK_ICON_SIZE_MENU,
                                        NULL);
                                else
// Icon Size
                                    pixbuf = gdk_pixbuf_new_from_file_at_size (cSmallIcon, 16, 16, NULL);
                                if (pixbuf != NULL) {
                                    gtk_image_set_from_pixbuf (GTK_IMAGE (pImage), pixbuf);
                                    gdk_pixbuf_unref (pixbuf);
                                    gtk_container_add (GTK_CONTAINER (pLabelContainer),
                                        pImage);
                                }
                                gtk_container_add (GTK_CONTAINER (pLabelContainer),
                                    pLabel);
                            }
                            
                            GtkWidget *pExternFrame;
                            if (iElementType == 'F') {
                                pExternFrame = gtk_frame_new (NULL);
// Marge GUI
                                gtk_container_set_border_width (GTK_CONTAINER (pExternFrame), 1);
                                gtk_frame_set_shadow_type (GTK_FRAME (pExternFrame), GTK_SHADOW_OUT);
                                gtk_frame_set_label_widget (GTK_FRAME (pExternFrame), (pLabelContainer != NULL ? pLabelContainer : pLabel));
                                pFrame = pExternFrame;
                            } else {
                                pExternFrame = gtk_expander_new (NULL);
                                gtk_expander_set_expanded (GTK_EXPANDER (pExternFrame), FALSE);
                                gtk_expander_set_label_widget (GTK_EXPANDER (pExternFrame), (pLabelContainer != NULL ? pLabelContainer : pLabel));
                                pFrame = gtk_frame_new (NULL);
// Marge GUI
                                gtk_container_set_border_width (GTK_CONTAINER (pFrame), 1);
                                gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);
                                gtk_container_add (GTK_CONTAINER (pExternFrame),
                                    pFrame);
                            }
                            
                            gtk_box_pack_start (GTK_BOX (pVBox),
                                pExternFrame,
                                FALSE,
                                FALSE,
                                0);
// Marge GUI
                            pFrameVBox = gtk_vbox_new (FALSE, 1);
                            gtk_container_add (GTK_CONTAINER (pFrame),
                                pFrameVBox);
                            g_free (cFrameTitle);
                            if (pAuthorizedValuesList[0] == NULL || *pAuthorizedValuesList[0] == '\0')
                                g_free (cValue);
                        }
                        break;

                    case 'v' :  // separateur.
                        {
                            GtkWidget *pAlign = gtk_alignment_new (.5, 0., 0.5, 0.);
                            pOneWidget = gtk_hseparator_new ();
                            gtk_container_add (GTK_CONTAINER (pAlign), pOneWidget);
                            gtk_box_pack_start(GTK_BOX (pFrameVBox != NULL ? pFrameVBox : pVBox),
                                pAlign,
                                FALSE,
                                FALSE,
                                0);
                        }
                    break ;

                    default :
                        cid_warning ("this conf file seems to be incorrect !");
                    break ;
                }

                if (pSubWidgetList != NULL) {
                    pGroupKeyWidget = g_new (gpointer, 4);
                    pGroupKeyWidget[0] = g_strdup (cGroupName);  // car on ne pourra pas le liberer s'il est partage entre plusieurs 'data'.
                    pGroupKeyWidget[1] = cKeyName;
                    pGroupKeyWidget[2] = pSubWidgetList;
                    pGroupKeyWidget[3] = (gchar *)cOriginalConfFilePath;
                    *pWidgetList = g_slist_prepend (*pWidgetList, pGroupKeyWidget);
                    if (bAddBackButton && cOriginalConfFilePath != NULL) {
                        pBackButton = gtk_button_new ();
                        GtkWidget *pImage = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_BUTTON);  // GTK_STOCK_GO_BACK
                        gtk_button_set_image (GTK_BUTTON (pBackButton), pImage);
                        g_signal_connect (G_OBJECT (pBackButton), "clicked", G_CALLBACK(_cid_set_original_value), pGroupKeyWidget);
                        gtk_box_pack_start(GTK_BOX (pHBox),
                            pBackButton,
                            FALSE,
                            FALSE,
                            0);
                    }
                }

                g_strfreev (pAuthorizedValuesList);
                g_free (cKeyComment);
            }

            j ++;
        }
        g_free (pKeyList);  // on libere juste la liste de chaines, pas les chaines a l'interieur.
        g_free (cGroupComment);
        
        i ++;
    }
    
    gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
    
    gtk_tooltips_enable (GTK_TOOLTIPS (pToolTipsGroup));
    gtk_widget_show_all (pDialog);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (pNoteBook), iPresentedNumPage);

    g_strfreev (pGroupList);
    
    cid_config_panel_created ();
    return pDialog;
}

void _cid_set_original_value (GtkButton *button, gpointer *data) {
    cid_debug ("%s (%s, %s, %s)\n", __func__, data[0], data[1], data[3]);
    gchar *cGroupName = data[0];
    gchar *cKeyName = data[1];
    GSList *pSubWidgetList = data[2];
    gchar *cOriginalConfFilePath = data[3];
    
    GSList *pList;
    gsize i = 0;
    GtkWidget *pOneWidget = pSubWidgetList->data;
    GError *erreur = NULL;
    gsize length = 0;
    
    GKeyFile *pKeyFile = g_key_file_new ();
    g_key_file_load_from_file (pKeyFile, cOriginalConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
    if (erreur != NULL) {
        cid_warning (erreur->message);
        g_error_free (erreur);
        erreur = NULL;
        return ;
    }
    
    if (GTK_IS_SPIN_BUTTON (pOneWidget) || GTK_IS_HSCALE (pOneWidget)) {
        gboolean bIsSpin = GTK_IS_SPIN_BUTTON (pOneWidget);
        double *fValuesList = g_key_file_get_double_list (pKeyFile, cGroupName, cKeyName, &length, &erreur);
        
        for (pList = pSubWidgetList; pList != NULL && i < length; pList = pList->next, i++) {
            pOneWidget = pList->data;
            if (bIsSpin)
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), fValuesList[i]);
            else
                gtk_range_set_value (GTK_RANGE (pOneWidget), fValuesList[i]);
        }
        
        g_free (fValuesList);
    }
    g_key_file_free (pKeyFile);
}
