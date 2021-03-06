/*
   *
   *                    cid-amarok2.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
#ifndef __CID_AMAROK_2__
#define  __CID_AMAROK_2__

#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

/**
 * Fonction appelée au lancement de cid
 * permettant d'effectuer la première 
 * recherche d'image 
 * @return URI de l'image à afficher
 */
gchar *cid_amarok_2_cover(void);

/**
 * Fonction permettant de se connecter 
 * au bus de amarok2 
 * @return VRAI ou FAUX
 */
gboolean amarok_2_dbus_connect_to_bus(void);

/** 
 * Fonction permettant de se déconnecter 
 * du bus de amarok2 
 */
void amarok_2_dbus_disconnect_from_bus (void);

/**
 * Fonction permettant de savoir si amarok2 
 * est lancé ou non 
 * @return VRAI ou FAUX en fonction
 */
gboolean dbus_detect_amarok_2(void);

/**
 * Test si amarok2 joue de la musique ou non 
 * @return VRAI ou FAUX
 */
gboolean amarok_2_getPlaying(void);

/**
 * renvoie l'URI du fichier en cours de lecture 
 * @return URI du fichier joué
 */
gchar *amarok_2_getPlayingUri(void);

/** 
 * récupère l'ensemble des informations disponibles 
 * sur le fichier joué 
 */
void am_getSongInfos(void);

/**
 * Fonction exécutée (automatiquement) au changement 
 * de morceau 
 * @param bus de connection
 * @param URI du fichier joué
 * @param pointeur de données (non utilisé)
 */
void am_onChangeSong(DBusGProxy *player_proxy, GHashTable *data_list, gpointer data);

/**
 * Fonction exécutée (automatiquement) au changement d'état Play/Pause 
 * @param bus de connection
 * @param flag on joue ou non
 * @param pointeur de données (non utilisé)
 */
void am_onChangeState(DBusGProxy *player_proxy,GValueArray *status, gpointer data);

/**
 * Fonction exécutée (automatiquement) à chaque changement d'URI 
 * du fichier image utilisé par amarok2 
 * @param bus de connection
 * @param URI de la nouvelle image
 * @param pointeur de données (non utilisé)
 */
void am_onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data);


/**
 * Permet d'ajouter des options de monitoring pour amarok2
 * @param menu
 */
void cid_build_amarok_2_menu (void);

G_END_DECLS
#endif
