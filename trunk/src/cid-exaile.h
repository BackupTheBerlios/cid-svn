/*
   *
   *                    cid-exaile.h
   *                       -------
   *                 Conky Images Display
   *              17/01/2009 - SANS Benjamin
   *             ----------------------------
   *
   *
*/
#ifndef __CID_EXAILE__
#define  __CID_EXAILE__

#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

/**
 * Fonction appelée au lancement de cid
 * permettant d'effectuer la première 
 * recherche d'image 
 * @return URI de l'image à afficher
 */
gboolean cid_exaile_cover(void);

/**
 * Fonction permettant de se connecter 
 * au bus de exaile 
 * @return VRAI ou FAUX
 */
gboolean exaile_dbus_connect_to_bus(void);

/** 
 * Fonction permettant de se déconnecter 
 * du bus de exaile 
 */
void exaile_dbus_disconnect_from_bus (void);

/**
 * Fonction permettant de savoir si exaile 
 * est lancé ou non 
 * @return VRAI ou FAUX en fonction
 */
gboolean dbus_detect_exaile(void);

/**
 * Test si exaile joue de la musique ou non 
 * @return VRAI ou FAUX
 */
gboolean exaile_getPlaying(void);

/** 
 * récupère l'ensemble des informations disponibles 
 * sur le fichier joué 
 */
void getExaileSongInfos(void);

/**
 * Permet d'ajouter des options de monitoring pour exaile
 * @param menu
 */
void cid_build_rhythmbox_menu (void);

G_END_DECLS
#endif
