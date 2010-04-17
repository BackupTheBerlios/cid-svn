/*
   *
   *                 cid-applet-canvas.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/

#ifndef __CID_APPLET_CANVAS__
#define  __CID_APPLET_CANVAS__

#include <glib.h>

#include "cid-struct.h"
G_BEGIN_DECLS

/**
*@file cid-applet-canvas.h This file defines numerous macros, that form a canvas for all the applets.
* You probably won't need to dig into this file, since you can generate an applet with the 'generate-new-applet.sh' script, that will build the whole canvas for you.
* Moreover, you can have a look at an applet that has a similar functioning to yours.
*/

//\_________________________________ STRUCT
typedef struct _AppletConfig AppletConfig;
typedef struct _AppletData AppletData;

//\_________________________________ FUNCTIONS NAMES
#define CID_APPLET_DEFINE_FUNC pre_init
#define CID_APPLET_INIT_FUNC init
#define CID_APPLET_STOP_FUNC stop
#define CID_APPLET_RELOAD_FUNC reload

#define CID_APPLET_READ_CONFIG_FUNC read_conf_file
#define CID_APPLET_RESET_CONFIG_FUNC reset_config
#define CID_APPLET_RESET_DATA_FUNC reset_data

#define CID_APPLET_ON_CLICK_FUNC action_on_click
#define CID_APPLET_ON_BUILD_MENU_FUNC action_on_build_menu
#define CID_APPLET_ON_MIDDLE_CLICK_FUNC action_on_middle_click
#define CID_APPLET_ON_DOUBLE_CLICK_FUNC action_on_double_click
#define CID_APPLET_ON_DROP_DATA_FUNC action_on_drop_data
#define CID_APPLET_ON_SCROLL_FUNC action_on_scroll
#define CID_APPLET_ON_UPDATE_ICON_FUNC action_on_update_icon

//\_________________________________ PROTO
#define CID_APPLET_DEFINE_PROTO \
gboolean CID_APPLET_DEFINE_FUNC (CidVisitCard *pVisitCard, CidModuleInterface *pInterface)
#define CID_APPLET_INIT_PROTO(pApplet) \
void CID_APPLET_INIT_FUNC (CidModuleInstance *pApplet, GKeyFile *pKeyFile)
#define CID_APPLET_STOP_PROTO \
void CID_APPLET_STOP_FUNC (CidModuleInstance *myApplet)
#define CID_APPLET_RELOAD_PROTO \
gboolean CID_APPLET_RELOAD_FUNC (CidModuleInstance *myApplet, GKeyFile *pKeyFile)

#define CID_APPLET_READ_CONFIG_PROTO \
gboolean CID_APPLET_READ_CONFIG_FUNC (CidModuleInstance *myApplet, GKeyFile *pKeyFile)
#define CID_APPLET_RESET_CONFIG_PROTO \
void CID_APPLET_RESET_CONFIG_FUNC (CidModuleInstance *myApplet)
#define CID_APPLET_RESET_DATA_PROTO \
void CID_APPLET_RESET_DATA_FUNC (CidModuleInstance *myApplet)

//\_________________________________ HEADERS
#define CID_APPLET_H \
CID_APPLET_DEFINE_PROTO; \
CID_APPLET_INIT_PROTO (pApplet); \
CID_APPLET_STOP_PROTO; \
CID_APPLET_RELOAD_PROTO;

#define CID_APPLET_CONFIG_H \
CID_APPLET_READ_CONFIG_PROTO; \
CID_APPLET_RESET_CONFIG_PROTO; \
CID_APPLET_RESET_DATA_PROTO;

//\_________________________________ BODY
//\______________________ pre_init.
/** Debut de la fonction de pre-initialisation de l'applet (celle qui est appele a l'enregistrement de tous les plug-ins).
*Definit egalement les variables globales suivantes : myIcon, myDock, myDesklet, myContainer, et myDrawContext.
*@param _cName nom de sous lequel l'applet sera enregistree par Cairo-Dock.
*@param _iMajorVersion version majeure du dock necessaire au bon fonctionnement de l'applet.
*@param _iMinorVersion version mineure du dock necessaire au bon fonctionnement de l'applet.
*@param _iMicroVersion version micro du dock necessaire au bon fonctionnement de l'applet.
*@param _iAppletCategory Categorie de l'applet (CAIRO_DOCK_CATEGORY_ACCESSORY, CAIRO_DOCK_CATEGORY_DESKTOP, CAIRO_DOCK_CATEGORY_CONTROLER)
*@param _cDescription description et mode d'emploi succint de l'applet.
*@param _cAuthor nom de l'auteur et eventuellement adresse mail.
*/
#define CID_APPLET_DEFINE_ALL_BEGIN(_cName, _cDescription, _cAuthor) \
CID_APPLET_DEFINE_PROTO \
{ \
    pVisitCard->cModuleName = g_strdup (_cName); \
    pVisitCard->cGettextDomain = MY_APPLET_GETTEXT_DOMAIN; \
    pVisitCard->cUserDataDir = MY_APPLET_USER_DATA_DIR; \
    pVisitCard->cShareDataDir = MY_APPLET_SHARE_DATA_DIR; \
    pVisitCard->cConfFileName = ""; \
    pVisitCard->cModuleVersion = MY_APPLET_VERSION;\
    pVisitCard->iSizeOfConfig = sizeof (AppletConfig);\
    pVisitCard->iSizeOfData = sizeof (AppletData);\
    pVisitCard->cAuthor = _cAuthor;\
    pVisitCard->cDescription = _cDescription;

#define CID_APPLET_DEFINE_COMMON_APPLET_INTERFACE \
    pInterface->initModule = CID_APPLET_INIT_FUNC;\
    pInterface->stopModule = CID_APPLET_STOP_FUNC;\
    pInterface->reloadModule = CID_APPLET_RELOAD_FUNC;
    
/*
    pInterface->reset_config = CID_APPLET_RESET_CONFIG_FUNC;\
    pInterface->reset_data = CID_APPLET_RESET_DATA_FUNC;\
    pInterface->read_conf_file = CID_APPLET_READ_CONFIG_FUNC;
*/

/** Fin de la fonction de pre-initialisation de l'applet.
*/
#define CID_APPLET_DEFINE_END \
    return TRUE ;\
}
/** Fonction de pre-initialisation generique. Ne fais que definir l'applet (en appelant les 2 macros precedentes), la plupart du temps cela est suffisant.
*/
#define CID_APPLET_DEFINITION(cName, cDescription, cAuthor) \
CID_APPLET_DEFINE_BEGIN (cName, cDescription, cAuthor) \
CID_APPLET_DEFINE_COMMON_APPLET_INTERFACE \
CID_APPLET_DEFINE_END

#define CID_APPLET_ATTACH_TO_INTERNAL_MODULE(cInternalModuleName) cid_attach_to_another_module (pVisitCard, cInternalModuleName)

//\______________________ init.
/** Debut de la fonction d'initialisation de l'applet (celle qui est appelee a chaque chargement de l'applet).
*Lis le fichier de conf de l'applet, et cree son icone ainsi que son contexte de dessin.
*@param pApplet une instance du module.
*/
#define CID_APPLET_INIT_ALL_BEGIN(pApplet) \
CID_APPLET_INIT_PROTO (pApplet)\
{ \
    cid_message ("%s (%s)", __func__, pApplet->cConfFilePath);

/** Fin de la fonction d'initialisation de l'applet.
*/
#define CID_APPLET_INIT_END \
}

//\______________________ stop.
/** Debut de la fonction d'arret de l'applet.
*/
#define CID_APPLET_STOP_BEGIN \
CID_APPLET_STOP_PROTO \
{

/** Fin de la fonction d'arret de l'applet.
*/
#define CID_APPLET_STOP_END \
    cid_release_data_slot (myApplet); \
}

//\______________________ reload.
/** Debut de la fonction de rechargement de l'applet.
*/
#define CID_APPLET_RELOAD_ALL_BEGIN \
CID_APPLET_RELOAD_PROTO \
{ \
    cid_message ("%s (%s)\n", __func__, myApplet->cConfFilePath);

/** Fin de la fonction de rechargement de l'applet.
*/
#define CID_APPLET_RELOAD_END \
    return TRUE; \
}


//\______________________ read_conf_file.
/** Debut de la fonction de configuration de l'applet (celle qui est appelee au debut de l'init).
*/
#define CID_APPLET_GET_CONFIG_ALL_BEGIN \
CID_APPLET_READ_CONFIG_PROTO \
{ \
    gboolean bFlushConfFileNeeded = FALSE;

/** Fin de la fonction de configuration de l'applet.
*/
#define CID_APPLET_GET_CONFIG_END \
    return bFlushConfFileNeeded; \
}

//\______________________ reset_config.
/** Debut de la fonction de liberation des donnees de la config.
*/
#define CID_APPLET_RESET_CONFIG_ALL_BEGIN \
CID_APPLET_RESET_CONFIG_PROTO \
{
/** Fin de la fonction de liberation des donnees de la config.
*/
#define CID_APPLET_RESET_CONFIG_END \
}

//\______________________ reset_data.
/** Debut de la fonction de liberation des donnees internes.
*/
#define CID_APPLET_RESET_DATA_BEGIN \
CID_APPLET_RESET_DATA_PROTO \
{
/** Fin de la fonction de liberation des donnees internes.
*/
#define CID_APPLET_RESET_DATA_ALL_END \
}

#define CID_APPLET_DEFINE_BEGIN(cName, cDescription, cAuthor) \
AppletConfig *myConfigPtr = NULL; \
AppletData *myDataPtr = NULL; \
CidModuleInstance *myApplet = NULL; \
CID_APPLET_DEFINE_ALL_BEGIN (cName, cDescription, cAuthor)

#define CID_APPLET_INIT_BEGIN \
CID_APPLET_INIT_ALL_BEGIN(pApplet) \
myApplet = pApplet; 

#define myConfig (* myConfigPtr)
#define myData (* myDataPtr)


#define CID_APPLET_RELOAD_BEGIN \
    CID_APPLET_RELOAD_ALL_BEGIN


#define CID_APPLET_RESET_DATA_END \
    myConfigPtr = NULL; \
    memset (myDataPtr, 0, sizeof (AppletData)); \
    myDataPtr = NULL; \
    myApplet = NULL; \
    CID_APPLET_RESET_DATA_ALL_END


#define CID_APPLET_RESET_CONFIG_BEGIN \
    CID_APPLET_RESET_CONFIG_ALL_BEGIN \
    if (myConfigPtr == NULL) \
        return ;


#define CID_APPLET_GET_CONFIG_BEGIN \
    CID_APPLET_GET_CONFIG_ALL_BEGIN\
    if (myConfigPtr == NULL)\
        myConfigPtr = (((gpointer)myApplet)+sizeof(CidModuleInstance));\
    if (myDataPtr == NULL)\
        myDataPtr = (((gpointer)myConfigPtr)+sizeof(AppletConfig));


extern AppletConfig *myConfigPtr;
extern AppletData *myDataPtr;
extern CidModuleInstance *myApplet;

G_END_DECLS
#endif
