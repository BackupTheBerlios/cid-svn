/*
   *
   *                        cid-panel-callbacks.h
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/
#ifndef __CID_PANEL_CALLBACKS__
#define __CID_PANEL_CALLBACKS__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

G_BEGIN_DECLS

/**
 * Fonction executée par le panneau de configuration
 * @param pDialog element déclencheur
 * @param action action sur la configuration
 * @param user_data données modifiées
 */
void _cid_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data);

/******************************************************************************************************\
|*                  Fonctions necessaires à la génération du conf panel                               *|
\******************************************************************************************************/ 
void _cid_set_color (GtkColorButton *pColorButton, GSList *pWidgetList);

void _cid_get_current_color (GtkColorButton *pColorButton, GSList *pWidgetList);

gboolean _cid_find_iter_from_name (GtkListStore *pModele, gchar *cName, GtkTreeIter *iter);

gboolean _cid_test_one_name (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer *data);

void _cid_go_up (GtkButton *button, GtkTreeView *pTreeView);

void _cid_go_down (GtkButton *button, GtkTreeView *pTreeView);

void _cid_add (GtkButton *button, gpointer *data);

void _cid_remove (GtkButton *button, gpointer *data);

void _cid_pick_a_file (GtkButton *button, gpointer *data);

void _cid_play_a_sound (GtkButton *button, gpointer *data);

void _cid_set_font (GtkFontButton *widget, GtkEntry *pEntry);

void _cid_key_grab_clicked (GtkButton *button, gpointer *data);

void _cid_select_one_item_in_combo (GtkComboBox *widget, gpointer *data);

void _cid_configure_renderer (GtkButton *button, gpointer *data);

void _cid_configure (GtkButton *button, gpointer *data);

gboolean _cid_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, gpointer *data);

gboolean _cid_get_active_elements (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, GSList **pStringList);

void cid_free_generated_widget_list (GSList *pWidgetList);

void cid_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList);

void _cid_get_each_widget_value (gpointer *data, GKeyFile *pKeyFile);


G_END_DECLS
#endif
