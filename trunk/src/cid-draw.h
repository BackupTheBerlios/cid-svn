/*
   *
   *                              cid-draw.h
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/
#ifndef __CID_GTK__
#define  __CID_GTK__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include <cairo.h>
//#include <librsvg/rsvg.h>
//#include <librsvg/rsvg-cairo.h>

G_BEGIN_DECLS

/**
 * affiche l'image dont le chemin
 * est passé en argument. 
 * @param URI de l'image à afficher
 */
void cid_display_image(gchar *image);

/**
 * boucle cid/gtk 
 * @param nombre d'argument de CID
 * @param liste des arguments de CID
 */
void cid_display_init(int argc, char **argv);

/**
 * crée une fenêtre, la déplace à la position
 * voulue, et la dimensionne 
 */
void cid_create_main_window();

/**
 * Crée un buffer de pixels à partir d'un
 * GtkWidget image 
 */
GdkPixbuf *cid_get_pixbuf (GtkWidget **imageWidget);

/**
 * Crée un GtkWidget image à partir de
 * l'URI de l'image
 */
GtkWidget *cid_get_image_widget(gchar **imageURI);

/**
 * determine si un gestionnaire de composite est présent 
 * @param fenêtre à évaluer
 * @param non utilisé
 * @param pointeur de données (non utilisé)
 */
void cid_set_colormap (GtkWidget *widget, GdkScreen *old_screen, gpointer userdata);

/**
 * renvoie une image à partir d'un buffer de pixels 
 * @param buffer de pixels
 * @return image à partir du buffer
 */
cairo_surface_t *cid_get_image_from_pixbuf (GdkPixbuf **pixbuf);

/** 
 * retourne une image aux dimensions voulues à partir de son URI.
 * @param cImagePath URI de l'image à charger.
 * @param iWidth Largeur.
 * @param iHeight Hauteur.
 * @return image.
 */
cairo_surface_t *cid_get_cairo_image (gchar *cImagePath, gdouble iWidth, gdouble iHeight);

/**
 * Fonction appelée pour dessiner la fenêtre
 * @param La fenêtre à dessiner
 * @param ?
 * @param non utilisé
 */
void cid_draw_window (GtkWidget *widget, GdkEventExpose *event, gpointer *userdata);

/**
 * On dessine l'etat du lecteur
 */
void cid_set_state_icon (void);

G_END_DECLS
#endif
