/*
   *
   *                             cid-animation.h
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/
#ifndef __CID_ANIMATION__
#define  __CID_ANIMATION__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

//#include <pthread.h>


#include "cid-struct.h"

#define CID_REDRAW gtk_widget_queue_draw(cid->cWindow);

/**
 * Fonction appelee au focus
 */
void cid_focus (GtkWidget *pWidget, GdkEventExpose *event, gpointer *userdata);

/**
 * Fonction appelée par cid_threaded_animation
 */
gboolean cid_rotate_on_changing_song (void *ptr);

/**
 * Fonction appelée pour animer CID
 */
void cid_animation (AnimationType iAnim);

/**
 * Fonction permettant de lancer une animation threadée
 */
void cid_threaded_animation (AnimationType iAnim, gint iDelay);

G_END_DECLS
#endif
