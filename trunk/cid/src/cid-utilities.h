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
 * Permet de lancer une commande en arriere plan
 * @param cCommand 'forme' de la commande
 * @param ... paramètres
 */
#define cid_launch_command(cCommand,...) cid_launch_command_full(cCommand, NULL, ##__VA_ARGS__)

/**
 * Permet de copier un fichier
 * @param cSrc Le fichier source
 * @param cDst Le fichier destination
 */
void cid_copy_file (const gchar *cSrc, const gchar *cDst);

/**
 * converti une chaine en son equivalent MAJUSCULE
 * @param cSrc chaine source
 * @return la chaine 'uppee'
 */
gchar *cid_toupper (gchar *cSrc);

/**
 * Permet de lancer une commande
 * @param cCommandFormat La commande a executer
 * @param cWorkingDirectory Le repertoire de travail
 * @param ... 
 */
gboolean cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...);
    
/**
 * sort en retournant le code de retour donné en parametre
 * @param code de retour
 * @return code de retournant
 */
int cid_sortie(int code);

/**
 * lit les arguments du programme
 * @param nombre d'arguments
 * @param tableau de chaines contenant la liste des arguments
 */
void cid_read_parameters (int argc, char **argv);

/**
 * definit la niveau de verbosite
 * @param niveau de verbosite
 */
void _cid_set_verbosity(gchar *cVerbosity);

/**
 * permet de jouer un son
 * @param cSoundPath chemin vers le son à jouer
 */
void cid_play_sound (const gchar *cSoundPath);

/**
 * Permet de deconnecter le monitoring du player
 */
void cid_disconnect_player ();

/**
 * Nettoyage des donnees musicales
 */
void cid_free_musicData(void);

/**
 * Nettoyage de notre structure principale
 * @param pCid pointeur sur notre structure
 */
void cid_free_main_structure (CidMainContainer *pCid);

/**
 * permet de jouer un son
 * @param cURL URL du site a afficher
 */
void cid_launch_web_browser (const gchar *cURL);

/**
 * permet de verifier que l'utilisateur entre des valeurs
 * correctes
 */
void cid_check_position (void);

/**
 * Fonction permettant de creer un tableau dynamique
 * @param iDataType le type des donnees qui suivent
 * @param ... les données, pouvant etre des chaines, des booleens, des 
 * entiers, ou des GType. Il faut obligatoirement terminer par un
 * G_TYPE_INVALID
 * @return pointeur vers notre structure
 */
CidDataTable *cid_create_datatable (GType iDataType, ...);

/**
 * Fonction permettant de liberer notre liste
 * @param pointeur vers notre liste
 */
void cid_free_datatable(CidDataTable **p_list);

/**
 * Permet de supprimer le premier element 'data' de la liste 'p_list'
 * @param p_list liste de depart
 * @param data element a supprimer
 * @return nouvelle liste 
 */
void cid_datatable_remove(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet de supprimer tous les elements 'data' de la liste 'p_list'
 * @param p_list liste de depart
 * @param data element a supprimer
 * @return nouvelle liste
 */
void cid_datatable_remove_all(CidDataTable **p_list, CidDataContent *data);

/**
 * Supprime de la liste 'p_list' l'element situe a l'indice 'position'
 * @param p_list liste de depart
 * @param position indice de l'element a supprimer
 * @return nouvelle liste
 */
void cid_datatable_remove_id(CidDataTable **p_list, gint position);

/**
 * Permet de connaitre la taille d'une liste
 * @param p_list liste dont on souhaite connaitre la taille
 * @return taille de la liste
 */
size_t cid_datatable_length(CidDataTable *p_list);

/**
 * Permet d'inserer un element a l'indice souhaite
 * @param p_list liste dans laquelle on souhaite ajouter l'element
 * @param data element a ajouter
 * @param position indice ou l'on souhaite ajouter l'element
 * @return nouvelle liste
 */
void cid_datatable_insert(CidDataTable **p_list, CidDataContent *data, gint position);

/**
 * Permet d'ajouter un element en debut de liste
 * @param p_list liste a laquelle on souhaite ajouter un element
 * @param data element a ajouter
 * @return nouvelle liste
 */
void cid_datatable_prepend(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet d'ajouter un element en fin de liste
 * @param p_list liste a laquelle on souhaite ajouter un element
 * @param data element a ajouter
 * @return nouvelle liste
 */
void cid_datatable_append(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet de creer un nouvel element pouvant etre insere dans une liste
 * @param iType type de l'element
 * @param value valeur de l'element
 * @return nouvel element
 */
CidDataContent *cid_datacontent_new (GType iType, void *value);

/**
 * Permet de comparer deux elements
 * @param d1 element 1
 * @param d2 element 2
 * @return TRUE si les elements sont egaux (en terme de valeur), sinon FALSE 
 */
gboolean cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2);

/**
 * Permet de parcourir tous les elements d'une liste donnee, et d'effectue le traitement voulu
 * @param p_list liste que l'on souhaite parcourir
 * @param func pointeur vers la fonction que l'on souhaite effectuer sur chaque element de la liste
 * @param pData donnees a transmettre a la fonction func
 * /!\ pData[0] contiendra toujours l'indice de l'element courant
 */
void cid_datatable_foreach (CidDataTable *p_list, CidDataAction func, gpointer *pData);

/**
 * Permet d'afficher la valeur d'un element
 * @param pCase element dont on souhaite afficher la valeur
 * @param pData donnees provenant eventuellement de cid_datatable_foreach
 */
void cid_datacase_print (CidDataCase *pCase, gpointer *pData);

/**
 * Fonction utilisee pour le str_replace_all
 * @param pCase element courant de l'iteration
 * @param pData donnees a utiliser
 */
static void cid_datacase_replace (CidDataCase *pCase, gpointer *pData);

/**
 * Permet de supprimer un element
 * @param pCase element a supprimer
 * @param pData donnees provenant du foreach
 */
void cid_free_datacase_full (CidDataCase *pCase, gpointer *pData);

/**
 * Permet de supprimer un conteneur
 * @param pContent contenu a supprimer
 * @param pData donnees provenant du foreach
 */
void cid_free_datacontent_full (CidDataContent *pContent, gpointer *pData);

#define cid_free_datacase(val) cid_free_datacase_full(val, NULL)

#define cid_free_datacontent(val) cid_free_datacontent_full(val, NULL)

#define cid_datacontent_new_string(val) cid_datacontent_new(G_TYPE_STRING, (void*)val)
#define cid_datacontent_new_int(val) cid_datacontent_new(G_TYPE_INT, (void*)val)
#define cid_datacontent_new_boolean(val) cid_datacontent_new(G_TYPE_BOOLEAN, (void*)val)

#define cid_datatable_remove_first(list) cid_datatable_remove_id(list, 1)
#define cid_datatable_remove_last(list) cid_datatable_remove_id(list, cid_datatable_length(list))

/**
 * Permet de remplacer tous les motifs d'une chaine donnee
 * @param string chaine que l'on souhaite modifier
 * @param sFrom motif que l'on souhaite remplacer
 * @param sTo motif avec lequel on remplace
 */
void cid_str_replace_all (gchar **string, const gchar *sFrom, const gchar *sTo);

void cid_str_replace_all_seq (gchar **string, gchar *seqFrom, gchar *seqTo);

gchar *_url_encode (const gchar * str);

G_END_DECLS
#endif
