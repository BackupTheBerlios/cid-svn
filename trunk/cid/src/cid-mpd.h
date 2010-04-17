/*
   *
   *                               cid-mpd.h
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#ifndef __CID_MPD__
#define  __CID_MPD__

#include "libmpdclient.h"
#include <gtk/gtk.h>
G_BEGIN_DECLS

/**
 * connexion à mpd
 * @param iInter durée entre 2 interrogations
 */
void cid_connect_to_mpd (gint iInter);

/**
 * deconnexion de mpd
 */
void cid_disconnect_from_mpd ();

/**
 * Ajoute les options de monitoring de mpd
 */
void cid_build_mpd_menu (void);

G_END_DECLS
#endif
