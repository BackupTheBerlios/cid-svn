/*
   *
   *                        cid-asynchrone.h
   *                             -------
   *                       Conky Images Display
   *             --------------------------------------------
   *
*/

#ifndef __CID_ASYNCHRONE__
#define  __CID_ASYNCHRONE__

#include <gtk/gtk.h>

typedef enum {
    CID_FREQUENCY_NORMAL = 0,
    CID_FREQUENCY_LOW,
    CID_FREQUENCY_VERY_LOW,
    CID_FREQUENCY_SLEEP,
    CID_NB_FREQUENCIES
} CidFrequencyState;

typedef void (* CidAquisitionTimerFunc ) (gpointer data);
typedef void (* CidReadTimerFunc ) (gpointer data);
typedef gboolean (* CidUpdateTimerFunc ) (gpointer data);
typedef struct {
    /// Sid du timer des mesures.
    gint iSidTimer;
    /// Sid du timer de fin de mesure.
    gint iSidTimerRedraw;
    /// Valeur atomique a 1 ssi le thread de mesure est en cours.
    gint iThreadIsRunning;
    /// mutex d'accessibilite a la structure des resultats.
    GMutex *pMutexData;
    /// fonction realisant l'acquisition des donnees. N'accede jamais a la structure des resultats.
    CidAquisitionTimerFunc acquisition;
    /// fonction realisant la lecture des donnees precedemment acquises; stocke les resultats dans la structures des resultats.
    CidReadTimerFunc read;
    /// fonction realisant la mise a jour de l'IHM en fonction des nouveaux resultats. Renvoie TRUE pour continuer, FALSE pour arreter.
    CidUpdateTimerFunc update;
    /// intervalle de temps en secondes, eventuellement nul pour une mesure unitaire.
    gint iCheckInterval;
    /// etat de la frequence des mesures.
    CidFrequencyState iFrequencyState;
    /// donnees passees en entree de chaque fonction.
    gpointer pUserData;
} CidMeasure;

/**
*Lance les mesures periodiques, prealablement preparee avec #cid_new_measure_timer. La 1ere iteration est executee immediatement. L'acquisition et la lecture des donnees est faite de maniere asynchrone (dans un thread secondaire), alors que le chargement des mesures se fait dans la boucle principale. La frequence est remise a son etat normal.
*@param pMeasureTimer la mesure periodique.
*/
void cid_launch_measure (CidMeasure *pMeasureTimer);
/**
*Idem que ci-dessus mais après un délai.
*@param pMeasureTimer la mesure periodique.
*@param fDelay délai en ms.
*/
void cid_launch_measure_delayed (CidMeasure *pMeasureTimer, double fDelay);
/**
*Cree une mesure periodique.
*@param iCheckInterval l'intervalle en s entre 2 mesures, eventuellement nul pour une mesure unitaire.
*@param acquisition fonction realisant l'acquisition des donnees. N'accede jamais a la structure des resultats.
*@param read fonction realisant la lecture des donnees precedemment acquises; stocke les resultats dans la structures des resultats.
*@param update fonction realisant la mise a jour de l'interface en fonction des nouveaux resultats, lus dans la structures des resultats.
*@param pUserData structure passee en entree des fonctions read et update.
*@return la mesure nouvellement allouee. A liberer avec #cid_free_measure_timer.
*/
CidMeasure *cid_new_measure_timer (int iCheckInterval, CidAquisitionTimerFunc acquisition, CidReadTimerFunc read, CidUpdateTimerFunc update, gpointer pUserData);
/**
*Stoppe les mesures. Si une mesure est en cours, le thread d'acquisition/lecture se terminera tout seul plus tard, et la mesure sera ignoree. On peut reprendre les mesures par un simple #cairo_dock_launch_measure. Ne doit _pas_ etre appelée durant la fonction 'read' ou 'update'; utiliser la sortie de 'update' pour cela.
*@param pMeasureTimer la mesure periodique.
*/
void cid_stop_measure_timer (CidMeasure *pMeasureTimer);
/**
*Stoppe et detruit une mesure periodique, liberant toutes ses ressources allouees.
*@param pMeasureTimer la mesure periodique.
*/
void cid_free_measure_timer (CidMeasure *pMeasureTimer);
/**
*Dis si une mesure est active, c'est a dire si elle est appelee periodiquement.
*@param pMeasureTimer la mesure periodique.
*@return TRUE ssi la mesure est active.
*/
gboolean cid_measure_is_active (CidMeasure *pMeasureTimer);
/**
*Dis si une mesure est en cours, c'est a dire si elle est soit dans le thread, soit en attente d'update.
*@param pMeasureTimer la mesure periodique.
*@return TRUE ssi la mesure est en cours.
*/
gboolean cid_measure_is_running (CidMeasure *pMeasureTimer);
/**
*Change la frequence des mesures. La prochaine mesure aura lien dans 1 iteration si elle etait deja active.
*@param pMeasureTimer la mesure periodique.
*@param iNewCheckInterval le nouvel intervalle entre 2 mesures, en s.
*/
void cid_change_measure_frequency (CidMeasure *pMeasureTimer, int iNewCheckInterval);
/**
*Change la frequence des mesures et les relance immediatement. La prochaine mesure est donc tout de suite.
*@param pMeasureTimer la mesure periodique.
*@param iNewCheckInterval le nouvel intervalle entre 2 mesures, en s.
*/
void cid_relaunch_measure_immediately (CidMeasure *pMeasureTimer, int iNewCheckInterval);

/**
*Degrade la frequence des mesures. La mesure passe dans un etat moins actif (typiquement utile si la mesure a echouee).
*@param pMeasureTimer la mesure periodique.
*/
void cid_downgrade_frequency_state (CidMeasure *pMeasureTimer);
/**
*Remet la frequence des mesures a un etat normal. Notez que cela est fait automatiquement au 1er lancement de la mesure.
*@param pMeasureTimer la mesure periodique.
*/
void cid_set_normal_frequency_state (CidMeasure *pMeasureTimer);

#endif
