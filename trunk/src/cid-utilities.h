/*
   *
   *                  cid-utilities.h
   *                      -------
   *                Conky Images Display
   *             05/10/2008 - Benjamin SANS
   *             --------------------------
   *
*/
#ifndef __CID_UTILITIES__
#define  __CID_UTILITIES__

G_BEGIN_DECLS

#define TESTING_COVER "../data/default.svg"
#define TESTING_FILE "../data/cid.conf"

#define cid_launch_command(cCommand,...) cid_launch_command_full(cCommand, NULL, ##__VA_ARGS__)

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
 * évalue le lecteur séléctionné
 * @param nom du lecteur
 * @return code du lecteur
 */
int cid_player_evaluation (gchar *player);

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
 * permet de jouer un son
 * @param cURL URL du site à afficher
 */
void cid_launch_web_browser (const gchar *cURL);

G_END_DECLS
#endif
