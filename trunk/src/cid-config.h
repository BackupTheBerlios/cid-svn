/*
   *
   *                             cid-config.h
   *                               -------
   *                         Conky Images Display
   *             16/10/2008 - Charlie MERLAND / Benjamin SANS
   *             --------------------------------------------
   *
*/
#ifndef __CID_CONFIG__
#define __CID_CONFIG__

#include "cid-struct.h"

G_BEGIN_DECLS

#define OLD_CONFIG_FILE ".cidrc"

gboolean cid_get_boolean_value (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault);

gchar *cid_get_string_value (GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gchar *cDefault);

#define CID_CONFIG_GET_BOOLEAN(cGroup,cKey,bDefault) cid_get_boolean_value (cid->pKeyFile,cGroup,cKey,bDefault)

#define CID_CONFIG_GET_STRING(cGroup,cKey,cDefault) cid_get_string_value (cid->pKeyFile,cGroup,cKey,cDefault)

/**
 * Fonction qui charge la configuration .
 * @param f fichier de configuration à lire.
 */
int cid_read_config (const char *f);

/**
 * vérifie que le fichier de configuration existe.
 * @param f fichier de configuration à lire.
 */
void cid_check_file (const char *f);

/**
 * fusionne les configurations entre les paramètres par défaut 
 * et le fichier de configurations.
 * @param conf le contenu du fichier de configurations.
 */
void cid_merge_config (FileSettings *conf);

/**
 * sauvegarde les données de cid
 */
void cid_save_data ();

/**
 * sauvegarde les clés dans le fichier de configurations.
 * @param pKeyFile pointeur de KeyFile.
 * @param cConfFilePath chemin du fichier à sauvegarder.
 */
void cid_write_keys_to_file (GKeyFile *pKeyFile, const gchar *cConfFilePath);

/**
 * vérifie la version du fichier de configuration.
 * @param f path du fichier à vérifier.
 */
gboolean cid_check_conf_file_version (const gchar *f);

void cid_update_keyfile_from_widget_list (GKeyFile *pKeyFile, GSList *pWidgetList);

void _cid_get_each_widget_value (gpointer *data, GKeyFile *pKeyFile);

void cid_read_config_after_update (const char *f, gpointer *pData);

void cid_free_conf (FileSettings *conf);

int cid_read_key_file (const gchar *f);

G_END_DECLS
#endif
