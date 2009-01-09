/*
   *
   *                     cid-amarok.h
   *                       -------
   *                 Conky Images Display
   *              22/08/2008 - SANS Benjamin
   *             ----------------------------
   *
*/

#ifndef __CID_AMAROK__
#define  __CID_AMAROK__

#include <gtk/gtk.h>
G_BEGIN_DECLS

/**
 * connexion à amarok
 * @param iInter durée entre 2 interrogations
 */
void cid_connect_to_amarok(gint iInter);

/**
 * deconnexion d'amarok
 */
void cid_disconnect_from_amarok ();

/**
 * Ajoute les options de monitoring de amarok
 */
void cid_build_amarok_menu (void);

G_END_DECLS
#endif

