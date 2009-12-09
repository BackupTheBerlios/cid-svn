/*
   *
   *                              cid-draw.c
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/

#include <math.h>
#include <signal.h>

#include "cid-draw.h"
#include "cid-animation.h"
#include "cid-utilities.h"
#include "cid-callbacks.h"
#include "cid-messages.h"
#include "cid-constantes.h"

extern CidMainContainer *cid;

static gboolean supports_alpha = FALSE;
static gboolean g_bRoundedBottomCorner = FALSE;
extern gboolean bFlyingButton;

static gchar *
cid_get_symbol_path (StateSymbol iState, SymbolColor iColor) 
{
    return g_strdup_printf("%s/%s-%s.png",cid->bDevMode ? "../data" : CID_DATA_DIR, STATE_SYMBOL[iState], STATE_COLOR[iColor]);
}

/* Fonction qui nous sert à afficher l'image dont le chemin
   est passé en argument. */
void 
cid_display_image(gchar *image) 
{
    cid_debug (" %s (%s);\n",__func__,image);
    
    // On recupere l'ancienne image pour faire le fondu
    if (cid->cPreviousSurface) 
    {
        cairo_surface_destroy (cid->cPreviousSurface);
        cid->cPreviousSurface = NULL;
    }
    if (cid->iAnimationType == CID_FADE_IN_OUT && cid->bRunAnimation)
        cid->cPreviousSurface = cairo_surface_reference(cid->cSurface);
        
    if (cid->cSurface) 
    {
        cairo_surface_destroy (cid->cSurface);
        cid->cSurface = NULL;
    }

    if (g_file_test (image, G_FILE_TEST_EXISTS)) 
    {
        cid->cSurface = cid_get_cairo_image (image, cid->iWidth, cid->iHeight);
        if (musicData.playing_cover && strcmp(musicData.playing_cover,image)!=0) 
        {
            g_free(musicData.playing_cover);
            musicData.playing_cover = g_strdup(image);
        }
        musicData.cover_exist = TRUE;
        if (musicData.iSidCheckCover != 0) 
        {
            g_source_remove (musicData.iSidCheckCover);
            musicData.iSidCheckCover = 0;
        }
    } 
    else 
    {
        cid->cSurface = cid_get_cairo_image (DEFAULT_IMAGE, cid->iWidth, cid->iHeight);
        musicData.cover_exist = FALSE;
        cid->iCheckIter = 0;
        if (musicData.iSidCheckCover == 0 && cid->iPlayer != PLAYER_NONE) 
        {
            cid_debug ("image : %s, mais n'existe pas encore => on boucle.\n", image);
            musicData.iSidCheckCover = g_timeout_add (1 SECONDES, (GSourceFunc) _check_cover_is_present, (gpointer) NULL);
        }
    }
    
    gtk_widget_queue_draw (cid->pWindow);
}

/* On dessine l'image à partir de son URI */
cairo_surface_t *
cid_get_cairo_image (char *cImagePath, gdouble iWidth, gdouble iHeight) 
{
    cid_debug ("%s (%s);\n",__func__,cImagePath);
    
    cairo_surface_t* pNewSurface = NULL;
    gboolean bIsSVG = FALSE, bIsPNG = FALSE, bIsXPM = FALSE;
    FILE *fd = fopen (cImagePath, "r");
    
    if (fd != NULL) 
    {
        char buffer[8];
        if (fgets (buffer, 7, fd) != NULL) 
        {
            if (strncmp (buffer+2, "xml", 3) == 0)
                bIsSVG = TRUE;
            else if (strncmp (buffer+1, "PNG", 3) == 0)
                bIsPNG = TRUE;
            else if (strncmp (buffer+3, "XPM", 3) == 0)
                bIsXPM = TRUE;
            cid_debug ("  format : %d;%d;%d", bIsSVG, bIsPNG, bIsXPM);
        }
        fclose (fd);
    }
    if (! bIsSVG && ! bIsPNG && ! bIsXPM) 
    {  // sinon, on se base sur l'extension.
        cid_debug ("  on se base sur l'extension en desespoir de cause.");
        if (g_str_has_suffix (cImagePath, ".svg"))
            bIsSVG = TRUE;
        else if (g_str_has_suffix (cImagePath, ".png"))
            bIsPNG = TRUE;
    }

//  if (!bIsPNG) {
        // buffer de pixels après redimenssionement
        GdkPixbuf *nCover;
        nCover = gdk_pixbuf_new_from_file_at_scale (cImagePath,iWidth, iHeight, FALSE, NULL);
        pNewSurface = cid_get_image_from_pixbuf (&nCover);
        if (!bIsSVG)
            g_object_unref (nCover);
/*
    } 
    else 
    {
        cairo_surface_t* surface_ini;
        cairo_t* pCairoContext = NULL;
        surface_ini = cairo_image_surface_create_from_png (cImagePath);
        if (cairo_surface_status (surface_ini) == CAIRO_STATUS_SUCCESS) 
        {
            
            pNewSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                cairo_image_surface_get_width (surface_ini),
                cairo_image_surface_get_height (surface_ini));
            pCairoContext = cairo_create (pNewSurface);
            
            cairo_scale (pCairoContext, iWidth, iHeight);
            
            cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
            cairo_paint (pCairoContext);
            cairo_destroy (pCairoContext);
        }
        cairo_surface_destroy (surface_ini);
    }   
*/
    return pNewSurface;
}

GdkPixbuf *
cid_get_pixbuf (GtkWidget **imageWidget) 
{
    return gtk_image_get_pixbuf(GTK_IMAGE(*imageWidget));
}

GtkWidget *
cid_get_image_widget(char **imageURI) 
{
    return gtk_image_new_from_file(*imageURI);
}

cairo_surface_t *
_cid_create_blank_surface (cairo_t *pSourceContext, int iWidth, int iHeight) 
{
    if (pSourceContext != NULL)
        return cairo_surface_create_similar (cairo_get_target (pSourceContext),
            CAIRO_CONTENT_COLOR_ALPHA,
            iWidth,
            iHeight);
    else
        return cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
            iWidth,
            iHeight);
}

/* Crée une image à partir d'un buffer de pixels */
cairo_surface_t *
cid_get_image_from_pixbuf (GdkPixbuf **pixbuf) 
{
    cid_debug (" %s ();\n",__func__);

    GdkPixbuf *pPixbufWithAlpha = *pixbuf;
    if (! gdk_pixbuf_get_has_alpha (*pixbuf)) 
    {  // on lui rajoute un canal alpha s'il n'en a pas.
        //g_print ("  ajout d'un canal alpha\n");
        pPixbufWithAlpha = gdk_pixbuf_add_alpha (*pixbuf, FALSE, 255, 255, 255);  // TRUE <=> les pixels blancs deviennent transparents.
    }

    //\____________________ On pre-multiplie chaque composante par le alpha (necessaire pour libcairo).
    int iNbChannels = gdk_pixbuf_get_n_channels (pPixbufWithAlpha);
    int iRowstride = gdk_pixbuf_get_rowstride (pPixbufWithAlpha);
    guchar *p, *pixels = gdk_pixbuf_get_pixels (pPixbufWithAlpha);

    int w = gdk_pixbuf_get_width (pPixbufWithAlpha);
    int h = gdk_pixbuf_get_height (pPixbufWithAlpha);
    int x, y;
    int red, green, blue;
    float fAlphaFactor;
    for (y = 0; y < h; y ++) 
    {
        for (x = 0; x < w; x ++) 
        {
            p = pixels + y * iRowstride + x * iNbChannels;
            fAlphaFactor = (float) p[3] / 255;
            red = p[0] * fAlphaFactor;
            green = p[1] * fAlphaFactor;
            blue = p[2] * fAlphaFactor;
            p[0] = blue;
            p[1] = green;
            p[2] = red;
        }
    }

    cairo_surface_t *surface_ini = cairo_image_surface_create_for_data (pixels,
        CAIRO_FORMAT_ARGB32,
        gdk_pixbuf_get_width (pPixbufWithAlpha),
        gdk_pixbuf_get_height (pPixbufWithAlpha),
        gdk_pixbuf_get_rowstride (pPixbufWithAlpha));

    cairo_surface_t *pNewSurface = _cid_create_blank_surface (NULL,
        cid->iWidth,
        cid->iHeight);
    cairo_t *pCairoContext = cairo_create (pNewSurface);

    cairo_set_source_surface (pCairoContext, surface_ini, 0, 0);
    cairo_paint (pCairoContext);
    cairo_surface_destroy (surface_ini);
    cairo_destroy (pCairoContext);
    
    if (pPixbufWithAlpha != *pixbuf)
        g_object_unref (pPixbufWithAlpha);

    return pNewSurface;
}

/* Fonction qui créée la fenêtre, la déplace à la position
   voulue, et la dimensionne */
void 
cid_create_main_window() 
{
    /* On crée la fenêtre */
    cid->pWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    cid_check_position ();
    
    /* On place, nomme, et dimenssione la fenetre */
    gtk_window_set_title (GTK_WINDOW (cid->pWindow), "cid");
    gtk_window_move (GTK_WINDOW(cid->pWindow), cid->iPosX, cid->iPosY);
    gtk_window_set_default_size (GTK_WINDOW(cid->pWindow), cid->iWidth, cid->iHeight);
    gtk_window_set_gravity (GTK_WINDOW (cid->pWindow), GDK_GRAVITY_STATIC);

    /* On affiche cid sur tous les bureaux, ou pas */
    if (cid->bAllDesktop)
        gtk_window_stick (GTK_WINDOW (cid->pWindow));
    /* On bloque le déplacement (marche pas :/), on enlève les
       barre de titre et bordures, on empêche l'apparition dans
       la taskbar et le pager, et on la garde en arrière-plan. */
    gtk_window_set_decorated(GTK_WINDOW(cid->pWindow), FALSE);
    gtk_window_set_keep_below (GTK_WINDOW (cid->pWindow), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (cid->pWindow), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (cid->pWindow), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (cid->pWindow), cid->cidHint);
    gtk_widget_set_app_paintable (cid->pWindow, TRUE);

    /* On s'abonne aux évènement */
    gtk_widget_add_events  (cid->pWindow,
                            GDK_BUTTON_PRESS_MASK | 
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_KEY_PRESS_MASK |
                            GDK_POINTER_MOTION_MASK | 
                            GDK_POINTER_MOTION_HINT_MASK);
    
    /* On intercepte et traite les évènements */
    g_signal_connect (G_OBJECT (cid->pWindow), "expose-event", G_CALLBACK (cid_draw_window), NULL);
    g_signal_connect (G_OBJECT (cid->pWindow), "screen-changed", G_CALLBACK (cid_set_colormap), NULL);
    
    g_signal_connect (G_OBJECT(cid->pWindow), "delete-event", G_CALLBACK(_cid_quit), NULL); // On ferme la fenêtre
    
    /* Ici on traite le focus in/out */
    g_signal_connect (G_OBJECT(cid->pWindow), "enter-notify-event", G_CALLBACK(cid_focus), GINT_TO_POINTER(TRUE)); // On passe le curseur sur la fenêtre
    g_signal_connect (G_OBJECT(cid->pWindow), "leave-notify-event", G_CALLBACK(cid_focus), GINT_TO_POINTER(FALSE)); // Le curseur quitte la fenêtre
    
    ///\_______ TESTING OPTION
    if (cid->bUnstable) 
    {
        /* On prépare le traitement du d'n'd */
        GtkTargetEntry *pTargetEntry = g_new0 (GtkTargetEntry, 6);
        pTargetEntry[0].target = "text/*";
        pTargetEntry[0].flags = (GtkTargetFlags) 0;
        pTargetEntry[0].info = 0;
        pTargetEntry[1].target = "text/uri-list";
        pTargetEntry[2].target = "text/plain";
        pTargetEntry[3].target = "text/plain;charset=UTF-8";
        pTargetEntry[4].target = "text/directory";
        pTargetEntry[5].target = "text/html";
        gtk_drag_dest_set (cid->pWindow,
            GTK_DEST_DEFAULT_DROP | GTK_DEST_DEFAULT_MOTION,  // GTK_DEST_DEFAULT_HIGHLIGHT ne rend pas joli je trouve.
            pTargetEntry,
            6,
            GDK_ACTION_COPY | GDK_ACTION_MOVE);  // le 'GDK_ACTION_MOVE' c'est pour KDE.
        g_free (pTargetEntry);
    
        /* traitement du d'n'd */
        g_signal_connect (G_OBJECT(cid->pWindow), "drag-data-received", G_CALLBACK(on_dragNdrop_data_received), NULL);
    }
    
    /* traitement des clics */
    g_signal_connect (G_OBJECT(cid->pWindow), "button-press-event", G_CALLBACK(on_clic), NULL); // Clic de souris
    g_signal_connect (G_OBJECT(cid->pWindow), "button-release-event", G_CALLBACK(on_clic), NULL); // Clic de souris
    
    /* On veut connaitre la position du curseur */
    g_signal_connect (G_OBJECT(cid->pWindow), "motion-notify-event", G_CALLBACK(on_motion), NULL);
 
    cid_set_colormap (cid->pWindow, NULL, NULL);
    
    /* Chargement des dessins */
    cid_load_symbols ();
    
    gtk_widget_show_all(cid->pWindow);
}

/* On détermine si un gestionnaire de composite est présent ou non */
void 
cid_set_colormap (GtkWidget *widget, GdkScreen *old_screen, gpointer userdata) 
{
    GdkScreen *screen = NULL;
    GdkColormap *colormap = NULL;
 
    screen = gtk_widget_get_screen (widget);
    colormap = gdk_screen_get_rgba_colormap (screen);
    if (colormap == NULL) 
    {
        cid_message ("Your screen does not support alpha channels!\n");
        colormap = gdk_screen_get_rgb_colormap(screen);
        supports_alpha = FALSE;
    } 
    else 
    {
        cid_message ("Your screen supports alpha channels!\n");
        supports_alpha = TRUE;
    }
 
    gtk_widget_set_colormap (widget, colormap);
}

void 
cid_draw_text (cairo_t *cr) 
{
    cairo_set_source_rgba (cr, 1, 0, 0, 1);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    cairo_save(cr);

    cairo_text_extents_t extents;

    cairo_text_extents(cr, musicData.playing_title, &extents);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, cid->dPoliceSize);
    
    gdouble posX = ((cid->iWidth/2) - (extents.width/2 - extents.x_bearing));
    //g_print ("posX : %f | taille-text : %f | bearing : %f",posX, extents.width, extents.x_bearing);
    
    cairo_move_to (cr, posX , cid->iHeight - extents.height);
        
    cairo_text_path (cr, musicData.playing_title);
    cairo_set_source_rgba (cr, cid->dPoliceColor[0], cid->dPoliceColor[1], cid->dPoliceColor[2], cid->dPoliceColor[3]);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, cid->dOutlineTextColor[0], cid->dOutlineTextColor[1], cid->dOutlineTextColor[2], cid->dOutlineTextColor[3]);
    cairo_set_line_width (cr, cid->dPoliceSize/15.0);

    cairo_stroke (cr);
    cairo_restore (cr);
/*
    // a custom shape that could be wrapped in a function 
    double x0      = 25.6,   // parameters like cairo_rectangle 
           y0      = 25.6,
           rect_width  = extents.width,
           rect_height = extents.height,
           radius = 102.4;   // and an approximate curvature radius

    double x1,y1;

    x1=x0+rect_width;
    y1=y0+rect_height;
    if (!rect_width || !rect_height)
        return;
    if (rect_width/2<radius) 
    {
        if (rect_height/2<radius) 
        {
            cairo_move_to  (cr, x0, (y0 + y1)/2);
            cairo_curve_to (cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
            cairo_curve_to (cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
            cairo_curve_to (cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
            cairo_curve_to (cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
        } 
        else 
        {
            cairo_move_to  (cr, x0, y0 + radius);
            cairo_curve_to (cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
            cairo_curve_to (cr, x1, y0, x1, y0, x1, y0 + radius);
            cairo_line_to (cr, x1 , y1 - radius);
            cairo_curve_to (cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
            cairo_curve_to (cr, x0, y1, x0, y1, x0, y1- radius);
        }
    } 
    else 
    {
        if (rect_height/2<radius) 
        {
            cairo_move_to  (cr, x0, (y0 + y1)/2);
            cairo_curve_to (cr, x0 , y0, x0 , y0, x0 + radius, y0);
            cairo_line_to (cr, x1 - radius, y0);
            cairo_curve_to (cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
            cairo_curve_to (cr, x1, y1, x1, y1, x1 - radius, y1);
            cairo_line_to (cr, x0 + radius, y1);
            cairo_curve_to (cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
        } 
        else 
        {
            cairo_move_to  (cr, x0, y0 + radius);
            cairo_curve_to (cr, x0 , y0, x0 , y0, x0 + radius, y0);
            cairo_line_to (cr, x1 - radius, y0);
            cairo_curve_to (cr, x1, y0, x1, y0, x1, y0 + radius);
            cairo_line_to (cr, x1 , y1 - radius);
            cairo_curve_to (cr, x1, y1, x1, y1, x1 - radius, y1);
            cairo_line_to (cr, x0 + radius, y1);
            cairo_curve_to (cr, x0, y1, x0, y1, x0, y1- radius);
        }
    }
    cairo_close_path (cr);

    cairo_set_source_rgb (cr, 0.5, 0.5, 1);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 0.5, 0, 0, 0.5);
    cairo_set_line_width (cr, 10.0);
    cairo_stroke (cr);
*/
}

/* Charge les symbols */
void 
cid_load_symbols (void) 
{
    if (cid->cPlay)
        cairo_surface_destroy (cid->cPlay);
    if (cid->cPause)
        cairo_surface_destroy (cid->cPause);
    if (cid->cNext)
        cairo_surface_destroy (cid->cNext);
    if (cid->cPrev)
        cairo_surface_destroy (cid->cPrev);
    if (cid->cPlay_big)
        cairo_surface_destroy (cid->cPlay_big);
    if (cid->cPause_big)
        cairo_surface_destroy (cid->cPause_big);
    if (cid->cCross)
        cairo_surface_destroy (cid->cCross);
    
    cid->cPlay      = cid_get_cairo_image(cid_get_symbol_path(CID_PLAY,cid->iSymbolColor),cid->iExtraSize,cid->iExtraSize);
    cid->cPause     = cid_get_cairo_image(cid_get_symbol_path(CID_PAUSE,cid->iSymbolColor),cid->iExtraSize,cid->iExtraSize);
    cid->cNext      = cid_get_cairo_image(cid_get_symbol_path(CID_NEXT,cid->iSymbolColor),cid->iPrevNextSize,cid->iPrevNextSize);
    cid->cPrev      = cid_get_cairo_image(cid_get_symbol_path(CID_PREV,cid->iSymbolColor),cid->iPrevNextSize,cid->iPrevNextSize);
    cid->cPlay_big  = cid_get_cairo_image(cid_get_symbol_path(CID_PLAY,cid->iSymbolColor),cid->iPlayPauseSize,cid->iPlayPauseSize);
    cid->cPause_big = cid_get_cairo_image(cid_get_symbol_path(CID_PAUSE,cid->iSymbolColor),cid->iPlayPauseSize,cid->iPlayPauseSize);
    cid->cCross = cid_get_cairo_image(g_strdup_printf("%s/cross.png",cid->bDevMode ? "../data" : CID_DATA_DIR),cid->iExtraSize,cid->iExtraSize);
}

/* Fonction qui s'occupe de dessiner la fenêtre */
void 
cid_set_render (cairo_t *pContext, gpointer *pData) 
{      
    cairo_t *cr = pContext;
    
    if (!cid->bMask)
        cairo_set_source_rgba (cr, cid->dRed, cid->dGreen, cid->dBlue, cid->dAlpha);
    else
        cairo_set_source_rgba (cr, cid->dColor[0], cid->dColor[1], cid->dColor[2], cid->dColor[3]);
        
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    
    // Si le lecteur est lance ou qu'on ne cache pas cid
    if ( musicData.opening || !cid->bHide ) 
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save (cr);

        ///\_______ des maths... je suis nul en maths xD
        ///         le but c'est calculer le bon ratio pour l'image pendant une rotation        
        //gdouble theta = fabs (cid->dRotate);
        //if (theta > M_PI/2)
        //  theta -= M_PI/2;
        
        //gdouble alpha = atan2 (cid->iHeight, cid->iWidth);
        
        gdouble hyp = sqrt (pow(cid->iWidth,2)+pow(cid->iHeight,2));
        
        //gdouble scaledWidth = fabs (cid->iWidth / (hyp * sin (alpha + theta)));
        //gdouble scaledHeight = fabs (cid->iHeight / (hyp * cos (alpha - theta)));
        
        //g_print ("theta : %f, alpha : %f, d : %f, scaledX : %f, scaledY : %f\n",theta,alpha, d, scaledWidth, scaledHeight);

        cairo_translate (cr, cid->iWidth/2, cid->iHeight/2);
        
        // Est ce qu'on est durant une animation
        if (!cid->bAnimation)
            cairo_rotate (cr, cid->dRotate * M_PI/180);
        else
            cairo_rotate (cr, (cid->dRotate + cid->dAngle) * M_PI/180);
        
        // Est ce qu'on coupe les coins
        if (cid->bKeepCorners)
            cairo_scale  (cr, cid->iWidth/hyp, cid->iHeight/hyp);
        

        cairo_translate (cr, -cid->iWidth/2, -cid->iHeight/2);
        
        // Si on utilise le fondu, et qu'on a un alpha <1 on dessine nos 2 surfaces :)
        if (cid->cPreviousSurface!=NULL && cid->iAnimationType == CID_FADE_IN_OUT 
            && cid->dFadeInOutAlpha < 1 && cid->bAnimation) {
            cairo_set_source_surface (cr, cid->cPreviousSurface, 0, 0);
            cairo_paint_with_alpha (cr, 1-cid->dFadeInOutAlpha);
            
            cairo_set_source_surface (cr, cid->cSurface, 0, 0);
            cairo_paint_with_alpha (cr, cid->dFadeInOutAlpha);
            
            
        } 
        else 
        {
            cairo_set_source_surface (cr, cid->cSurface, 0, 0);
            cairo_paint (cr);
        }
        cairo_restore (cr); 
    } 
    else 
    if (!cid->bCurrentlyFlying)
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
        cairo_save (cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_paint (cr);
        cairo_restore (cr);
    }
    
    // Si on affiche l'etat du lecteur
    if (cid->bPlayerState && musicData.opening && cid->iPlayer != PLAYER_NONE) 
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_set_source_surface (cr, musicData.playing ? cid->cPlay : cid->cPause, 0, 0);
        cairo_paint (cr);
        cairo_restore (cr); 
    }
    
    ///\_______ TESTING OPTION
    // si on lit de la musique et qu'on ne cache pas cid, on affiche le nom de la piste jouee
    if (cid->bUnstable && cid->bDisplayTitle && (musicData.opening || !cid->bHide)) 
    {  
        cid_draw_text(cr);
    }
    
    // Si on survolle CID et qu'on affiche un masque
    if (cid->bCurrentlyFlying && cid->bMask) 
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, cid->dRed, cid->dGreen, cid->dBlue, cid->dAlpha);
        cairo_paint (cr);
        cairo_restore (cr); 
    }
    
    // Si on survolle et qu'on autorise le deplacement, on affiche une petite croix
    if (cid->bCurrentlyFlying && !cid->bLockPosition) 
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_translate (cr, cid->iWidth-cid->iExtraSize, 0);
        cairo_set_source_surface (cr, cid->cCross, 0, 0);
        cairo_paint (cr);
        cairo_restore (cr); 
    }
    
    // Si on survolle et qu'on autorise l'affichage des controles
    if (cid->bCurrentlyFlying && cid->bDisplayControl && cid->iPlayer != PLAYER_NONE && cid->bMonitorPlayer) 
    {
        // Previous button
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_translate (cr, 0, (cid->iHeight - cid->iPrevNextSize)/2);
        cairo_set_source_surface (cr, cid->cPrev, 0, 0);
        if (cid->iCursorX < cid->iPrevNextSize &&
            cid->iCursorY < (cid->iHeight + cid->iPrevNextSize)/2 &&
            cid->iCursorY > (cid->iHeight - cid->iPrevNextSize)/2) {
            
            cairo_paint_with_alpha (cr, .5);
            
        } 
        else 
        {
            if (cid->dAnimationProgress < 1)
                cairo_paint_with_alpha (cr, cid->dAnimationProgress);
            else
                cairo_paint (cr);
        }
        cairo_restore (cr); 
        // Play/Pause button
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_translate (cr, (cid->iWidth - cid->iPlayPauseSize)/2, (cid->iHeight - cid->iPlayPauseSize)/2);
        cairo_set_source_surface (cr, !musicData.playing ? cid->cPlay_big : cid->cPause_big, 0, 0);
        if (cid->iCursorX < (cid->iWidth + cid->iPlayPauseSize)/2 &&
            cid->iCursorX > (cid->iWidth - cid->iPlayPauseSize)/2 &&
            cid->iCursorY < (cid->iHeight + cid->iPlayPauseSize)/2 &&
            cid->iCursorY > (cid->iHeight - cid->iPlayPauseSize)/2) {
            
            cairo_paint_with_alpha (cr, .5);
            
        } 
        else 
        {
            if (cid->dAnimationProgress < 1)
                cairo_paint_with_alpha (cr, cid->dAnimationProgress);
            else
                cairo_paint (cr);
        }
        cairo_restore (cr); 
        // Next button
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 0);
        cairo_translate (cr, (cid->iWidth - cid->iPrevNextSize), (cid->iHeight - cid->iPrevNextSize)/2);
        cairo_set_source_surface (cr, cid->cNext, 0, 0);
        if (cid->iCursorX > cid->iWidth - cid->iPrevNextSize &&
            cid->iCursorY < (cid->iHeight + cid->iPrevNextSize)/2 &&
            cid->iCursorY > (cid->iHeight - cid->iPrevNextSize)/2) {
            
            cairo_paint_with_alpha (cr, .5);
            
        } 
        else 
        {
            if (cid->dAnimationProgress < 1)
                cairo_paint_with_alpha (cr, cid->dAnimationProgress);
            else
                cairo_paint (cr);
        }
        cairo_restore (cr); 
    }
    
    cairo_destroy (cr);
}

void 
cid_set_state_icon (void) 
{
    gtk_widget_queue_draw (cid->pWindow);
}

/* redessine la fenêtre */
void 
cid_draw_window (GtkWidget *pWidget, GdkEventExpose *event, gpointer *userdata) 
{

    cid->pContext = gdk_cairo_create (cid->pWindow->window);
    if (!cid->pContext)
        return;

    cid_set_render (cid->pContext, userdata);
}

double 
cid_draw_frame (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination) 
{ // la largeur est donnee par rapport "au fond".
    if (2*fRadius > fFrameHeight + fLineWidth)
        fRadius = (fFrameHeight + fLineWidth) / 2 - 1;
    double fDeltaXForLoop = fInclination * (fFrameHeight + fLineWidth - (g_bRoundedBottomCorner ? 2 : 1) * fRadius);
    double cosa = 1. / sqrt (1 + fInclination * fInclination);
    double sina = cosa * fInclination;

    cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);

    cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
    //\_________________ Coin haut droit.
    cairo_rel_curve_to (pCairoContext,
        0, 0,
        fRadius * (1. / cosa - fInclination), 0,
        fRadius * cosa, sens * fRadius * (1 - sina));
    cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (fFrameHeight + fLineWidth - fRadius * (g_bRoundedBottomCorner ? 2 : 1 - sina)));
    //\_________________ Coin bas droit.
    if (g_bRoundedBottomCorner)
        cairo_rel_curve_to (pCairoContext,
            0, 0,
            fRadius * (1 + sina) * fInclination, sens * fRadius * (1 + sina),
            -fRadius * cosa, sens * fRadius * (1 + sina));

    cairo_rel_line_to (pCairoContext, - fFrameWidth -  2 * fDeltaXForLoop - (g_bRoundedBottomCorner ? 0 : 2 * fRadius * cosa), 0);
    //\_________________ Coin bas gauche.
    if (g_bRoundedBottomCorner)
        cairo_rel_curve_to (pCairoContext,
            0, 0,
            -fRadius * (fInclination + 1. / cosa), 0,
            -fRadius * cosa, -sens * fRadius * (1 + sina));
    cairo_rel_line_to (pCairoContext, fDeltaXForLoop, sens * (- fFrameHeight - fLineWidth + fRadius * (g_bRoundedBottomCorner ? 2 : 1 - sina)));
    //\_________________ Coin haut gauche.
    cairo_rel_curve_to (pCairoContext,
        0, 0,
        fRadius * (1 - sina) * fInclination, -sens * fRadius * (1 - sina),
        fRadius * cosa, -sens * fRadius * (1 - sina));
    if (fRadius < 1)
        cairo_close_path (pCairoContext);
    return fDeltaXForLoop + fRadius * cosa;
}

cairo_surface_t *
cid_create_surface_from_text_full (gchar *cText, cairo_t* pSourceContext, CidLabelDescription *pLabelDescription, double fMaxScale, int iMaxWidth, int *iTextWidth, int *iTextHeight, double *fTextXOffset, double *fTextYOffset) 
{
    g_return_val_if_fail (cText != NULL && pLabelDescription != NULL && pSourceContext != NULL && cairo_status (pSourceContext) == CAIRO_STATUS_SUCCESS, NULL);
    
    //\_________________ On ecrit le texte dans un calque Pango.
    PangoLayout *pLayout = pango_cairo_create_layout (pSourceContext);
    
    PangoFontDescription *pDesc = pango_font_description_new ();
    pango_font_description_set_absolute_size (pDesc, fMaxScale * pLabelDescription->iSize * PANGO_SCALE);
    pango_font_description_set_family_static (pDesc, pLabelDescription->cFont);
    pango_font_description_set_weight (pDesc, pLabelDescription->iWeight);
    pango_font_description_set_style (pDesc, pLabelDescription->iStyle);
    pango_layout_set_font_description (pLayout, pDesc);
    pango_font_description_free (pDesc);
    
    pango_layout_set_text (pLayout, cText, -1);
    
    //\_________________ On cree une surface aux dimensions du texte.
    PangoRectangle ink, log;
    pango_layout_get_pixel_extents (pLayout, &ink, &log);
    
    double fZoom = ((iMaxWidth != 0 && ink.width + 2 > iMaxWidth) ? 1.*iMaxWidth / (ink.width + 2) : 1.);
    
    *iTextWidth = (ink.width + 2) * fZoom;
    *iTextHeight = ink.height + 2 + 1; // +1 car certaines polices "debordent".
    //Test du zoom en W ET H *iTextHeight = (ink.height + 2 + 1) * fZoom; 
    
    cairo_surface_t* pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
        CAIRO_CONTENT_COLOR_ALPHA,
        *iTextWidth, *iTextHeight);
    cairo_t* pCairoContext = cairo_create (pNewSurface);
    
    //\_________________ On dessine le fond.
    if (pLabelDescription->fBackgroundColor != NULL && pLabelDescription->fBackgroundColor[3] > 0)  // non transparent.
    {
        cairo_save (pCairoContext);
        double fRadius = fMaxScale * MIN (.5 * 1, 5.);  // bon compromis.
        double fLineWidth = 1.;
        double fFrameWidth = *iTextWidth - 2 * fRadius - fLineWidth;
        double fFrameHeight = *iTextHeight - fLineWidth;
        double fDockOffsetX = fRadius + fLineWidth/2;
        double fDockOffsetY = 0.;
        cid_draw_frame (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, 1, 0.);
        cairo_set_source_rgba (pCairoContext, pLabelDescription->fBackgroundColor[0], pLabelDescription->fBackgroundColor[1], pLabelDescription->fBackgroundColor[2], pLabelDescription->fBackgroundColor[3]);
        cairo_fill_preserve (pCairoContext);
        cairo_restore(pCairoContext);
    }
    
    cairo_translate (pCairoContext, -ink.x, -ink.y+1);  // meme remarque.
    
    //\_________________ On dessine les contours.
    cairo_save (pCairoContext);
    if (fZoom!= 1)
        cairo_scale (pCairoContext, fZoom, 1.);
    cairo_push_group (pCairoContext);
    cairo_set_source_rgb (pCairoContext, 0.2, 0.2, 0.2);
    int i;
    for (i = 0; i < 4; i++)
    {
        cairo_move_to (pCairoContext, i&2, 2*(i&1));
        pango_cairo_show_layout (pCairoContext, pLayout);
    }
    cairo_pop_group_to_source (pCairoContext);
    cairo_paint_with_alpha (pCairoContext, .75);
    cairo_restore(pCairoContext);
    
    //\_________________ On remplit l'interieur du texte.
    cairo_pattern_t *pGradationPattern = NULL;
    if (pLabelDescription->fColorStart != pLabelDescription->fColorStop)
    {
        if (pLabelDescription->bVerticalPattern)
            pGradationPattern = cairo_pattern_create_linear (0.,
                ink.y-1.,
                0.,
                *iTextHeight+ink.y-1);
        else
            pGradationPattern = cairo_pattern_create_linear (ink.x,
                0.,
                *iTextWidth + ink.x,
                0.);
        g_return_val_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS, NULL);
        cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
        cairo_pattern_add_color_stop_rgba (pGradationPattern,
            0.,
            pLabelDescription->fColorStart[0],
            pLabelDescription->fColorStart[1],
            pLabelDescription->fColorStart[2],
            1.);
        cairo_pattern_add_color_stop_rgba (pGradationPattern,
            1.,
            pLabelDescription->fColorStop[0],
            pLabelDescription->fColorStop[1],
            pLabelDescription->fColorStop[2],
            1.);
        cairo_set_source (pCairoContext, pGradationPattern);
    }
    else
        cairo_set_source_rgb (pCairoContext, pLabelDescription->fColorStart[0], pLabelDescription->fColorStart[1], pLabelDescription->fColorStart[2]);
    cairo_move_to (pCairoContext, 1., 1.);
    if (fZoom!= 1)
        cairo_scale (pCairoContext, fZoom, 1.);
    pango_cairo_show_layout (pCairoContext, pLayout);
    cairo_pattern_destroy (pGradationPattern);
    
    cairo_destroy (pCairoContext);
    
    /* set_device_offset is buggy, doesn't work for positive offsets. so we use explicit offsets... so unfortunate.
    cairo_surface_set_device_offset (pNewSurface,
                     log.width / 2. - ink.x,
                     log.height     - ink.y);*/
    *fTextXOffset = (log.width * fZoom / 2. - ink.x) / fMaxScale;
    *fTextYOffset = - (pLabelDescription->iSize - (log.height - ink.y)) / fMaxScale ;  // en tenant compte de l'ecart du bas du texte.
    //*fTextYOffset = - (ink.y) / fMaxScale;  // pour tenir compte de l'ecart du bas du texte.
    
    *iTextWidth = *iTextWidth / fMaxScale;
    *iTextHeight = *iTextHeight / fMaxScale;
    
    g_object_unref (pLayout);
    return pNewSurface;
}
