/*
   *
   *                  cid-utilities.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_UTILITIES__
#define  __CID_UTILITIES__

#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include "cid-struct.h"

G_BEGIN_DECLS

#define _(string) gettext (string)

/**
 * Permet de lancer une commande en arrière plan
 * @param cCommand 'forme' de la commande
 * @param ... paramètres
 */
#define cid_launch_command(cCommand,...) cid_launch_command_full(cCommand, NULL, ##__VA_ARGS__)

/**
 * converti une chaine en son équivalent MAJUSCULE
 * @param cSrc chaine source
 * @return la chaine 'uppee'
 */
gchar *cid_toupper (gchar *cSrc);

/**
 * Permet de lancer une commande
 * @param cCommandFormat La commande à executer
 * @param cWorkingDirectory Le repertoire de travail
 * @param ... 
 */
gboolean cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...);
    
/**
 * sort en retournant le code de retour donné en paramètre
 * @param code de retour
 * @return code de retournant
 */
int cid_sortie(int code);

/**
 * lit les arguments du programme
 * @param nombre d'arguments
 * @param tableau de chaînes contenant la liste des arguments
 */
void cid_read_parameters (int argc, char **argv);

/**
 * définit la niveau de verbosité
 * @param niveau de verbosité
 */
void _cid_set_verbosity(gchar *cVerbosity);

/**
 * permet de jouer un son
 * @param cSoundPath chemin vers le son à jouer
 */
void cid_play_sound (const gchar *cSoundPath);

/**
 * Permet de déconnecter le monitoring du player
 */
void cid_disconnect_player ();

/**
 * Nettoyage des données musicales
 */
void cid_free_musicData(void);

/**
 * Nettoyage de notre structure principale
 */
void cid_free_main_structure (CidMainContainer *pCid);

/**
 * permet de jouer un son
 * @param cURL URL du site à afficher
 */
void cid_launch_web_browser (const gchar *cURL);

/**
 * permet de verifier que l'utilisateur entre des valeurs
 * correctes
 */
void cid_check_position (void);

/**
 * Fonction permettant de creer un tableau dynamique
 */
CidDataTable *cid_create_datatable (GType iDataType, ...);

void cid_delete_datatable(CidDataTable **p_list);
CidDataTable *cid_datatable_remove(CidDataTable *p_list, CidDataContent *data);
CidDataTable *cid_datatable_remove_all(CidDataTable *p_list, CidDataContent *data);
CidDataTable *cid_datatable_remove_id(CidDataTable *p_list, int position);
size_t cid_datatable_length(CidDataTable *p_list);
CidDataTable *cid_datatable_insert(CidDataTable *p_list, CidDataContent *data, int position);
CidDataTable *cid_datatable_prepend(CidDataTable *p_list, CidDataContent *data);
CidDataTable *cid_datatable_append(CidDataTable *p_list, CidDataContent *data);
CidDataContent *cid_datacontent_new (GType iType, void *value);
gboolean cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2);
void cid_datatable_foreach (CidDataTable *p_list, CidDataAction func);
void cid_datacase_print (CidDataCase *pCase);

#define cid_datacontent_new_string(val) cid_datacontent_new(G_TYPE_STRING, val)
#define cid_datacontent_new_int(val) cid_datacontent_new(G_TYPE_INT, val)
#define cid_datacontent_new_boolean(val) cid_datacontent_new(G_TYPE_BOOLEAN, val)

#define cid_datatable_remove_first(list) cid_datatable_remove_id(list, 1)
#define cid_datatable_remove_last(list) cid_datatable_remove_id(list, cid_datatable_length(list))

G_END_DECLS
#endif
