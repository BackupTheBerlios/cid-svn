/*
   *
   *                            cid-callbacks.c
   *                                -------
   *                          Conky Images Display
   *                              Benjamin SANS
   *                    --------------------------------
   *
*/

#include "cid.h"

void cid_interrupt (int signal) {
	_cid_quit(NULL,NULL);
}

void _cid_quit (GtkWidget *p_widget, gpointer user_data) {

	rhythmbox_dbus_disconnect_from_bus();

	cid_save_data ();

	g_print ("Bye !\n");

	cid_sortie (CID_EXIT_SUCCESS);

	/* Parametres inutilises */
	(void)p_widget;
	(void)user_data;
}

void on_clic (GtkWidget *p_widget, GdkEventButton* pButton) {

	if (pButton->button == 1 && pButton->type == GDK_BUTTON_PRESS) { // clic gauche
		cid_info ("Clic gauche %s\n",(pButton->type == GDK_BUTTON_RELEASE) ? "relaché" : "maintenu");
		if (!cid->bLockPosition)
			gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (p_widget)),
										pButton->button,
										pButton->x_root,
										pButton->y_root,
										pButton->time);
	} else if (pButton->button == 1 && pButton->type == GDK_BUTTON_RELEASE) {
		cid_info("Clic court");
		if (cid->bMonitorPlayer)
			if (cid->iPlayer == PLAYER_RHYTHMBOX)
				_playPause_rhythmbox();
			else if (cid->iPlayer == PLAYER_AMAROK_1)
				_playPause_amarok ();
	} else if (pButton->button == 2 && pButton->type == GDK_BUTTON_RELEASE) { // clic milieu
		cid_info ("Clic milieu\n");
		if (cid->bMonitorPlayer)
			if (cid->iPlayer == PLAYER_RHYTHMBOX)
				_next_rhythmbox ();
			else if (cid->iPlayer == PLAYER_AMAROK_1)
				_next_amarok ();
	} else if (pButton->button == 3 && pButton->type == GDK_BUTTON_RELEASE){ //clic droit
		cid_info ("clic droit\n");
		
		GtkWidget *menu = cid_build_menu (cid);  

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

void _cid_web_button_clicked (GtkLinkButton *button, const gchar *link_, gpointer user_data) {
	cid_launch_web_browser(link_);
}

void _cid_about (GtkMenuItem *pMenuItem, gpointer *data) {
	
	gchar *cTitle = g_strdup_printf ("\nConky Images Display (2008-2009)\n version %s", CID_VERSION);
	GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (cid->cWindow),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		cTitle);
	g_free (cTitle);
	
/*************************************************************/
	GtkWidget *pLink = gtk_link_button_new("http://cid.freezee.org/");
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pLink);
	gtk_link_button_set_uri_hook ((GtkLinkButtonUriFunc) _cid_web_button_clicked, NULL, NULL);
/*************************************************************/
	
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
		"<b>Main developers :</b>\n  Benjamin SANS\n  Charlie MERLAND\n\
<b>Original idea/first development :</b>\n  Charlie MERLAND\n\
<b>Translations :</b>\n  Benjamin SANS\n\
<b>Source references :</b>\n  some part of the code is inspired by Cairo-Dock\n\
<b>Special Thanks :</b>\n  Adrien Pilleboue\n  Fabrice Rey\n");
	
	_cid_add_about_page (pNoteBook,
		"Support",
		"<b>Installation script :</b>\n  Benjamin SANS\n\
<b>Site (http://cid.freezee.org/) :</b>\n  Charlie MERLAND\n\
<b>Suggestions/Comments/Beta-Testers :</b>\n  Les forumeurs de ubuntu-fr\n  Les forumeurs de jeuxvideo.com\n\
\n\
<b>Any suggestion? Leave it on :</b>\n  http://cid.freezee.org/\n");
	
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

gboolean _check_cover_is_present (gpointer data) {
	//cid_debug ("On cherche....\n");
	if (g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS)) {
		cid_display_image(musicData.playing_cover);
		musicData.cover_exist = TRUE;
		musicData.iSidCheckCover = 0;
		return FALSE;
	} else {
		return TRUE;
	}
}

void _cid_conf_panel (GtkMenuItem *pItemMenu, gpointer *data) {
	cid_save_data();
	
	cid_edit_conf_file_with_panel (GTK_WINDOW(cid->cWindow), cid->pConfFile, "CID Configuration Panel", 690, 480, '\0', NULL, (CidReadConfigFunc) cid_read_config_after_update, CID_GETTEXT_PACKAGE);
}

void _cid_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data) {
	GKeyFile *pKeyFile = user_data[0];
	GSList *pWidgetList = user_data[1];
	gchar *cConfFilePath = user_data[2];
	GtkTextBuffer *pTextBuffer = user_data[3];
	CidReadConfigFunc pConfigFunc = user_data[4];
	GtkWindow *pWindow = user_data[5];
	gchar *cTitle = user_data[6];
	int iWindowWidth = GPOINTER_TO_INT (user_data[7]);
	int iWindowHeight = GPOINTER_TO_INT (user_data[8]);
	gchar iIdentifier = GPOINTER_TO_INT (user_data[9]);
	GPtrArray *pDataGarbage = user_data[10];

	if (action == GTK_RESPONSE_ACCEPT || action == GTK_RESPONSE_APPLY) {
		gtk_window_set_modal (GTK_WINDOW (pDialog), TRUE);  // pour prevenir tout interaction avec l'appli pendant sa re-configuration.

		if (pWidgetList != NULL) {
			cid_update_keyfile_from_widget_list (pKeyFile, pWidgetList);
			cid_write_keys_to_file (pKeyFile, cConfFilePath);
		} else {
			GtkTextIter start, end;
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &start, 0);
			gtk_text_buffer_get_iter_at_offset (pTextBuffer, &end, -1);

			gchar *cConfiguration = gtk_text_buffer_get_text (pTextBuffer, &start, &end, FALSE);

			gboolean write_ok = g_file_set_contents (cConfFilePath, cConfiguration, -1, NULL);
			g_free (cConfiguration);
			if (! write_ok)
				cid_warning ("error while writing to %s", cConfFilePath);
		}

		if (pConfigFunc != NULL)
			pConfigFunc (cConfFilePath, /*data*/NULL);
			
		gtk_window_set_modal (GTK_WINDOW (pDialog), FALSE);
	}

	if (action == GTK_RESPONSE_ACCEPT || action == GTK_RESPONSE_REJECT || action == GTK_RESPONSE_NONE) {
		//cairo_dock_mark_prefered_conf_file (cConfFilePath);

		gtk_widget_destroy (GTK_WIDGET (pDialog));
		cid_config_panel_destroyed ();
		g_key_file_free (pKeyFile);
		cid_free_generated_widget_list (pWidgetList);
		g_ptr_array_foreach (pDataGarbage, (GFunc) g_free, NULL);
		g_ptr_array_free (pDataGarbage, TRUE);
		g_free (cConfFilePath);
		g_free (cTitle);
		g_free (user_data);
	}
}

void _cid_set_color (GtkColorButton *pColorButton, GSList *pWidgetList) {
	GdkColor gdkColor;
	gtk_color_button_get_color (pColorButton, &gdkColor);

	GtkSpinButton *pSpinButton;
	GSList *pList = pWidgetList;
	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.red / 65535);
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.green / 65535);
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gtk_spin_button_set_value (pSpinButton, 1. * gdkColor.blue / 65535);
	pList = pList->next;

	if (gtk_color_button_get_use_alpha (pColorButton))
	{
		if (pList == NULL)
		return;
		pSpinButton = pList->data;
		gtk_spin_button_set_value (pSpinButton, 1. * gtk_color_button_get_alpha (pColorButton) / 65535);
	}
}

void _cid_get_current_color (GtkColorButton *pColorButton, GSList *pWidgetList) {
	GdkColor gdkColor;
	GtkSpinButton *pSpinButton;

	GSList *pList = pWidgetList;
	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.red = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.green = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	gdkColor.blue = gtk_spin_button_get_value (pSpinButton) * 65535;
	pList = pList->next;

	gtk_color_button_set_color (pColorButton, &gdkColor);

	if (pList == NULL)
		return;
	pSpinButton = pList->data;
	if (gtk_color_button_get_use_alpha (pColorButton))
		gtk_color_button_set_alpha (pColorButton, gtk_spin_button_get_value (pSpinButton) * 65535);
}

gboolean _cid_find_iter_from_name (GtkListStore *pModele, gchar *cName, GtkTreeIter *iter) {
	if (cName == NULL)
		return FALSE;
	gboolean bFound = FALSE;
	gpointer data[3] = {cName, iter, &bFound};
	gtk_tree_model_foreach (GTK_TREE_MODEL (pModele), (GtkTreeModelForeachFunc) _cid_test_one_name, data);
	return bFound;
}

gboolean _cid_test_one_name (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer *data) {
	gchar *cName = NULL;
	gtk_tree_model_get (model, iter, CID_MODEL_NAME, &cName, -1);
	if (strcmp (data[0], cName) == 0)
	{
		GtkTreeIter *iter_to_fill = data[1];
		memcpy (iter_to_fill, iter, sizeof (GtkTreeIter));
		gboolean *bFound = data[2];
		*bFound = TRUE;
		return TRUE;
	}
	return FALSE;
}

gboolean _cid_increase_order (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder) {
	int iMyOrder;
	gtk_tree_model_get (model, iter, CID_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder == *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CID_MODEL_ORDER, iMyOrder + 1, -1);
		return TRUE;
	}
	return FALSE;
}

void _cid_go_up (GtkButton *button, GtkTreeView *pTreeView) {
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	int iOrder;
	gtk_tree_model_get (pModel, &iter, CID_MODEL_ORDER, &iOrder, -1);
	iOrder --;
	if (iOrder < 0)
		return;

	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cid_increase_order, &iOrder);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter, CID_MODEL_ORDER, iOrder, -1);
}

gboolean _cid_decrease_order (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder) {
	int iMyOrder;
	gtk_tree_model_get (model, iter, CID_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder == *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CID_MODEL_ORDER, iMyOrder - 1, -1);
		return TRUE;
	}
	return FALSE;
}

void _cid_go_down (GtkButton *button, GtkTreeView *pTreeView) {
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	int iOrder;
	gtk_tree_model_get (pModel, &iter, CID_MODEL_ORDER, &iOrder, -1);
	iOrder ++;
	//g_print ("  ordre max : %d\n", gtk_tree_model_iter_n_children (pModel, NULL) - 1);
	if (iOrder > gtk_tree_model_iter_n_children (pModel, NULL) - 1)
		return;

	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cid_decrease_order, &iOrder);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter, CID_MODEL_ORDER, iOrder, -1);
}

void _cid_add (GtkButton *button, gpointer *data) {
	GtkTreeView *pTreeView = data[0];
	GtkWidget *pEntry = data[1];

	GtkTreeIter iter;
	memset (&iter, 0, sizeof (GtkTreeIter));

	GtkTreeModel *pModel = gtk_tree_view_get_model (pTreeView);
	gtk_list_store_append (GTK_LIST_STORE (pModel), &iter);

	gtk_list_store_set (GTK_LIST_STORE (pModel), &iter,
		CID_MODEL_ACTIVE, TRUE,
		CID_MODEL_NAME, gtk_entry_get_text (GTK_ENTRY (pEntry)),
		CID_MODEL_ORDER, gtk_tree_model_iter_n_children (pModel, NULL) - 1, -1);
	//g_print (" -> ordre %d\n", gtk_tree_model_iter_n_children (pModel, NULL) - 1);

	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
	gtk_tree_selection_select_iter (pSelection, &iter);
}

 gboolean _cid_decrease_order_if_greater (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, int *pOrder) {
	int iMyOrder;
	gtk_tree_model_get (model, iter, CID_MODEL_ORDER, &iMyOrder, -1);

	if (iMyOrder > *pOrder)
	{
		gtk_list_store_set (GTK_LIST_STORE (model), iter, CID_MODEL_ORDER, iMyOrder - 1, -1);
		return TRUE;
	}
	return FALSE;
}

void _cid_remove (GtkButton *button, gpointer *data) {
	GtkTreeView *pTreeView = data[0];
	GtkWidget *pEntry = data[1];

	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);
	GtkTreeModel *pModel;

	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
	return ;

	gchar *cValue = NULL;
	int iOrder;
	gtk_tree_model_get (pModel, &iter,
		CID_MODEL_NAME, &cValue,
		CID_MODEL_ORDER, &iOrder, -1);

	gtk_list_store_remove (GTK_LIST_STORE (pModel), &iter);
	gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cid_decrease_order_if_greater, &iOrder);

	gtk_entry_set_text (GTK_ENTRY (pEntry), cValue);
	g_free (cValue);
}

void _cid_show_image_preview (GtkFileChooser *pFileChooser, GtkImage *pPreviewImage) {
	gchar *cFileName = gtk_file_chooser_get_preview_filename (pFileChooser);
	if (cFileName == NULL)
		return ;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cFileName, 64, 64, NULL);
	g_free (cFileName);
	if (pixbuf != NULL)
	{
		gtk_image_set_from_pixbuf (pPreviewImage, pixbuf);
		gdk_pixbuf_unref (pixbuf);
		gtk_file_chooser_set_preview_widget_active (pFileChooser, TRUE);
	}
	else
		gtk_file_chooser_set_preview_widget_active (pFileChooser, FALSE);
}

void _cid_pick_a_file (GtkButton *button, gpointer *data) {
	GtkEntry *pEntry = data[0];
	gint iFileType = GPOINTER_TO_INT (data[1]);
	GtkWindow *pParentWindow = data[2];

	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		(iFileType == 0 ? "Pick up a file" : "Pick up a directory"),
		pParentWindow,
		(iFileType == 0 ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER),
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	gchar *cDirectoryPath = g_path_get_basename (gtk_entry_get_text (pEntry));
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog), cDirectoryPath);
	g_free (cDirectoryPath);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), FALSE);

	GtkWidget *pPreviewImage = gtk_image_new ();
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (pFileChooserDialog), pPreviewImage);
	g_signal_connect (GTK_FILE_CHOOSER (pFileChooserDialog), "update-preview", G_CALLBACK (_cid_show_image_preview), pPreviewImage);

	gtk_widget_show (pFileChooserDialog);
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		gchar *cFilePath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pFileChooserDialog));
		gtk_entry_set_text (pEntry, cFilePath);
	}
	gtk_widget_destroy (pFileChooserDialog);
}

void _cid_play_a_sound (GtkButton *button, gpointer *data) {
	GtkWidget *pEntry = data[0];
	const gchar *cSoundPath = gtk_entry_get_text (GTK_ENTRY (pEntry));
	cid_play_sound (cSoundPath);
}

void _cid_set_font (GtkFontButton *widget, GtkEntry *pEntry) {
	const gchar *cFontName = gtk_font_button_get_font_name (GTK_FONT_BUTTON (widget));
	cid_message (" -> %s\n", cFontName);
	if (cFontName != NULL)
		gtk_entry_set_text (pEntry, cFontName);
}

void _cid_key_grab_cb (GtkWidget *wizard_window, GdkEventKey *event, GtkEntry *pEntry) {
	gchar *key;
	cid_message ("key press event\n");
	if (gtk_accelerator_valid (event->keyval, event->state))
	{
		/* This lets us ignore all ignorable modifier keys, including
		* NumLock and many others. :)
		*
		* The logic is: keep only the important modifiers that were pressed
		* for this event. */
		event->state &= gtk_accelerator_get_default_mod_mask();

		/* Generate the correct name for this key */
		key = gtk_accelerator_name (event->keyval, event->state);

		g_printerr ("KEY GRABBED: %s\n", key);

		/* Re-enable widgets */
		gtk_widget_set_sensitive (GTK_WIDGET(pEntry), TRUE);

		/* Disconnect the key grabber */
		g_signal_handlers_disconnect_by_func (GTK_OBJECT(wizard_window), GTK_SIGNAL_FUNC(_cid_key_grab_cb), pEntry);

		/* Copy the pressed key to the text entry */
		gtk_entry_set_text (GTK_ENTRY(pEntry), key);

		/* Free the string */
		g_free (key);
	}
}

void _cid_key_grab_clicked (GtkButton *button, gpointer *data) {
	GtkEntry *pEntry = data[0];
	GtkWindow *pParentWindow = data[1];

	cid_message ("clicked\n");
	//set widget insensitive
	gtk_widget_set_sensitive (GTK_WIDGET(pEntry), FALSE);
	//  gtk_widget_set_sensitive (wizard_notebook, FALSE);

	g_signal_connect (GTK_WIDGET(pParentWindow), "key-press-event", GTK_SIGNAL_FUNC(_cid_key_grab_cb), pEntry);
}

gpointer _cid_launch_threaded (gchar *cCommand) {
	system (cCommand);
	g_free (cCommand);
	return NULL;
}

void _cid_selection_changed (GtkTreeModel *model, GtkTreeIter iter, gpointer *data) {
	GtkLabel *pDescriptionLabel = data[0];
	GtkImage *pPreviewImage = data[1];

	gchar *cDescriptionFilePath = NULL, *cPreviewFilePath;
	gtk_tree_model_get (model, &iter, CID_MODEL_DESCRIPTION_FILE, &cDescriptionFilePath, CID_MODEL_IMAGE, &cPreviewFilePath, -1);

	if (cDescriptionFilePath != NULL)
	{
		gboolean bDistant = FALSE;
		if (strncmp (cDescriptionFilePath, "http://", 7) == 0 || strncmp (cDescriptionFilePath, "ftp://", 6) == 0)
		{
			g_print ("fichier readme distant (%s)\n", cDescriptionFilePath);
			
			gchar *cTmpFilePath = g_strdup ("/tmp/cid-readme.XXXXXX");
			int fds = mkstemp (cTmpFilePath);
			if (fds == -1)
			{
				g_free (cTmpFilePath);
				return ;
			}
			
			gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s' -t 2 -w 2", cDescriptionFilePath, cTmpFilePath);
			system (cCommand);
			g_free (cCommand);
			close(fds);
			
			g_free (cDescriptionFilePath);
			cDescriptionFilePath = cTmpFilePath;
			bDistant = TRUE;
		}
		gchar *cDescription = NULL;
		gsize length = 0;
		GError *erreur = NULL;
		g_file_get_contents  (cDescriptionFilePath,
			&cDescription,
			&length,
			&erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			cDescription = g_strdup ("");
		}
		gtk_label_set_markup (pDescriptionLabel, cDescription);
		g_free (cDescription);
		if (bDistant)
		{
			g_remove (cDescriptionFilePath);
		}
	}

	if (cPreviewFilePath != NULL)
	{
		gboolean bDistant = FALSE;
		if (strncmp (cPreviewFilePath, "http://", 7) == 0 || strncmp (cPreviewFilePath, "ftp://", 6) == 0)
		{
			g_print ("fichier preview distant (%s)\n", cPreviewFilePath);
			
			gchar *cTmpFilePath = g_strdup ("/tmp/cid-preview.XXXXXX");
			int fds = mkstemp (cTmpFilePath);
			if (fds == -1)
			{
				g_free (cTmpFilePath);
				return ;
			}
			
			gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s' -t 2 -w 2", cPreviewFilePath, cTmpFilePath);
			system (cCommand);
			g_free (cCommand);
			close(fds);
			
			g_free (cPreviewFilePath);
			cPreviewFilePath = cTmpFilePath;
			bDistant = TRUE;
		}
		
		int iPreviewWidth, iPreviewHeight;
		GdkPixbuf *pPreviewPixbuf = NULL;
		if (gdk_pixbuf_get_file_info (cPreviewFilePath, &iPreviewWidth, &iPreviewHeight) != NULL)
		{
// Size
			iPreviewWidth = MIN (iPreviewWidth, 200);
			iPreviewHeight = MIN (iPreviewHeight, 100);
			pPreviewPixbuf = gdk_pixbuf_new_from_file_at_size (cPreviewFilePath, iPreviewWidth, iPreviewHeight, NULL);
		}
		if (pPreviewPixbuf == NULL)
		{
			cid_warning ("pas de prevue disponible\n");
			pPreviewPixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				TRUE,
				8,
				1,
				1);
		}
		gtk_image_set_from_pixbuf (pPreviewImage, pPreviewPixbuf);
		gdk_pixbuf_unref (pPreviewPixbuf);
		
		if (bDistant)
		{
			g_remove (cPreviewFilePath);
		}
	}

	g_free (cDescriptionFilePath);
	g_free (cPreviewFilePath);
}

void _cid_select_one_item_in_combo (GtkComboBox *widget, gpointer *data) {
	GtkTreeModel *model = gtk_combo_box_get_model (widget);
	g_return_if_fail (model != NULL);

	GtkTreeIter iter;
	gtk_combo_box_get_active_iter (widget, &iter);

	_cid_selection_changed (model, iter, data);
}

void _cid_configure_renderer (GtkButton *button, gpointer *data) {
	GtkTreeView *pCombo = data[0];
	GtkWindow *pDialog = data[1];
	 
	//cairo_dock_configure_module (pDialog, "rendering");
}

void _cid_configure (GtkButton *button, gpointer *data) {
	GtkTreeView *pTreeView = data[0];
	GtkWindow *pDialog = data[1];
	GtkTreeSelection *pSelection = gtk_tree_view_get_selection (pTreeView);

	GtkTreeModel *pModel;
	GtkTreeIter iter;
	if (! gtk_tree_selection_get_selected (pSelection, &pModel, &iter))
		return ;

	gchar *cSelectedValue = NULL;
	gtk_tree_model_get (pModel, &iter, CID_MODEL_RESULT, &cSelectedValue, -1);

	//cairo_dock_configure_module (NULL, cSelectedValue);
	g_free (cSelectedValue);
}

gboolean _cid_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, gpointer *data) {
	GtkTreeIter iter;
	gtk_tree_model_get_iter (model, &iter, path);

	_cid_selection_changed (model, iter, data);
	return TRUE;
}

gboolean _cid_get_active_elements (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, GSList **pStringList) {
	//g_print ("%s (%d)\n", __func__, *pOrder);
	gboolean bActive;
	gchar *cValue = NULL;
	gtk_tree_model_get (model, iter, CID_MODEL_ACTIVE, &bActive, CID_MODEL_NAME, &cValue, -1);

	if (bActive)
	{
		*pStringList = g_slist_append (*pStringList, cValue);
	}
	else
	{
		g_free (cValue);
	}
	return FALSE;
}

void _cid_free_widget_list (gpointer *data, gpointer user_data)
{
	//g_print ("%s - %s\n", (gchar *)data[0], (gchar *)data[1]);
	g_free (data[0]);
	g_free (data[1]);
	g_slist_free (data[2]);  // les elements de data[2] sont les widgets, et se feront liberer lors de la destruction de la fenetre.
}

void cid_free_generated_widget_list (GSList *pWidgetList) {
	g_slist_foreach (pWidgetList, (GFunc) _cid_free_widget_list, NULL);
	g_slist_free (pWidgetList);
}
