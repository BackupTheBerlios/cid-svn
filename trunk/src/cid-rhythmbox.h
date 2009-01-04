/*
   *
   *                    cid-rhythmbox.h
   *                       -------
   *                 Conky Images Display
   *              22/08/2008 - SANS Benjamin
   *             ----------------------------
   *
   *
   *    Source originale: Cairo-dock
   *    Auteur origial: Adrien Pilleboue
*/
#ifndef __CID_RHYTHMBOX__
#define  __CID_RHYTHMBOX__

#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

/**
 * Fonction appelée au lancement de cid
 * permettant d'effectuer la première 
 * recherche d'image 
 * @return URI de l'image à afficher
 */
gchar *cid_rhythmbox_cover(void);

/**
 * Fonction permettant de se connecter 
 * au bus de rhythmbox 
 * @return VRAI ou FAUX
 */
gboolean rhythmbox_dbus_connect_to_bus(void);

/** 
 * Fonction permettant de se déconnecter 
 * du bus de rhythmbox 
 */
void rhythmbox_dbus_disconnect_from_bus (void);

/**
 * Fonction permettant de savoir si rhythmbox 
 * est lancé ou non 
 * @return VRAI ou FAUX en fonction
 */
gboolean dbus_detect_rhythmbox(void);

/**
 * Test si Rhythmbox joue de la musique ou non 
 * @return VRAI ou FAUX
 */
gboolean rhythmbox_getPlaying(void);

/**
 * renvoie l'URI du fichier en cours de lecture 
 * @return URI du fichier joué
 */
gchar *rhythmbox_getPlayingUri(void);

/** 
 * récupère l'ensemble des informations disponibles 
 * sur le fichier joué 
 */
void getSongInfos(void);

/**
 * Fonction exécutée (automatiquement) au changement 
 * de morceau 
 * @param bus de connection
 * @param URI du fichier joué
 * @param pointeur de données (non utilisé)
 */
void onChangeSong(DBusGProxy *player_proxy, const gchar *uri, gpointer data);

/**
 * Fonction exécutée (automatiquement) au changement d'état Play/Pause 
 * @param bus de connection
 * @param flag on joue ou non
 * @param pointeur de données (non utilisé)
 */
void onChangeState(DBusGProxy *player_proxy,gboolean playing, gpointer data);

/**
 * Fonction exécutée (automatiquement) à chaque changement de temps joué 
 * @param bus de connection
 * @param durée écoulée
 * @param pointeur de données (non utilisé)
 */
void onElapsedChanged(DBusGProxy *player_proxy,int elapsed, gpointer data);

/**
 * Fonction exécutée (automatiquement) à chaque changement d'URI 
 * du fichier image utilisé par rhythmbox 
 * @param bus de connection
 * @param URI de la nouvelle image
 * @param pointeur de données (non utilisé)
 */
void onCovertArtChanged(DBusGProxy *player_proxy,const gchar *cImageURI, gpointer data);


/**
 * Permet d'ajouter des options de monitoring pour rhythmbox
 * @param menu
 */
void cid_build_rhythmbox_menu (GtkWidget *pMenuItem, GtkWidget *menu);

G_END_DECLS
#endif
