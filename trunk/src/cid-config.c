/*
   *
   *                             cid-config.c
   *                               -------
   *                         Conky Images Display
   *             16/10/2008 - Charlie MERLAND / Benjamin SANS
   *             --------------------------------------------
   *
*/

#include "cid.h"
#include <math.h>

static gint iNbRead=0;
FileSettings *conf;

//gsize colorSize2 = 4* sizeof (double);

void cid_check_file (const gchar *f) {
	gchar *cCommand=NULL;
	gchar *cCommand2;
	if (!g_file_test (f, G_FILE_TEST_EXISTS)) {
		if (g_file_test (g_strdup_printf("%s/%s",g_getenv("HOME"),OLD_CONFIG_FILE), G_FILE_TEST_EXISTS)) {
			cCommand = g_strdup_printf("rm %s/%s",g_getenv("HOME"),OLD_CONFIG_FILE);
		} else if (g_file_test (g_strdup_printf("%s/.config/%s",g_getenv("HOME"),OLD_CONFIG_FILE), G_FILE_TEST_EXISTS)) {
			cCommand = g_strdup_printf("rm %s/.config/%s",g_getenv("HOME"),OLD_CONFIG_FILE);
		} 
		cCommand2 = g_strdup_printf ("cp %s/%s %s", CID_DATA_DIR, CID_CONFIG_FILE, cid->pConfFile);
		cid_info ("Commande: %s\n", cCommand);
		cid_info ("Commande: %s\n", cCommand2);
		system (cCommand);
		system (cCommand2);
		g_free (cCommand);
		g_free (cCommand2);
	}
}

gboolean cid_check_conf_file_version (const gchar *f) {
	gchar *cCommand=NULL;
	gchar line_f1[80], line_f2[80];
	FILE *f1, *f2;
	f1 = fopen ((const char *)g_strdup_printf("%s/%s",CID_DATA_DIR, CID_CONFIG_FILE),"r");
	f2 = fopen ((const char *)f,"r");
	
	if (!fgets(line_f1,80,f1))
		cid_exit (3,"couldn't read conf file, try to delete it");
	if (!fgets(line_f2,80,f2))
		cid_exit (3,"couldn't read conf file, try to delete it");
	
	fclose (f1);
	fclose (f2);
	
	cid_info ("line_f1 %s\nline_f2 %s\n",line_f1,line_f2);
		
	if (strcmp(line_f1,line_f2)!=0) {
		cid_info ("bad file version, building a new one\n");
		cCommand = g_strdup_printf("rm %s",f);
		system (cCommand);
		g_free (cCommand);
		cCommand = g_strdup_printf ("cp %s/%s %s", CID_DATA_DIR, CID_CONFIG_FILE, cid->pConfFile);
		system (cCommand);
		g_free (cCommand);
		
		cid_save_data ();
		cid_read_key_file (cid->pConfFile);
		cid_merge_config (conf);
		return FALSE;
	}
	return TRUE;
}

void cid_merge_config (FileSettings *conf) {
	if (iNbRead>0) {
		//g_free (cid->pPlayer);
		g_free (cid->dColor);
		g_free (cid->dFlyingColor);
	}
	//cid->pPlayer        = (strcmp(conf->player_name,"")!=0) ? g_strdup(conf->player_name) : NULL;
	cid->bHide          = (conf->hide);
	cid->bKeepCorners   = (conf->corners);
	cid->bLockPosition  = (conf->lock);
	cid->bAllDesktop    = (conf->allDesktop);
	cid->bRunAnimation  = (conf->animation);
	cid->bMonitorPlayer = (conf->monitoring);
	cid->bThreaded      = (conf->threaded);
	cid->iPlayer        = (conf->iPlayer);
	cid->iAnimationType = (conf->iAnimationType);
	cid->iInter         = (conf->inter!=0) ? (conf->inter * 1000) : (5 * 1000);
	cid->iPosX          = (conf->pos_x!=0) ? (conf->pos_x) : 0;
	cid->iPosY          = (conf->pos_y!=0) ? (conf->pos_y) : 0;
	cid->iWidth         = (conf->size_x!=0) ? (conf->size_x) : 150;
	cid->iHeight        = (conf->size_y!=0) ? (conf->size_y) : 150;
	cid->dColor         = conf->couleur;
	cid->dFlyingColor   = conf->couleurSurvol;
	cid->dRed           = cid->dColor[0];
	cid->dGreen         = cid->dColor[1];
	cid->dBlue          = cid->dColor[2];
	cid->dAlpha         = cid->dColor[3];
	cid->dRotate        = (conf->rotation);
	DEFAULT_IMAGE       = (g_file_test (conf->image, G_FILE_TEST_EXISTS)) ? g_strdup(conf->image) : g_strdup(CID_DEFAULT_IMAGE);
}

void cid_read_config_after_update (const char *f, gpointer *pData) {
	g_slice_free (FileSettings,conf);
	cid_read_config (f);
	
	cid_disconnect_player ();
	
	cid_free_musicData();
	
	cid_run_with_player();
	gtk_window_move (GTK_WINDOW(cid->cWindow), cid->iPosX, cid->iPosY);
	gtk_window_resize (GTK_WINDOW (cid->cWindow), cid->iWidth, cid->iHeight);
	if (!cid->bAllDesktop)
		gtk_window_unstick(GTK_WINDOW(cid->cWindow));
	else
		gtk_window_stick(GTK_WINDOW(cid->cWindow));
	gtk_widget_queue_draw (cid->cWindow);
	
	(void) pData;
}

gboolean cid_load_key_file(void) {
	GKeyFileFlags flags;
	GError *error = NULL;

	/* Create a new GKeyFile object and a bitwise list of flags. */
	cid->pKeyFile = g_key_file_new ();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load the GKeyFile from keyfile.conf or return. */
	if (!g_key_file_load_from_file (cid->pKeyFile, cid->pConfFile, flags, &error)) {
		g_error (error->message);
		return FALSE;
	}
	return TRUE;
}

void cid_key_file_free(void) {
	g_key_file_free (cid->pKeyFile);
}

gboolean cid_get_boolean_value (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault) {
	GError *error = NULL;
	gboolean bGet = g_key_file_get_boolean (pKeyFile, cGroup, cKey, &error);
	if (error != NULL) {
		g_error_free(error);
		error = NULL;
		bGet = bDefault;
	}
	return bGet;
}

gchar *cid_get_string_value (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gchar *cDefault) {
	GError *error = NULL;
	gchar *cGet = g_key_file_get_string (pKeyFile, cGroup, cKey, &error);
	if (error != NULL) {
		g_error_free(error);
		error = NULL;
		cGet = cDefault;
	}
	return cGet;
}

int cid_read_key_file (const gchar *f) {
	GError *monitor = NULL, *image = NULL, *animation = NULL, *allDesktop = NULL, *move = NULL;
	
	if (!cid_load_key_file())
		cid_exit(CID_ERROR_READING_FILE,"Key File error");
	
	// [player] configuration
	conf->iPlayer    = g_key_file_get_integer  (cid->pKeyFile, "player", "PLAYER", NULL);
	conf->inter      = g_key_file_get_integer (cid->pKeyFile, "player", "INTER", NULL);
	conf->monitoring = CID_CONFIG_GET_BOOLEAN ("player", "MONITOR", TRUE);

	// [options] configuration
	conf->lock           = CID_CONFIG_GET_BOOLEAN ("options", "LOCK", TRUE);
	conf->hide           = CID_CONFIG_GET_BOOLEAN ("options", "HIDE", FALSE);
	conf->image          = CID_CONFIG_GET_STRING  ("options", "IMAGE", CID_DEFAULT_IMAGE);
	conf->animation      = CID_CONFIG_GET_BOOLEAN ("options", "ANIMATION", TRUE);
	conf->iAnimationType = g_key_file_get_integer (cid->pKeyFile, "options", "ANIMATION_TYPE", NULL);
	conf->threaded       = CID_CONFIG_GET_BOOLEAN ("options", "THREAD", FALSE);
	
	// [position] configuration
	conf->pos_x    = g_key_file_get_integer (cid->pKeyFile, "position", "GAP_X", NULL);
	conf->pos_y    = g_key_file_get_integer (cid->pKeyFile, "position", "GAP_Y", NULL);	
	conf->size_x   = g_key_file_get_integer (cid->pKeyFile, "position", "SIZE_X", NULL);
	conf->size_y   = g_key_file_get_integer (cid->pKeyFile, "position", "SIZE_Y", NULL);
	conf->rotation = g_key_file_get_double  (cid->pKeyFile, "position", "ROTATION", NULL);
	conf->couleur  = g_key_file_get_double_list (cid->pKeyFile, "position", "COLOR", &cid->gColorSize, NULL);
	conf->couleurSurvol  = g_key_file_get_double_list (cid->pKeyFile, "position", "FLYING_COLOR", &cid->gFlyingColorSize, NULL);
	conf->corners  = CID_CONFIG_GET_BOOLEAN ("position", "KEEP_CORNERS", FALSE);
	conf->allDesktop = CID_CONFIG_GET_BOOLEAN ("position", "ALL_DESKTOP", TRUE);
	cid_key_file_free();
}

int cid_read_config (const char *f) {
	cid_info ("Reading file : %s\n",f);
	
	/* Create a new Settings object. If you are using GTK+ 2.8 or below, you should
	* use g_new() or g_malloc() instead! */
	conf = g_slice_new (FileSettings);
	
	cid_check_file (f);	
			
	cid_read_key_file (f);
	cid_merge_config (conf);
	
	if (!cid->bDevMode) 
		cid_check_conf_file_version (f);
	//cid_player_evaluation (cid->pPlayer);
	
	cid_free_conf (conf);
	
	iNbRead++;

	return 0;
}

void cid_free_conf (FileSettings *conf) {
	//g_free(conf->player_name);
	g_free(conf->image);
}

void cid_get_data () {
	/* On récupère la position de cid */
	gtk_window_get_position(GTK_WINDOW (cid->cWindow), &cid->iPosX, &cid->iPosY);
	
	/* On récupère la taille de cid */
	gtk_window_get_size(GTK_WINDOW (cid->cWindow), &cid->iWidth, &cid->iHeight);
}

void cid_save_data () {
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;

	/* Create a new GKeyFile object and a bitwise list of flags. */
	keyfile = g_key_file_new ();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load the GKeyFile from keyfile.conf or return. */
	if (!g_key_file_load_from_file (keyfile, cid->pConfFile, flags, &error)) {
		g_error (error->message);
		return;
	}
	
	if (cid->cWindow!=NULL)
		cid_get_data();
	
	// [player] configuration
	g_key_file_set_integer (keyfile, "player", "PLAYER", cid->iPlayer);
	g_key_file_set_integer (keyfile, "player", "INTER", cid->iInter/1000);
	g_key_file_set_boolean (keyfile, "player", "MONITOR", cid->bMonitorPlayer);

	// [options] configuration
	g_key_file_set_boolean (keyfile, "options", "LOCK", cid->bLockPosition);
	g_key_file_set_boolean (keyfile, "options", "ANIMATION", cid->bRunAnimation);
	g_key_file_set_boolean (keyfile, "options", "HIDE", cid->bHide);
	if (strcmp(cid->pDefaultImage,TESTING_COVER)!=0 && strcmp(cid->pDefaultImage,CID_DEFAULT_IMAGE)!=0)
		g_key_file_set_string  (keyfile, "options", "IMAGE", (g_file_test (cid->pDefaultImage, G_FILE_TEST_EXISTS) ? cid->pDefaultImage : /*CID_DEFAULT_IMAGE*/""));
	else
		g_key_file_set_string  (keyfile, "options", "IMAGE", "");
	g_key_file_set_boolean (keyfile, "options", "THREAD", cid->bThreaded);
	g_key_file_set_integer (keyfile, "options", "ANIMATION_TYPE", cid->iAnimationType);
		
	// [position] configuration
	g_key_file_set_integer (keyfile, "position", "SIZE_X",cid->iWidth<=175 ? cid->iWidth : 175);
	g_key_file_set_integer (keyfile, "position", "SIZE_Y",cid->iHeight<=175 ? cid->iHeight : 175);
	g_key_file_set_integer (keyfile, "position", "GAP_X",cid->iPosX);
	g_key_file_set_integer (keyfile, "position", "GAP_Y",cid->iPosY);
	g_key_file_set_double (keyfile, "position", "ROTATION",(cid->dRotate));
	g_key_file_set_double_list (keyfile, "position", "COLOR", (cid->dColor), cid->gColorSize);
	g_key_file_set_double_list (keyfile, "position", "FLYING_COLOR", (cid->dFlyingColor), cid->gFlyingColorSize);
	g_key_file_set_boolean (keyfile, "position", "KEEP_CORNERS", cid->bKeepCorners);
	g_key_file_set_boolean (keyfile, "position", "ALL_DESKTOP", cid->bAllDesktop);

	cid_write_keys_to_file (keyfile, cid->pConfFile);
}

void cid_write_keys_to_file (GKeyFile *pKeyFile, const gchar *cConfFilePath) {
	cid_debug ("%s (%s)", __func__, cConfFilePath);
	GError *erreur = NULL;

	gchar *cDirectory = g_path_get_dirname (cConfFilePath);
	if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
		g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
	}
	g_free (cDirectory);


	gsize length;
	gchar *cNewConfFilePath = g_key_file_to_data (pKeyFile, &length, &erreur);
	if (erreur != NULL) {
		cid_warning ("Error while fetching data : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}

	g_file_set_contents (cConfFilePath, cNewConfFilePath, length, &erreur);
	if (erreur != NULL) {
		cid_warning ("Error while writing data : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
}

void cid_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList) {
	g_slist_foreach (pWidgetList, (GFunc) _cid_get_each_widget_value, pKeyFile);
}

void _cid_get_each_widget_value (gpointer *data, GKeyFile *pKeyFile) {
	gchar *cGroupName = data[0];
	gchar *cKeyName = data[1];
	GSList *pSubWidgetList = data[2];
	GSList *pList;
	gsize i = 0, iNbElements = g_slist_length (pSubWidgetList);
	GtkWidget *pOneWidget = pSubWidgetList->data;

	if (GTK_IS_CHECK_BUTTON (pOneWidget))
	{
		gboolean *tBooleanValues = g_new0 (gboolean, iNbElements);
		for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
		{
			pOneWidget = pList->data;
			tBooleanValues[i] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pOneWidget));
			i ++;
		}
		if (iNbElements > 1)
			g_key_file_set_boolean_list (pKeyFile, cGroupName, cKeyName, tBooleanValues, iNbElements);
		else
			g_key_file_set_boolean (pKeyFile, cGroupName, cKeyName, tBooleanValues[0]);
		g_free (tBooleanValues);
	}
	else if (GTK_IS_SPIN_BUTTON (pOneWidget) || GTK_IS_HSCALE (pOneWidget))
	{
		gboolean bIsSpin = GTK_IS_SPIN_BUTTON (pOneWidget);
		
		if ((bIsSpin && gtk_spin_button_get_digits (GTK_SPIN_BUTTON (pOneWidget)) == 0) || (! bIsSpin && gtk_scale_get_digits (GTK_SCALE (pOneWidget)) == 0))
		{
			int *tIntegerValues = g_new0 (int, iNbElements);
			for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
			{
				pOneWidget = pList->data;
				tIntegerValues[i] = (bIsSpin ? gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (pOneWidget)) : gtk_range_get_value (GTK_RANGE (pOneWidget)));
				i ++;
			}
			if (iNbElements > 1)
				g_key_file_set_integer_list (pKeyFile, cGroupName, cKeyName, tIntegerValues, iNbElements);
			else
				g_key_file_set_integer (pKeyFile, cGroupName, cKeyName, tIntegerValues[0]);
			g_free (tIntegerValues);
		}
		else
		{
			double *tDoubleValues = g_new0 (double, iNbElements);
			for (pList = pSubWidgetList; pList != NULL; pList = pList->next)
			{
				pOneWidget = pList->data;
				tDoubleValues[i] = (bIsSpin ? gtk_spin_button_get_value (GTK_SPIN_BUTTON (pOneWidget)) : gtk_range_get_value (GTK_RANGE (pOneWidget)));
				i ++;
			}
			if (iNbElements > 1)
				g_key_file_set_double_list (pKeyFile, cGroupName, cKeyName, tDoubleValues, iNbElements);
			else
				g_key_file_set_double (pKeyFile, cGroupName, cKeyName, tDoubleValues[0]);
			g_free (tDoubleValues);
		}
	}
	else if (GTK_IS_COMBO_BOX (pOneWidget))
	{
		GtkTreeIter iter;
		gchar *cValue =  NULL;
		if (GTK_IS_COMBO_BOX_ENTRY (pOneWidget))
		{
			cValue = gtk_combo_box_get_active_text (GTK_COMBO_BOX (pOneWidget));
		}
		else if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (pOneWidget), &iter))
		{
			GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (pOneWidget));
			if (model != NULL)
				gtk_tree_model_get (model, &iter, CID_MODEL_RESULT, &cValue, -1);
		}
		g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (cValue != NULL ? cValue : "\0"));
		g_free (cValue);
	}
	else if (GTK_IS_ENTRY (pOneWidget))
	{
		const gchar *cValue = gtk_entry_get_text (GTK_ENTRY (pOneWidget));
		const gchar* const * cPossibleLocales = g_get_language_names ();
		gchar *cKeyNameFull, *cTranslatedValue;
		while (cPossibleLocales[i] != NULL)  // g_key_file_set_locale_string ne marche pas avec une locale NULL comme le fait 'g_key_file_get_locale_string', il faut donc le faire a la main !
		{
			cKeyNameFull = g_strdup_printf ("%s[%s]", cKeyName, cPossibleLocales[i]);
			cTranslatedValue = g_key_file_get_string (pKeyFile, cGroupName, cKeyNameFull, NULL);
			g_free (cKeyNameFull);
			if (cTranslatedValue != NULL && strcmp (cTranslatedValue, "") != 0)
				{
				g_free (cTranslatedValue);
				break;
				}
			g_free (cTranslatedValue);
			i ++;
		}
		if (cPossibleLocales[i] != NULL)
			g_key_file_set_locale_string (pKeyFile, cGroupName, cKeyName, cPossibleLocales[i], (cValue!=NULL ? cValue : "\0"));
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (cValue!=NULL ? cValue : "\0"));
	}
	else if (GTK_IS_TREE_VIEW (pOneWidget))
	{
		GtkTreeModel *pModel = gtk_tree_view_get_model (GTK_TREE_VIEW (pOneWidget));
		GSList *pActiveElementList = NULL;
		gtk_tree_model_foreach (GTK_TREE_MODEL (pModel), (GtkTreeModelForeachFunc) _cid_get_active_elements, &pActiveElementList);

		iNbElements = g_slist_length (pActiveElementList);
		gchar **tStringValues = g_new0 (gchar *, iNbElements + 1);

		i = 0;
		GSList * pListElement;
		for (pListElement = pActiveElementList; pListElement != NULL; pListElement = pListElement->next)
		{
			tStringValues[i] = pListElement->data;
			i ++;
		}
		if (iNbElements > 1)
			g_key_file_set_string_list (pKeyFile, cGroupName, cKeyName, (const gchar * const *)tStringValues, iNbElements);
		else
			g_key_file_set_string (pKeyFile, cGroupName, cKeyName, (tStringValues[0] != NULL ? tStringValues[0] : ""));
		g_slist_free (pActiveElementList);  // ses donnees sont dans 'tStringValues' et seront donc liberees avec.
		g_strfreev (tStringValues);
	}
}
