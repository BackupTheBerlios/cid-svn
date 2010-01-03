/*
   *
   *                             cid-modules.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid-modules.h"
#include "cid-messages.h"

static GHashTable *s_hModuleTable = NULL;
static GHashTable *s_hInternalModuleTable = NULL;
static GList *s_AutoLoadedModules = NULL;

void 
cid_initialize_module_manager (const gchar *cModuleDirPath)
{
    if (s_hModuleTable == NULL)
        s_hModuleTable = g_hash_table_new_full (g_str_hash,
            g_str_equal,
            NULL,  // la cle est le nom du module, et pointe directement sur le champ 'cModuleName' du module.
            (GDestroyNotify) cid_free_module);
    
    if (s_hInternalModuleTable == NULL)
    {
        s_hInternalModuleTable = g_hash_table_new_full (g_str_hash,
            g_str_equal,
            NULL,  // la cle est le nom du module, et pointe directement sur le champ 'cModuleName' du module.
            (GDestroyNotify) NULL);  // ne sont jamais liberes.
        //cid_preload_internal_modules (s_hInternalModuleTable);
    }
    
    if (cModuleDirPath != NULL && g_file_test (cModuleDirPath, G_FILE_TEST_IS_DIR))
    {
        GError *erreur = NULL;
        cid_preload_module_from_directory (cModuleDirPath, &erreur);
        if (erreur != NULL)
        {
            cid_warning ("%s\n  no module will be available", erreur->message);
            g_error_free (erreur);
        }
    }
/*
    //\________________ ceci est un vilain hack ... mais je trouvais ca lourd de compiler un truc qui n'a aucun code, et puis comme ca on a l'aide meme sans les plug-ins.
    CairoDockModule *pHelpModule = g_new0 (CairoDockModule, 1);
    CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
    pHelpModule->pVisitCard = pVisitCard;
    pVisitCard->cModuleName = g_strdup ("Help");
    pVisitCard->iMajorVersionNeeded = 2;
    pVisitCard->iMinorVersionNeeded = 0;
    pVisitCard->iMicroVersionNeeded = 0;
    pVisitCard->cPreviewFilePath = NULL;
    pVisitCard->cGettextDomain = NULL;
    pVisitCard->cDockVersionOnCompilation = g_strdup (CAIRO_DOCK_VERSION);
    pVisitCard->cUserDataDir = g_strdup ("help");
    pVisitCard->cShareDataDir = g_strdup (CAIRO_DOCK_SHARE_DATA_DIR);
    pVisitCard->cConfFileName = g_strdup ("help.conf");
    pVisitCard->cModuleVersion = g_strdup ("0.0.5");
    pVisitCard->iCategory = CAIRO_DOCK_CATEGORY_SYSTEM;
    pVisitCard->cIconFilePath = g_strdup_printf ("%s/%s", CAIRO_DOCK_SHARE_DATA_DIR, "help.svg");
    pVisitCard->iSizeOfConfig = 0;
    pVisitCard->iSizeOfData = 0;
    pVisitCard->cDescription = N_("A useful FAQ that contains also a lot of hints.\nLet the mouse over a sentence to make the hint dialog popups.");
    pHelpModule->pInterface = g_new0 (CairoDockModuleInterface, 1);
    g_hash_table_insert (s_hModuleTable, pHelpModule->pVisitCard->cModuleName, pHelpModule);
    ///pHelpModule->cConfFilePath = cairo_dock_check_module_conf_file (pHelpModule->pVisitCard);
    cairo_dock_activate_module (pHelpModule, NULL);
*/
}

static gchar *
cid_extract_default_module_name_from_path (gchar *cSoFilePath)
{
    gchar *ptr = g_strrstr (cSoFilePath, "/");
    if (ptr == NULL)
        ptr = cSoFilePath;
    else
        ptr ++;
    if (strncmp (ptr, "lib", 3) == 0)
        ptr += 3;

    if (strncmp (ptr, "cid-", 3) == 0)
        ptr += 3;
    else if (strncmp (ptr, "cid_", 3) == 0)
        ptr += 3;

    gchar *cModuleName = g_strdup (ptr);

    ptr = g_strrstr (cModuleName, ".so");
    if (ptr != NULL)
        *ptr = '\0';

    return cModuleName;
}

gchar *
cid_check_module_conf_file (CidVisitCard *pVisitCard)
{
    if (pVisitCard->cConfFileName == NULL)
        return NULL;
    
    int r;
    gchar *cUserDataDirPath = g_strdup_printf ("%s/plug-ins/%s", /*g_cCurrentThemePath*/CID_MODULES_DIR, pVisitCard->cUserDataDir);
    if (! g_file_test (cUserDataDirPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
    {
        cid_message ("directory %s doesn't exist, it will be added.", cUserDataDirPath);
        
        gchar *command = g_strdup_printf ("mkdir -p %s", cUserDataDirPath);
        r = system (command);
        g_free (command);
    }
    
    gchar *cConfFilePath = g_strdup_printf ("%s/%s", cUserDataDirPath, pVisitCard->cConfFileName);
    if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))
    {
        cid_message ("no conf file %s, we will take the default one", cConfFilePath);
        gchar *command = g_strdup_printf ("cp %s/%s %s", pVisitCard->cShareDataDir, pVisitCard->cConfFileName, cConfFilePath);
        r = system (command);
        g_free (command);
    }
    
    if (! g_file_test (cConfFilePath, G_FILE_TEST_EXISTS))  // la copie ne s'est pas bien passee.
    {
        cid_warning ("couldn't copy %s/%s in %s; check permissions and file's existence", pVisitCard->cShareDataDir, pVisitCard->cConfFileName, cUserDataDirPath);
        g_free (cUserDataDirPath);
        g_free (cConfFilePath);
        return NULL;
    }
    
    g_free (cUserDataDirPath);
    return cConfFilePath;
}

static void 
cid_open_module (CidModule *pCairoDockModule, GError **erreur)
{
    //\__________________ On ouvre le .so.
    GModule *module = g_module_open (pCairoDockModule->cSoFilePath, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
    if (!module)
    {
        g_set_error (erreur, 1, 1, "while opening module '%s' : (%s)", pCairoDockModule->cSoFilePath, g_module_error ());
        return ;
    }
    pCairoDockModule->pModule = module;

    //\__________________ On identifie le module.
    gboolean bSymbolFound;
    CidModulePreInit function_pre_init = NULL;
    bSymbolFound = g_module_symbol (module, "pre_init", (gpointer) &function_pre_init);
    if (bSymbolFound && function_pre_init != NULL)
    {
        pCairoDockModule->pVisitCard = g_new0 (CidVisitCard, 1);
        pCairoDockModule->pInterface = g_new0 (CidModuleInterface, 1);
        gboolean bModuleLoaded = function_pre_init (pCairoDockModule->pVisitCard, pCairoDockModule->pInterface);
        if (! bModuleLoaded)
        {
            cid_free_visit_card (pCairoDockModule->pVisitCard);
            pCairoDockModule->pVisitCard = NULL;
            cid_debug ("module '%s' has not been loaded", pCairoDockModule->cSoFilePath);  // peut arriver a xxx-integration.
            return ;
        }
    }
    else
    {
        pCairoDockModule->pVisitCard = NULL;
        g_set_error (erreur, 1, 1, "this module ('%s') does not have the common entry point 'pre_init', it may be broken or icompatible with cairo-dock", pCairoDockModule->cSoFilePath);
        return ;
    }
    
    //\__________________ On verifie sa compatibilite.
    CidVisitCard *pVisitCard = pCairoDockModule->pVisitCard;
    /*
    if (pVisitCard->iMajorVersionNeeded > g_iMajorVersion || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded > g_iMinorVersion) || (pVisitCard->iMajorVersionNeeded == g_iMajorVersion && pVisitCard->iMinorVersionNeeded == g_iMinorVersion && pVisitCard->iMicroVersionNeeded > g_iMicroVersion))
    {
        g_set_error (erreur, 1, 1, "this module ('%s') needs at least Cairo-Dock v%d.%d.%d, but Cairo-Dock is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->iMajorVersionNeeded, pVisitCard->iMinorVersionNeeded, pVisitCard->iMicroVersionNeeded, CAIRO_DOCK_VERSION);
        cairo_dock_free_visit_card (pCairoDockModule->pVisitCard);
        pCairoDockModule->pVisitCard = NULL;
        return ;
    }
    */
    /*
    if (pVisitCard->cDockVersionOnCompilation != NULL && strcmp (pVisitCard->cDockVersionOnCompilation, CID_VERSION) != 0)
    {
        g_set_error (erreur, 1, 1, "this module ('%s') was compiled with C.I.D. v%s, but C.I.D. is in v%s\n  It will be ignored", pCairoDockModule->cSoFilePath, pVisitCard->cDockVersionOnCompilation, CAIRO_DOCK_VERSION);
        cid_free_visit_card (pCairoDockModule->pVisitCard);
        pCairoDockModule->pVisitCard = NULL;
        return ;
    }
    */

    if (pVisitCard->cModuleName == NULL)
        pVisitCard->cModuleName = cid_extract_default_module_name_from_path (pCairoDockModule->cSoFilePath);

    if (cid_module_is_auto_loaded (pCairoDockModule))  // c'est un module qui soit ne peut etre activer et/ou desactiver, soit s'est lie a un module interne; on l'activera donc automatiquement.
    {
        s_AutoLoadedModules = g_list_prepend (s_AutoLoadedModules, pCairoDockModule);
    }
}

void 
cid_free_visit_card (CidVisitCard *pVisitCard)
{
    /*g_free (pVisitCard->cReadmeFilePath);
    g_free (pVisitCard->cPreviewFilePath);
    g_free (pVisitCard->cGettextDomain);
    g_free (pVisitCard->cDockVersionOnCompilation);
    g_free (pVisitCard->cModuleName);
    g_free (pVisitCard->cUserDataDir);
    g_free (pVisitCard->cShareDataDir);
    g_free (pVisitCard->cConfFileName);
    g_free (pVisitCard->cModuleVersion);
    g_free (pVisitCard->cIconFilePath);*/
    g_free (pVisitCard);  // toutes les chaines sont statiques.
}

static void 
cid_close_module (CidModule *module)
{
    g_module_close (module->pModule);
    
    g_free (module->pInterface);
    module->pInterface = NULL;
    
    cid_free_visit_card (module->pVisitCard);
    module->pVisitCard = NULL;
    
    g_free (module->cConfFilePath);
    module->cConfFilePath = NULL;
}

CidModule * 
cid_load_module (gchar *cSoFilePath, GError **erreur)  // cSoFilePath vers un fichier de la forme 'libtruc.so'. Le module est rajoute dans la table des modules.
{
    //g_print ("%s (%s)\n", __func__, cSoFilePath);
    if (cSoFilePath == NULL)  // g_module_open () plante si 'cSoFilePath' est NULL.
    {
        g_set_error (erreur, 1, 1, "%s () : no such module", __func__);
        return NULL;
    }

    CidModule *pCairoDockModule = g_new0 (CidModule, 1);
    pCairoDockModule->cSoFilePath = g_strdup (cSoFilePath);

    GError *tmp_erreur = NULL;
    cid_open_module (pCairoDockModule, &tmp_erreur);
    if (tmp_erreur != NULL)
    {
        g_propagate_error (erreur, tmp_erreur);
        g_free (pCairoDockModule);
        return NULL;
    }

    if (s_hModuleTable != NULL && pCairoDockModule->pVisitCard != NULL)
        g_hash_table_insert (s_hModuleTable, pCairoDockModule->pVisitCard->cModuleName, pCairoDockModule);

    return pCairoDockModule;
}

void 
cid_preload_module_from_directory (const gchar *cModuleDirPath, GError **erreur)
{
    cid_message ("%s (%s)", __func__, cModuleDirPath);
    GError *tmp_erreur = NULL;
    GDir *dir = g_dir_open (cModuleDirPath, 0, &tmp_erreur);
    if (tmp_erreur != NULL)
    {
        g_propagate_error (erreur, tmp_erreur);
        return ;
    }

    CidModule *pModule;
    const gchar *cFileName;
    GString *sFilePath = g_string_new ("");
    do
    {
        cFileName = g_dir_read_name (dir);
        if (cFileName == NULL)
            break ;
        
        if (g_str_has_suffix (cFileName, ".so"))
        {
            g_string_printf (sFilePath, "%s/%s", cModuleDirPath, cFileName);
            pModule = cid_load_module (sFilePath->str, &tmp_erreur);
            if (tmp_erreur != NULL)
            {
                cid_warning (tmp_erreur->message);
                g_error_free (tmp_erreur);
                tmp_erreur = NULL;
            }
        }
    }
    while (1);
    g_string_free (sFilePath, TRUE);
    g_dir_close (dir);
}

void cid_free_module (CidModule *module)
{
	if (module == NULL)
		return ;
	cid_debug ("%s (%s)", __func__, module->pVisitCard->cModuleName);

	cid_deactivate_module (module);

	cid_close_module (module);

	cid_free_visit_card (module->pVisitCard);
	g_free (module->cSoFilePath);
	g_free (module);
}

/*
#define REGISTER_INTERNAL_MODULE(cGroupName) \
	pModule = g_new0 (CidInternalModule, 1);\
	cid_pre_init_##cGroupName (pModule);\
	g_hash_table_insert (pModuleTable, (gpointer)pModule->cModuleName, (gpointer)pModule)
    
void 
cid_preload_internal_modules (GHashTable *pModuleTable)
{
	cd_message ("");
	CidInternalModule *pModule;
	
	REGISTER_INTERNAL_MODULE (Position);
	REGISTER_INTERNAL_MODULE (Accessibility);
	REGISTER_INTERNAL_MODULE (System);
	REGISTER_INTERNAL_MODULE (TaskBar);
	REGISTER_INTERNAL_MODULE (HiddenDock);
	REGISTER_INTERNAL_MODULE (Dialogs);
	REGISTER_INTERNAL_MODULE (Indicators);
	REGISTER_INTERNAL_MODULE (Views);
	REGISTER_INTERNAL_MODULE (Labels);
	REGISTER_INTERNAL_MODULE (Desklets);
	REGISTER_INTERNAL_MODULE (Icons);
	REGISTER_INTERNAL_MODULE (Background);
}
*/

void 
cid_deactivate_module (CidModule *module)
{
	g_return_if_fail (module != NULL);
	cid_debug ("%s (%s, %s)", __func__, module->pVisitCard->cModuleName, module->cConfFilePath);
	g_list_foreach (module->pInstancesList, (GFunc) cid_stop_module_instance, NULL);
	g_list_foreach (module->pInstancesList, (GFunc) cid_free_module_instance, NULL);
	g_list_free (module->pInstancesList);
	module->pInstancesList = NULL;
}

/**
* Stoppe une instance d'un module.
*/
void 
cid_stop_module_instance (CidModuleInstance *pInstance)
{
	if (pInstance->pModule->pInterface->stopModule != NULL)
		pInstance->pModule->pInterface->stopModule (pInstance);
	
	if (pInstance->pModule->pInterface->reset_data != NULL)
		pInstance->pModule->pInterface->reset_data (pInstance);
	
	if (pInstance->pModule->pInterface->reset_config != NULL)
		pInstance->pModule->pInterface->reset_config (pInstance);
}

/**
* Detruit une instance de module et libere les resources associees.
*/
void 
cid_free_module_instance (CidModuleInstance *pInstance)
{
	g_free (pInstance->cConfFilePath);
	/**g_free (pInstance->pConfig);
	g_free (pInstance->pData);*/
	g_free (pInstance);
}
