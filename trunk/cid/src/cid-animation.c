/*
   *
   *                             cid-animation.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid-animation.h"
#include "cid-messages.h"
#include "cid-asynchrone.h"
#include "cid-draw.h"
#include "cid-constantes.h"

extern CidMainContainer *cid;

static CidMeasure *pMeasureTimerAnimation = NULL;
static CidMeasure *pMeasureTimerFocus = NULL;

/* On fait un fondu */
gboolean 
cid_fade_in_out (void *ptr) 
{
    cid->iCurrentlyDrawing = 1;
    cid->bAnimation = TRUE;
    
    if (cid->dFadeInOutAlpha > .99)
        cid->dFadeInOutAlpha = 0;
        
    cid->dFadeInOutAlpha += IN_OUT_VARIATION * cid->iAnimationSpeed;
    
    CID_REDRAW;
    
    cid->iCurrentlyDrawing = 0;
    
    cid->bAnimation = cid->dFadeInOutAlpha < .99;
    if (!cid->bAnimation)
        cid_stop_measure_timer(pMeasureTimerAnimation);
    return cid->bAnimation;
}

/* On change la couleur de fond de la fenêtre au passage de la souris */
gboolean 
cid_focus_in(void *ptr) 
{
    cid->iCurrentlyDrawing = 1;
    cid->bFocusAnimation = TRUE;
    
    cid->dAlpha += cid->dFocusVariation * FADE_VARIATION;
    cid->dAnimationProgress = cid->dAlpha / (cid->dFocusVariation > 0 ? 
        cid->dFlyingColor[3]-cid->dColor[3] : cid->dColor[3]-cid->dFlyingColor[3]);
    
    CID_REDRAW;
    
    cid->iCurrentlyDrawing = 0;
    
    cid->bFocusAnimation = cid->dFocusVariation>0 ? 
        cid->dAlpha < cid->dFlyingColor[3] : cid->dAlpha > cid->dFlyingColor[3];
    if (!cid->bFocusAnimation) 
        cid_stop_measure_timer(pMeasureTimerFocus);
    return cid->bFocusAnimation;
}

/* On restaure la couleur de fond d'origine lorsqu'on quitte la fenêtre */
gboolean 
cid_focus_out(void *ptr) 
{
    cid->iCurrentlyDrawing = 1;
    
    cid->bFocusAnimation = TRUE;
    
    cid->dAlpha -= cid->dFocusVariation * FADE_VARIATION;
    cid->dAnimationProgress = cid->dAlpha / (cid->dFocusVariation > 0 ? 
        cid->dFlyingColor[3]-cid->dColor[3] : cid->dColor[3]-cid->dFlyingColor[3]);

    if (cid->dFocusVariation>0 ? cid->dAlpha <= cid->dColor[3] : cid->dAlpha >= cid->dColor[3]) 
    {
        cid->dRed = cid->dColor[0];
        cid->dGreen = cid->dColor[1];
        cid->dBlue = cid->dColor[2];
    }
    
    CID_REDRAW;
    
    cid->iCurrentlyDrawing = 0;
    cid->bCurrentlyFlying = cid->dFocusVariation>0 ? 
        cid->dAlpha > cid->dColor[3] : cid->dAlpha < cid->dColor[3];
    cid->bFocusAnimation = cid->bCurrentlyFlying;
    if (!cid->bFocusAnimation)
        cid_stop_measure_timer(pMeasureTimerFocus);
    return cid->bCurrentlyFlying;
}

/* On appelle la fonction adéquate selon qu'on "expose" ou non la fenêtre */
void 
cid_focus (GtkWidget *pWidget, GdkEventExpose *event, gpointer *userdata) 
{

    gboolean bFocusIn = GPOINTER_TO_INT (&userdata[0]);
    
    cid_info ("CID is currently focused %s.\n", bFocusIn ? "in" : "out");
    
    if (cid->bShowAbove)
        gtk_window_set_keep_below (GTK_WINDOW (cid->pWindow), !bFocusIn);
    
    if (bFocusIn) 
    {
        cid->bCurrentlyFlying = TRUE;
        // On change la couleur du fond par celle choisie
        cid->dRed = cid->dFlyingColor[0];
        cid->dGreen = cid->dFlyingColor[1];
        cid->dBlue = cid->dFlyingColor[2];
        // Coloration "smoothie"
        cid_animation(CID_FOCUS_IN);
    } else 
        cid_animation(CID_FOCUS_OUT);
}

/* callback de rotation */
gboolean 
cid_rotate_on_changing_song (void *ptr) 
{
    cid->iCurrentlyDrawing = 1;
    cid->bAnimation = TRUE;
    
    if (cid->dAngle < 360) 
    {
        cid->dAngle += (1.0 * cid->iAnimationSpeed);
        CID_REDRAW;
    } 
    else 
    {
        cid->bAnimation = FALSE;
        cid->dAngle = 0;
        cid_stop_measure_timer(pMeasureTimerAnimation);
    }

    cid->iCurrentlyDrawing = 0;
    
    return cid->bAnimation;
}

/* on lance un thread pour une animation */
void 
cid_threaded_animation (AnimationType iAnim, gint iDelay) 
{
    switch(iAnim) {
        case CID_ROTATE:
            if (pMeasureTimerAnimation) 
            {
                if (cid_measure_is_running(pMeasureTimerAnimation))
                    cid_stop_measure_timer(pMeasureTimerAnimation);
                if (cid_measure_is_active(pMeasureTimerAnimation))
                    cid_free_measure_timer(pMeasureTimerAnimation);
            }
            pMeasureTimerAnimation = cid_new_measure_timer (iDelay, 
                                                            NULL, 
                                                            NULL, 
                                                            (CidUpdateTimerFunc) cid_rotate_on_changing_song, 
                                                            NULL);
        
            cid_launch_measure(pMeasureTimerAnimation);
            break;
        case CID_FADE_IN_OUT:
            if (pMeasureTimerAnimation) 
            {
                if (cid_measure_is_running(pMeasureTimerAnimation))
                    cid_stop_measure_timer(pMeasureTimerAnimation);
                if (cid_measure_is_active(pMeasureTimerAnimation))
                    cid_free_measure_timer(pMeasureTimerAnimation);
            }
            pMeasureTimerAnimation = cid_new_measure_timer (iDelay, 
                                                            NULL, 
                                                            NULL, 
                                                            (CidUpdateTimerFunc) cid_fade_in_out, 
                                                            NULL);
        
            cid_launch_measure(pMeasureTimerAnimation);
            break;
        case CID_FOCUS_IN:
            if (pMeasureTimerFocus) 
            {
                if (cid_measure_is_running(pMeasureTimerFocus))
                    cid_stop_measure_timer(pMeasureTimerFocus);
                if (cid_measure_is_active(pMeasureTimerFocus))
                    cid_free_measure_timer(pMeasureTimerFocus);
            }
            pMeasureTimerFocus = cid_new_measure_timer (iDelay, 
                                                        NULL, 
                                                        NULL, 
                                                        (CidUpdateTimerFunc) cid_focus_in, 
                                                        NULL);
        
            cid_launch_measure(pMeasureTimerFocus);
            break;
        case CID_FOCUS_OUT:
            if (pMeasureTimerFocus) 
            {
                if (cid_measure_is_running(pMeasureTimerFocus))
                    cid_stop_measure_timer(pMeasureTimerFocus);
                if (cid_measure_is_active(pMeasureTimerFocus))
                    cid_free_measure_timer(pMeasureTimerFocus);
            }
            pMeasureTimerFocus = cid_new_measure_timer (iDelay, 
                                                        NULL, 
                                                        NULL, 
                                                        (CidUpdateTimerFunc) cid_focus_out, 
                                                        NULL);
        
            cid_launch_measure(pMeasureTimerFocus);
            break;
    }
}

/* G_PRIORITY_HIGH */
/* On anime l'image par une rotation */
void 
cid_animation (AnimationType iAnim) 
{
    switch(iAnim) {
        case CID_ROTATE:
            if (cid->bRunAnimation && !cid->bAnimation)
                if (cid->bThreaded)
                    cid_threaded_animation(iAnim,1 SECONDES/25);
                else 
                    g_timeout_add_full (-50, 1 SECONDES/25, (GSourceFunc) cid_rotate_on_changing_song, NULL, NULL);
            break;
        case CID_FADE_IN_OUT:
            if (cid->bRunAnimation && !cid->bAnimation)
                if (cid->bThreaded)
                    cid_threaded_animation(iAnim,1 SECONDES/12);
                else
                    g_timeout_add_full (-50, 1 SECONDES/12, (GSourceFunc) cid_fade_in_out, NULL, NULL);
            break;
        case CID_FOCUS_IN:
            if (cid->bThreaded)
                cid_threaded_animation(iAnim,1 SECONDES/12);
            else 
                g_timeout_add (1 SECONDES/12, (GSourceFunc) cid_focus_in, NULL);
            break;
        case CID_FOCUS_OUT:
            if (cid->bThreaded)
                cid_threaded_animation(iAnim,1 SECONDES/12);
            else 
                g_timeout_add (1 SECONDES/12, (GSourceFunc) cid_focus_out, NULL);
            break;
    }
}
