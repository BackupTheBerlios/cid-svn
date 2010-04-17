#ifndef __CID_MODULES__
#define  __CID_MODULES__

#include <glib.h>

#include "cid-struct.h"
G_BEGIN_DECLS


/**
*@file cairo-dock-modules.h This class defines and handles the external and internal modules of Cairo-Dock.
* A module has an interface and a visit card :
*  - the visit card allows it to define itself (name, category, default icon, etc)
*  - the interface defines the entry points for init, stop, reload, read config, and reset datas.
* Modules can be instanciated several times; each time they are, an instance is created. This instance will hold all the data used by the module's functions : the icon and its container, the config structure and its conf file, the data structure and a slot to plug datas into containers and icons. All these parameters are optionnal; a module that has an icon is also called an applet.
* Internal modules are just simplified version of modules, and are used internally by Cairo-Dock. As a special feature, a module can bind itself to an internal module, if its purpose is to complete it.
*/


/// Definition of the visit card of a module.
struct _CidVisitCard {
    /// nom du module qui servira a l'identifier.
    gchar *cModuleName;
    /// Nom du domaine pour la traduction du module par 'gettext'.
    gchar *cGettextDomain;
    /// Version de cid pour laquelle a ete compilee le module.
    gchar *cVersionOnCompilation;
    /// version courante du module.
    gchar *cModuleVersion;
    /// repertoire du plug-in cote utilisateur.
    gchar *cUserDataDir;
    /// repertoire d'installation du plug-in.
    gchar *cShareDataDir;
    /// nom de son fichier de conf.
    gchar *cConfFileName;
    /// taille de la structure contenant la config du module.
    gint iSizeOfConfig;
    /// taille de la structure contenant les donnees du module.
    gint iSizeOfData;
    /// description et mode d'emploi succint.
    gchar *cDescription;
    /// auteur/pseudo
    gchar *cAuthor;
    /// nom d'un module interne auquel ce module se rattache, ou NULL si aucun.
    const gchar *cInternalModule;
    /// octets reserves pour preserver la compatibilite binaire lors de futurs ajouts sur l'interface entre plug-ins et dock.
    char reserved[8];
};

/// Definition of the interface of a module.
struct _CidModuleInterface {
    void        (* initModule)          (CidModuleInstance *pInstance, GKeyFile *pKeyFile);
    void        (* stopModule)          (CidModuleInstance *pInstance);
    gboolean    (* reloadModule)        (CidModuleInstance *pInstance, GKeyFile *pKeyFile);
    gboolean    (* read_conf_file)      (CidModuleInstance *pInstance, GKeyFile *pKeyFile);
    void        (* reset_config)        (CidModuleInstance *pInstance);
    void        (* reset_data)          (CidModuleInstance *pInstance);
    void        (* load_custom_widget)  (CidModuleInstance *pInstance, GKeyFile *pKeyFile);
    void        (* save_custom_widget)  (CidModuleInstance *pInstance, GKeyFile *pKeyFile);
};

/// Definition of an instance of a module.
struct _CidModuleInstance {
    CidModule *pModule;
    gchar *cConfFilePath;
    gint iSlotID;
    /**gpointer *myConfig;
    gpointer *myData;*/
};

/// Fill the visit card and the interface of a module.
typedef gboolean (* CidModulePreInit) (CidVisitCard *pVisitCard, CidModuleInterface *pInterface);

/// Definition of an external module.
struct _CidModule {
    /// chemin du .so
    gchar *cSoFilePath;
    /// structure du module, contenant les pointeurs vers les fonctions du module.
    GModule *pModule;
    /// fonctions d'interface du module.
    CidModuleInterface *pInterface;
    /// carte de visite du module.
    CidVisitCard *pVisitCard;
    /// chemin du fichier de conf du module.
    gchar *cConfFilePath;
    /// Heure de derniere (re)activation du module.
    gdouble fLastLoadingTime;
    /// Liste d'instance du plug-in.
    GList *pInstancesList;
};


typedef gpointer CidInternalModuleConfigPtr;
typedef gpointer CidInternalModuleDataPtr;
typedef void (* CidInternalModuleReloadFunc) (CidInternalModuleConfigPtr *pPrevConfig, CidInternalModuleConfigPtr *pNewConfig);
typedef gboolean (* CidInternalModuleGetConfigFunc) (GKeyFile *pKeyFile, CidInternalModuleConfigPtr *pConfig);
typedef void (* CidInternalModuleResetConfigFunc) (CidInternalModuleConfigPtr *pConfig);
typedef void (* CidInternalModuleResetDataFunc) (CidInternalModuleDataPtr *pData);

/// Definition of an internal module.
struct _CidInternalModule {
    //\_____________ Carte de visite.
    const gchar *cModuleName;
    const gchar *cDescription;
    const gchar *cIcon;
    const gchar *cTitle;
    gint iSizeOfConfig;
    gint iSizeOfData;
    const gchar **cDependencies;  // NULL terminated.
    //\_____________ Interface.
    CidInternalModuleReloadFunc reload;
    CidInternalModuleGetConfigFunc get_config;
    CidInternalModuleResetConfigFunc reset_config;
    CidInternalModuleResetDataFunc reset_data;
    //\_____________ Instance.
    CidInternalModuleConfigPtr pConfig;
    CidInternalModuleDataPtr pData;
    GList *pExternalModules;
};

void cid_initialize_module_manager (const gchar *cModuleDirPath);

/*
*Verifie que le fichier de conf d'un plug-in est bien present dans le repertoire utilisateur du plug-in, sinon le copie a partir du fichier de conf fournit lors de l'installation. Cree au besoin le repertoire utilisateur du plug-in.
*@param pVisitCard la carte de visite du module.
*@return Le chemin du fichier de conf en espace utilisateur, ou NULL si le fichier n'a pu etre ni trouve, ni cree.
*/
gchar *cid_check_module_conf_file (CidVisitCard *pVisitCard);

void cid_free_visit_card (CidVisitCard *pVisitCard);

/** Load a module into the table of modules. The module is opened and its visit card and interface are retrieved.
*@param cSoFilePath path to the .so file.
*@param erreur error set if something bad happens.
*@return the newly allocated module.
*/
CidModule * cid_load_module (gchar *cSoFilePath, GError **erreur);

/** Load all th emodules of a given folder.
*@param cModuleDirPath path to the a folder containing .so files.
*@param erreur error set if something bad happens.
*/
void cid_preload_module_from_directory (const gchar *cModuleDirPath, GError **erreur);



void cid_activate_modules_from_list (gchar **cActiveModuleList, double fTime);

void cid_deactivate_old_modules (double fTime);


void cid_free_module (CidModule *module);

GKeyFile *cid_pre_read_module_instance_config (CidModuleInstance *pInstance);

/** Create and initialize all the instances of a module.
*@param module the module to activate.
*@param erreur error set if something bad happens.
*/
void cid_activate_module (CidModule *module, GError **erreur);

/** Stop and destroy all the instances of a module.
*@param module the module to deactivate
*/
void cid_deactivate_module (CidModule *module);

void cid_reload_module_instance (CidModuleInstance *pInstance, gboolean bReloadAppletConf);

/** Reload all the instances of the module.
*@param module the module to reload
*@param bReloadAppletConf TRUE to reload the config of the instances before reloading them.
*/
void cid_reload_module (CidModule *module, gboolean bReloadAppletConf);


void cid_deactivate_all_modules (void);

void cid_activate_module_and_load (gchar *cModuleName);
void cid_deactivate_module_instance_and_unload (CidModuleInstance *pInstance);
void cid_deactivate_module_and_unload (const gchar *cModuleName);

void cid_configure_module_instance (GtkWindow *pParentWindow, CidModuleInstance *pModuleInstance, GError **erreur);
void cid_configure_inactive_module (GtkWindow *pParentWindow, CidModule *pModule);
void cid_configure_module (GtkWindow *pParentWindow, const gchar *cModuleName);

/** Get the module which has a given name.
*@param cModuleName the unique name of the module.
*/
CidModule *cid_find_module_from_name (const gchar *cModuleName);

CidModule *cid_foreach_module (GHRFunc pCallback, gpointer user_data);
CidModule *cid_foreach_module_in_alphabetical_order (GCompareFunc pCallback, gpointer user_data);


gchar *cid_list_active_modules (void);
void cid_update_conf_file_with_active_modules (void);


int cid_get_nb_modules (void);

void cid_update_module_instance_order (CidModuleInstance *pModuleInstance, double fOrder);


CidModuleInstance *cid_instanciate_module (CidModule *pModule, gchar *cConfFilePah);
void cid_free_module_instance (CidModuleInstance *pInstance);
void cid_stop_module_instance (CidModuleInstance *pInstance);
void cid_deinstanciate_module (CidModuleInstance *pInstance);

void cid_remove_module_instance (CidModuleInstance *pInstance);
void cid_add_module_instance (CidModule *pModule);

void cid_read_module_config (GKeyFile *pKeyFile, CidModuleInstance *pInstance);


gboolean cid_reserve_data_slot (CidModuleInstance *pInstance);
void cid_release_data_slot (CidModuleInstance *pInstance);

#define cid_get_icon_data(pIcon, pInstance) ((pIcon)->pDataSlot[pInstance->iSlotID])
#define cid_get_container_data(pContainer, pInstance) ((pContainer)->pDataSlot[pInstance->iSlotID])

#define cid_set_icon_data(pIcon, pInstance, pData) \
    (pIcon)->pDataSlot[pInstance->iSlotID] = pData
#define cid_set_container_data(pContainer, pInstance, pData) \
    (pContainer)->pDataSlot[pInstance->iSlotID] = pData


void cid_preload_internal_modules (GHashTable *pModuleTable);

void cid_reload_internal_module (CidInternalModule *pModule, const gchar *cConfFilePath);

CidInternalModule *cid_find_internal_module_from_name (const gchar *cModuleName);

gboolean cid_get_internal_module_config (CidInternalModule *pModule, GKeyFile *pKeyFile);

gboolean cid_get_global_config (GKeyFile *pKeyFile);


void cid_popup_module_instance_description (CidModuleInstance *pModuleInstance);


void cid_attach_to_another_module (CidVisitCard *pVisitCard, const gchar *cOtherModuleName);

#define cid_module_is_auto_loaded(pModule) (pModule->pInterface->initModule == NULL || pModule->pInterface->stopModule == NULL || pModule->pVisitCard->cInternalModule != NULL)


G_END_DECLS
#endif
