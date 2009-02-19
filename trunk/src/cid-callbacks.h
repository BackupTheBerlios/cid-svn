/*
   *
   *                            cid-callbacks.h
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/
#ifndef __CID_CALLBACKS__
#define __CID_CALLBACKS__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

G_BEGIN_DECLS

/**
 * Fonction exécutée à l'arret du programme 
 * @param p_widget non utilisé
 * @param user_data non utilisé
 */
void _cid_quit (GtkWidget *p_widget, gpointer user_data);

/**
 * Fonction exécutée en cas d'interruption du programme 
 * @param signal signal à l'origine de l'interruption
 */
void cid_interrupt (int signal);

/**
 * Fonction executée lors d'un clic de souris
 * @param p_widget objet cliqué
 * @param pButton evènement
 */
void on_clic (GtkWidget *p_widget, GdkEventButton* pButton);

/**
 * Fonction executée pour générer la page "À propos"
 * @param pMenuItem element déclencheur
 * @param data non utilisé
 */
void _cid_about (GtkMenuItem *pMenuItem, gpointer *data);

/**
 * Ajouter une page à la page "À propos"
 * @param pNoteBook sous-page
 * @param cPageLabel label
 * @param cAboutText contenu
 */
void _cid_add_about_page (GtkWidget *pNoteBook, const gchar *cPageLabel, const gchar *cAboutText);

/**
 * Fonction executée pour générer l'interface de configuration
 * @param pItemMenu element déclencheur
 * @param data non utilisé
 */
void _cid_conf_panel (GtkMenuItem *pItemMenu, gpointer *data);

/**
 * Fonction executée par le panneau de configuration
 * @param pDialog element déclencheur
 * @param action action sur la configuration
 * @param user_data données modifiées
 */
void _cid_user_action_on_config (GtkDialog *pDialog, gint action, gpointer *user_data);

/**
 * Vérifie si l'image à afficher existe
 * @param pointeur de données (non utilisé)
 * @return VRAI ou FAUX
 */
gboolean _check_cover_is_present (gpointer data);

/**
 * Lance le telechargement des pochettes
 */
gboolean _cid_proceed_download_cover (gpointer p);

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

gpointer _cid_launch_threaded (gchar *cCommand);

void _cid_select_one_item_in_combo (GtkComboBox *widget, gpointer *data);

void _cid_configure_renderer (GtkButton *button, gpointer *data);

void _cid_configure (GtkButton *button, gpointer *data);

gboolean _cid_select_one_item_in_tree (GtkTreeSelection * selection, GtkTreeModel * model, GtkTreePath * path, gboolean path_currently_selected, gpointer *data);

gboolean _cid_get_active_elements (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, GSList **pStringList);

void cid_free_generated_widget_list (GSList *pWidgetList);

G_END_DECLS
#endif
