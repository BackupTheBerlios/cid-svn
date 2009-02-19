/*
   *
   *                             cid-config.c
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid.h"
#include <math.h>

static gint iNbRead=0;

void cid_check_file (const gchar *f) {
	gchar *cCommand=NULL;
	gchar *cFileName = cid->bTesting && !cid->bDevMode ? g_strdup(SVN_CONF_FILE) : g_strdup(CID_CONFIG_FILE);
	if (!g_file_test (f, G_FILE_TEST_EXISTS)) {
		cCommand = g_strdup_printf("mkdir -p %s/.config/cid > /dev/null",g_getenv("HOME"));
		system (cCommand);
		g_free (cCommand);
		cCommand = NULL;
		if (g_file_test (g_strdup_printf("%s/%s",g_getenv("HOME"),OLD_CONFIG_FILE), G_FILE_TEST_EXISTS))
			cCommand = g_strdup_printf("mv %s/%s %s/.config/cid/%s",
										g_getenv("HOME"),
										OLD_CONFIG_FILE,
										g_getenv("HOME"),
										cFileName);
		else if (g_file_test (g_strdup_printf("%s/.config/%s",g_getenv("HOME"),OLD_CONFIG_FILE), G_FILE_TEST_EXISTS))
			cCommand = g_strdup_printf("mv %s/.config/%s %s/.config/cid/%s",
										g_getenv("HOME"),
										OLD_CONFIG_FILE,
										g_getenv("HOME"),
										cFileName);
		else if (g_file_test (g_strdup_printf("%s/.config/%s",g_getenv("HOME"),cFileName), G_FILE_TEST_EXISTS))
			cCommand = g_strdup_printf("mv %s/.config/%s %s/.config/cid/%s",
										g_getenv("HOME"),
										cFileName,
										g_getenv("HOME"),
										cFileName);
		else 
			cCommand = g_strdup_printf ("cp %s/%s %s", CID_DATA_DIR, CID_CONFIG_FILE, cid->pConfFile);
		cid_info ("Commande: %s\n", cCommand);
		system (cCommand);
		g_free (cCommand);
		g_free (cFileName);
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
		cCommand = g_strdup_printf("rm -rf %s > /dev/null",f);
		system (cCommand);
		g_free (cCommand);
		cCommand = g_strdup_printf ("cp %s/%s %s", CID_DATA_DIR, CID_CONFIG_FILE, cid->pConfFile);
		system (cCommand);
		g_free (cCommand);
		
		cid_save_data ();
		cid_read_key_file (cid->pConfFile);
		return FALSE;
	}
	return TRUE;
}

void cid_read_config_after_update (const char *f, gpointer *pData) {
	cid_read_config (f, NULL);
	
	if ((cid->bChangedTestingConf != (cid->bUnstable && cid->bTesting))) {
		cid->bChangedTestingConf = cid->bTesting && cid->bUnstable;
		gtk_widget_destroy(cid->cWindow);
		cid_create_main_window();
	}
	
	cid_disconnect_player ();
	
	cid_free_musicData();
	
	if (cid->pMonitorList) 
		g_slice_free (CidControlFunctionsList,cid->pMonitorList);
	
	
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

gboolean cid_load_key_file(const gchar *cFile) {
	GKeyFileFlags flags;
	GError *error = NULL;

	/* Create a new GKeyFile object and a bitwise list of flags. */
	cid->pKeyFile = g_key_file_new ();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load the GKeyFile from keyfile.conf or return. */
	if (!g_key_file_load_from_file (cid->pKeyFile, cFile, flags, &error)) {
		g_error (error->message);
		return FALSE;
	}
	return TRUE;
}

void cid_key_file_free(void) {
	g_key_file_free (cid->pKeyFile);
}

gboolean cid_get_boolean_value_full (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault) {
	GError *error = NULL;
	gboolean bGet = g_key_file_get_boolean (pKeyFile, cGroup, cKey, &error);
	if (error != NULL) {
		g_error_free(error);
		error = NULL;
		bGet = bDefault;
	}
	return bGet;
}

gchar *cid_get_string_value_full (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gchar *cDefault, gboolean bFile, gboolean bDir) {
	GError *error = NULL;
	gchar *cGet = g_key_file_get_string (pKeyFile, cGroup, cKey, &error);
	if (error != NULL && bDefault) {
		g_error_free(error);
		error = NULL;
		cGet = cDefault;
	}
	if ((bFile || bDir) && cDefault != NULL && !g_file_test (cGet, bDir ? G_FILE_TEST_IS_DIR : G_FILE_TEST_EXISTS)) {
		g_free (cGet);
		if (g_file_test (cDefault, bDir ? G_FILE_TEST_IS_DIR : G_FILE_TEST_EXISTS))
			return cDefault;
		else 
			return NULL;
	}		
	return cGet;
}

gint cid_get_int_value_full (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gint iDefault, gboolean bMax, gint iMax) {
	GError *error = NULL;
	gint iGet = g_key_file_get_integer (pKeyFile, cGroup, cKey, &error);
	if (error != NULL && bDefault) {
		g_error_free(error);
		error = NULL;
		iGet = iDefault;
	}
	if (bMax && iGet > iMax)
		return iMax;
	return iGet;
}

void cid_read_key_file (const gchar *f) {	
	if (!cid_load_key_file(f))
		cid_exit(CID_ERROR_READING_FILE,"Key File error");
	
	// [System] configuration
	cid->iPlayer        = CID_CONFIG_GET_INTEGER ("System", "PLAYER");
	cid->iInter         = CID_CONFIG_GET_INTEGER_WITH_DEFAULT ("System", "INTER", 5) * 1000;
	cid->bMonitorPlayer = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "MONITOR", TRUE);
	cid->bPlayerState   = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "STATE", TRUE);
	cid->bDisplayTitle  = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "TITLE", TRUE);
	cid->dPoliceSize    = g_key_file_get_double  (cid->pKeyFile, "System", "POLICE_SIZE", NULL);
	cid->dPoliceColor   = g_key_file_get_double_list (cid->pKeyFile, "System", "POLICE_COLOR", &cid->gPlainTextSize, NULL);
	cid->dOutlineTextColor = g_key_file_get_double_list (cid->pKeyFile, "System", "OUTLINE_COLOR", &cid->gOutlineTextSize, NULL);

	// [Options] configuration
	cid->bHide           = CID_CONFIG_GET_BOOLEAN ("Options", "HIDE");
	cid->pDefaultImage   = CID_CONFIG_GET_FILE_PATH  ("Options", "IMAGE", CID_DEFAULT_IMAGE);
	cid->bRunAnimation   = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Options", "ANIMATION", TRUE);
	cid->iAnimationType  = CID_CONFIG_GET_INTEGER ("Options", "ANIMATION_TYPE");
	cid->bThreaded       = CID_CONFIG_GET_BOOLEAN ("Options", "THREAD");
	cid->bDownload       = CID_CONFIG_GET_BOOLEAN ("Options", "DOWNLOAD");
	cid->iImageSize      = CID_CONFIG_GET_INTEGER ("Options", "D_SIZE");
	cid->iTimeToWait     = CID_CONFIG_GET_INTEGER_WITH_DEFAULT ("Options", "DELAY", 5);
	cid->bUnstable       = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Options", "B_UNSTABLE", TRUE);
	
	// [Behaviour] configuration
	cid->iPosX          = CID_CONFIG_GET_INTEGER ("Behaviour", "GAP_X");
	cid->iPosY          = CID_CONFIG_GET_INTEGER ("Behaviour", "GAP_Y");	
	cid->iWidth         = CID_CONFIG_GET_INTEGER_WITH_DEFAULT_AND_MAX ("Behaviour", "SIZE_X", 150, 175);
	cid->iHeight        = CID_CONFIG_GET_INTEGER_WITH_DEFAULT_AND_MAX ("Behaviour", "SIZE_Y", 150, 175);
	cid->dRotate        = g_key_file_get_double  (cid->pKeyFile, "Behaviour", "ROTATION", NULL);
	cid->dColor         = g_key_file_get_double_list (cid->pKeyFile, "Behaviour", "COLOR", &cid->gColorSize, NULL);
	cid->dFlyingColor   = g_key_file_get_double_list (cid->pKeyFile, "Behaviour", "FLYING_COLOR", &cid->gFlyingColorSize, NULL);
	cid->bKeepCorners   = CID_CONFIG_GET_BOOLEAN ("Behaviour", "KEEP_CORNERS");
	cid->bAllDesktop    = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "ALL_DESKTOP", TRUE);
	cid->bLockPosition  = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "LOCK", TRUE);
	cid->bMask          = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "MASK", TRUE);
	
	cid->dRed            = cid->dColor[0];
	cid->dGreen          = cid->dColor[1];
	cid->dBlue           = cid->dColor[2];
	cid->dAlpha          = cid->dColor[3];
	cid->dFocusVariation = cid->dFlyingColor[3]>cid->dAlpha ? +.05 : -.05;
	
	
	cid_key_file_free();
}

int cid_read_config (const char *f, gpointer *pData) {
	cid_info ("Reading file : %s\n",f);
	
	if (!cid->bDevMode) 
		cid_check_file (f);
			
	cid_read_key_file (f);
	
	if (!cid->bDevMode) 
		cid_check_conf_file_version (f);
	
	iNbRead++;

	return 0;
}

void cid_get_data () {
	/* On récupère la position de cid */
	gtk_window_get_position(GTK_WINDOW (cid->cWindow), &cid->iPosX, &cid->iPosY);
	
	/* On récupère la taille de cid */
	gtk_window_get_size(GTK_WINDOW (cid->cWindow), &cid->iWidth, &cid->iHeight);
}

void cid_save_data () {
	if (!cid_load_key_file(cid->pConfFile))
		cid_exit(CID_ERROR_READING_FILE,"Key File error");
	
	if (cid->cWindow!=NULL)
		cid_get_data();
	
	// [System] configuration
	g_key_file_set_integer (cid->pKeyFile, "System", "PLAYER", cid->iPlayer);
	g_key_file_set_integer (cid->pKeyFile, "System", "INTER", cid->iInter/1000);
	g_key_file_set_boolean (cid->pKeyFile, "System", "MONITOR", cid->bMonitorPlayer);
	g_key_file_set_boolean (cid->pKeyFile, "System", "STATE", cid->bPlayerState);
	g_key_file_set_boolean (cid->pKeyFile, "System", "TITLE", cid->bDisplayTitle);
	g_key_file_set_double (cid->pKeyFile, "System", "POLICE_SIZE",(cid->dPoliceSize));
	g_key_file_set_double_list (cid->pKeyFile, "System", "POLICE_COLOR", (cid->dPoliceColor), cid->gPlainTextSize);
	g_key_file_set_double_list (cid->pKeyFile, "System", "OUTLINE_COLOR", (cid->dOutlineTextColor), cid->gOutlineTextSize);

	// [Options] configuration
	g_key_file_set_boolean (cid->pKeyFile, "Options", "ANIMATION", cid->bRunAnimation);
	g_key_file_set_boolean (cid->pKeyFile, "Options", "HIDE", cid->bHide);
	if (strcmp(cid->pDefaultImage,TESTING_COVER)!=0 && strcmp(cid->pDefaultImage,CID_DEFAULT_IMAGE)!=0)
		g_key_file_set_string  (cid->pKeyFile, "Options", "IMAGE", cid->pDefaultImage);
	else
		g_key_file_set_string  (cid->pKeyFile, "Options", "IMAGE", "");
	g_key_file_set_boolean (cid->pKeyFile, "Options", "THREAD", cid->bThreaded);
	g_key_file_set_boolean (cid->pKeyFile, "Options", "DOWNLOAD", cid->bDownload);
	g_key_file_set_integer (cid->pKeyFile, "Options", "ANIMATION_TYPE", cid->iAnimationType);
	g_key_file_set_integer (cid->pKeyFile, "Options", "DELAY", cid->iTimeToWait);
	g_key_file_set_integer (cid->pKeyFile, "Options", "D_SIZE", cid->iImageSize);
	g_key_file_set_boolean (cid->pKeyFile, "Options", "B_UNSTABLE", cid->bUnstable);
		
	// [Behaviour] configuration
	g_key_file_set_integer (cid->pKeyFile, "Behaviour", "SIZE_X",cid->iWidth);
	g_key_file_set_integer (cid->pKeyFile, "Behaviour", "SIZE_Y",cid->iHeight);
	g_key_file_set_integer (cid->pKeyFile, "Behaviour", "GAP_X",cid->iPosX);
	g_key_file_set_integer (cid->pKeyFile, "Behaviour", "GAP_Y",cid->iPosY);
	g_key_file_set_double (cid->pKeyFile, "Behaviour", "ROTATION",(cid->dRotate));
	g_key_file_set_double_list (cid->pKeyFile, "Behaviour", "COLOR", (cid->dColor), cid->gColorSize);
	g_key_file_set_double_list (cid->pKeyFile, "Behaviour", "FLYING_COLOR", (cid->dFlyingColor), cid->gFlyingColorSize);
	g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "KEEP_CORNERS", cid->bKeepCorners);
	g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "ALL_DESKTOP", cid->bAllDesktop);
	g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "LOCK", cid->bLockPosition);
	g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "MASK", cid->bMask);

	cid_write_keys_to_file (cid->pKeyFile, cid->pConfFile);
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
