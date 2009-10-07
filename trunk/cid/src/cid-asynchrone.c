/*
   *
   *                        cid-asynchrone.c
   *                             -------
   *                       Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid-asynchrone.h"
#include "cid-messages.h"

///////////////
/// MEASURE ///
///////////////
#define cid_schedule_next_measure(pMeasureTimer) do {\
    if (pMeasureTimer->iSidTimer == 0 && pMeasureTimer->iCheckInterval)\
        pMeasureTimer->iSidTimer = g_timeout_add (pMeasureTimer->iCheckInterval, (GSourceFunc) _cid_timer, pMeasureTimer); } while (0)

#define cid_cancel_next_measure(pMeasureTimer) do {\
    if (pMeasureTimer->iSidTimer != 0) {\
        g_source_remove (pMeasureTimer->iSidTimer);\
        pMeasureTimer->iSidTimer = 0; } } while (0)
        
#define cid_perform_measure_update(pMeasureTimer) do {\
    gboolean bContinue = pMeasureTimer->update (pMeasureTimer->pUserData);\
    if (! bContinue) {\
        cid_cancel_next_measure (pMeasureTimer); }\
    else {\
        pMeasureTimer->iFrequencyState = CID_FREQUENCY_NORMAL;\
        cid_schedule_next_measure (pMeasureTimer); } } while (0)

static gboolean _cid_timer (CidMeasure *pMeasureTimer) {
    cid_launch_measure (pMeasureTimer);
    return TRUE;
}

static gpointer _cid_threaded_calculation (CidMeasure *pMeasureTimer) {
    //\_______________________ On obtient nos donnees.
    if (pMeasureTimer->acquisition != NULL)
        pMeasureTimer->acquisition (pMeasureTimer->pUserData);
    
    pMeasureTimer->read (pMeasureTimer->pUserData);
    
    //\_______________________ On indique qu'on a fini.
    g_atomic_int_set (&pMeasureTimer->iThreadIsRunning, 0);
    return NULL;
}

static gboolean _cid_check_for_redraw (CidMeasure *pMeasureTimer) {
    int iThreadIsRunning = g_atomic_int_get (&pMeasureTimer->iThreadIsRunning);
    if (! iThreadIsRunning) { // le thread a fini.
        //\_______________________ On met a jour avec ces nouvelles donnees et on lance/arrete le timer.
        cid_perform_measure_update (pMeasureTimer);
        
        pMeasureTimer->iSidTimerRedraw = 0;
        return FALSE;
    }
    return TRUE;
}

void cid_launch_measure (CidMeasure *pMeasureTimer) {
    g_return_if_fail (pMeasureTimer != NULL);
    if (pMeasureTimer->read == NULL) { // pas de thread, tout est dans la fonction d'update.
        cid_perform_measure_update (pMeasureTimer);
    } else if (g_atomic_int_compare_and_exchange (&pMeasureTimer->iThreadIsRunning, 0, 1)) { // il etait egal a 0, on lui met 1 et on lance le thread.
        GError *erreur = NULL;
        GThread* pThread = g_thread_create ((GThreadFunc) _cid_threaded_calculation, pMeasureTimer, FALSE, &erreur);
        if (erreur != NULL) { // on n'a pas pu lancer le thread.
            cid_warning (erreur->message);
            g_error_free (erreur);
            g_atomic_int_set (&pMeasureTimer->iThreadIsRunning, 0);
        }
        
        if (pMeasureTimer->iSidTimerRedraw == 0)
            pMeasureTimer->iSidTimerRedraw = g_timeout_add (MAX (150, MIN (0.15 * pMeasureTimer->iCheckInterval, 333)), (GSourceFunc) _cid_check_for_redraw, pMeasureTimer);
    } else if (pMeasureTimer->iSidTimerRedraw != 0) { // le thread est deja fini.
        if (pMeasureTimer->iSidTimerRedraw != 0) { // on etait en attente de mise a jour, on fait la mise a jour tout de suite.
            g_source_remove (pMeasureTimer->iSidTimerRedraw);
            pMeasureTimer->iSidTimerRedraw = 0;
            
            cid_perform_measure_update (pMeasureTimer);
        } else { // la mesure est au repos.
            pMeasureTimer->iFrequencyState = CID_FREQUENCY_NORMAL;
            cid_schedule_next_measure (pMeasureTimer);
        }
    }
}

static gboolean _cid_one_shot_timer (CidMeasure *pMeasureTimer) {
    pMeasureTimer->iSidTimerRedraw = 0;
    cid_launch_measure (pMeasureTimer);
    return FALSE;
}

void cid_launch_measure_delayed (CidMeasure *pMeasureTimer, double fDelay) {
    pMeasureTimer->iSidTimerRedraw = g_timeout_add (fDelay, (GSourceFunc) _cid_one_shot_timer, pMeasureTimer);
}

CidMeasure *cid_new_measure_timer (int iCheckInterval, CidAquisitionTimerFunc acquisition, CidReadTimerFunc read, CidUpdateTimerFunc update, gpointer pUserData) {
    CidMeasure *pMeasureTimer = g_new0 (CidMeasure, 1);
    //if (read != NULL || acquisition != NULL)
    //  pMeasureTimer->pMutexData = g_mutex_new ();
    pMeasureTimer->iCheckInterval = iCheckInterval;
    pMeasureTimer->acquisition = acquisition;
    pMeasureTimer->read = read;
    pMeasureTimer->update = update;
    pMeasureTimer->pUserData = pUserData;
    return pMeasureTimer;
}

static void _cid_pause_measure_timer (CidMeasure *pMeasureTimer) {
    if (pMeasureTimer == NULL)
        return ;
    
    cid_cancel_next_measure (pMeasureTimer);
    
    if (pMeasureTimer->iSidTimerRedraw != 0) {
        g_source_remove (pMeasureTimer->iSidTimerRedraw);
        pMeasureTimer->iSidTimerRedraw = 0;
    }
}

void cid_stop_measure_timer (CidMeasure *pMeasureTimer) {
    if (pMeasureTimer == NULL)
        return ;
    
    _cid_pause_measure_timer (pMeasureTimer);
    
    cid_debug ("***on attend que le thread termine...(%d)", g_atomic_int_get (&pMeasureTimer->iThreadIsRunning));
    while (g_atomic_int_get (&pMeasureTimer->iThreadIsRunning))
        g_usleep (10);
        ///gtk_main_iteration ();
    cid_debug ("***temine.");
}

void cid_free_measure_timer (CidMeasure *pMeasureTimer) {
    if (pMeasureTimer == NULL)
        return ;
    cid_stop_measure_timer (pMeasureTimer);
    
    if (pMeasureTimer->pMutexData != NULL)
        g_mutex_free (pMeasureTimer->pMutexData);
    g_free (pMeasureTimer);
}

gboolean cid_measure_is_active (CidMeasure *pMeasureTimer) {
    return (pMeasureTimer != NULL && pMeasureTimer->iSidTimer != 0);
}

gboolean cid_measure_is_running (CidMeasure *pMeasureTimer) {
    return (pMeasureTimer != NULL && pMeasureTimer->iSidTimerRedraw != 0);
}

static void _cid_restart_timer_with_frequency (CidMeasure *pMeasureTimer, int iNewCheckInterval) {
    gboolean bNeedsRestart = (pMeasureTimer->iSidTimer != 0);
    _cid_pause_measure_timer (pMeasureTimer);
    
    if (bNeedsRestart && iNewCheckInterval != 0)
        pMeasureTimer->iSidTimer = g_timeout_add (iNewCheckInterval, (GSourceFunc) _cid_timer, pMeasureTimer);
}

void cid_change_measure_frequency (CidMeasure *pMeasureTimer, int iNewCheckInterval) {
    g_return_if_fail (pMeasureTimer != NULL);
    pMeasureTimer->iCheckInterval = iNewCheckInterval;
    
    _cid_restart_timer_with_frequency (pMeasureTimer, iNewCheckInterval);
}

void cid_relaunch_measure_immediately (CidMeasure *pMeasureTimer, int iNewCheckInterval) {
    cid_stop_measure_timer (pMeasureTimer);  // on stoppe avant car on ne veut pas attendre la prochaine iteration.
    if (iNewCheckInterval == -1)  // valeur inchangee.
        iNewCheckInterval = pMeasureTimer->iCheckInterval;
    cid_change_measure_frequency (pMeasureTimer, iNewCheckInterval); // nouvelle frequence eventuelement.
    cid_launch_measure (pMeasureTimer);  // mesure immediate.
}

void cid_downgrade_frequency_state (CidMeasure *pMeasureTimer) {
    if (pMeasureTimer->iFrequencyState < CID_FREQUENCY_SLEEP) {
        pMeasureTimer->iFrequencyState ++;
        int iNewCheckInterval;
        switch (pMeasureTimer->iFrequencyState) {
            case CID_FREQUENCY_LOW :
                iNewCheckInterval = 2 * pMeasureTimer->iCheckInterval;
            break ;
            case CID_FREQUENCY_VERY_LOW :
                iNewCheckInterval = 4 * pMeasureTimer->iCheckInterval;
            break ;
            case CID_FREQUENCY_SLEEP :
                iNewCheckInterval = 10 * pMeasureTimer->iCheckInterval;
            break ;
            default :  // ne doit pas arriver.
                iNewCheckInterval = pMeasureTimer->iCheckInterval;
            break ;
        }
        
        cid_message ("degradation de la mesure (etat <- %d/%d)", pMeasureTimer->iFrequencyState, CID_NB_FREQUENCIES-1);
        _cid_restart_timer_with_frequency (pMeasureTimer, iNewCheckInterval);
    }
}

void cid_set_normal_frequency_state (CidMeasure *pMeasureTimer) {
    if (pMeasureTimer->iFrequencyState != CID_FREQUENCY_NORMAL) {
        pMeasureTimer->iFrequencyState = CID_FREQUENCY_NORMAL;
        _cid_restart_timer_with_frequency (pMeasureTimer, pMeasureTimer->iCheckInterval);
    }
}
