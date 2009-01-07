/*
   *
   *                               cid-gtk.c
   *                                -------
   *                          Conky Images Display
   *                    Charlie MERLAND / Benjamin SANS
   *                    --------------------------------
   *
*/

#include "cid.h"
#include <math.h>

gboolean supports_alpha = FALSE;
gboolean g_bRoundedBottomCorner = FALSE;

/* Fonction qui nous sert à afficher l'image dont le chemin
   est passé en argument. */
void cid_display_image(gchar *image) {
	/* debuggage */
	cid_debug (" %s (%s);\n",__func__,image);
	
	cairo_surface_destroy (cid->cSurface);

	if (g_file_test (image, G_FILE_TEST_EXISTS)) {
		cid->cSurface = cid_get_image (image, cid->iWidth, cid->iHeight);
		musicData.cover_exist = TRUE;
		if (musicData.iSidCheckCover != 0) {
			g_source_remove (musicData.iSidCheckCover);
			musicData.iSidCheckCover = 0;
		}
	} else {
		cid->cSurface = cid_get_image (DEFAULT_IMAGE, cid->iWidth, cid->iHeight);
		musicData.cover_exist = FALSE;
		cid->iCheckIter = 0;
		if (/*image != NULL && */musicData.iSidCheckCover == 0) {
			cid_debug ("image : %s, mais n'existe pas encore => on boucle.\n", image);
			musicData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc) _check_cover_is_present, (gpointer) NULL);
		}
	}
	
	gtk_widget_queue_draw (cid->cWindow);
}

/* On dessine l'image à partir de son URI */
cairo_surface_t *cid_get_image (char *cImagePath, gdouble iWidth, gdouble iHeight) {
	cid_debug ("%s (%s);\n",__func__,cImagePath);
	
//	cairo_surface_t* pNewSurface = NULL;
	gboolean bIsSVG = FALSE, bIsPNG = FALSE, bIsXPM = FALSE;
	FILE *fd = fopen (cImagePath, "r");
	
	if (fd != NULL) {
		char buffer[8];
		if (fgets (buffer, 7, fd) != NULL) {
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
	if (! bIsSVG && ! bIsPNG && ! bIsXPM) {  // sinon, on se base sur l'extension.
		cid_debug ("  on se base sur l'extension en desespoir de cause.");
		if (g_str_has_suffix (cImagePath, ".svg"))
			bIsSVG = TRUE;
		else if (g_str_has_suffix (cImagePath, ".png"))
			bIsPNG = TRUE;
	}

	//if (!bIsPNG) {
		// buffer de pixels après redimenssionement
		GdkPixbuf *nCover;
		// pochette
		GtkWidget *cCover;
	
		/* On récupère l'image depuis son chemin, sinon on sort */
		cCover = cid_get_image_widget(cImagePath);
		if(!cCover)
			cid_exit(CID_GTK_ERROR,"%s() : error loading image",__func__);
	
		nCover = gdk_pixbuf_scale_simple(cid_get_pixbuf(cCover), iWidth, iHeight, GDK_INTERP_BILINEAR);
		
		if (!bIsSVG)	
			gtk_widget_destroy(cCover);
		
		return cid_get_image_from_pixbuf (nCover);
	
		/// /!\ Fatal IO error 14 (Bad address) on X server :0.0.
		//g_object_unref(nCover);
		//gtk_widget_destroy(cCover);
 /*
	} else {
		cairo_surface_t* surface_ini;
		cairo_t* pCairoContext = NULL;
		surface_ini = cairo_image_surface_create_from_png (cImagePath);
		if (cairo_surface_status (surface_ini) == CAIRO_STATUS_SUCCESS) {
			
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
	//return pNewSurface;
}

GdkPixbuf *cid_get_pixbuf (GtkWidget *imageWidget) {
	return gtk_image_get_pixbuf(GTK_IMAGE(imageWidget));
}

GtkWidget *cid_get_image_widget(char *imageURI) {
	return gtk_image_new_from_file(imageURI);
}

/* Crée une image à partir d'un buffer de pixels */
cairo_surface_t *cid_get_image_from_pixbuf (GdkPixbuf *pixbuf) {
	cid_debug (" %s ();\n",__func__);
	GdkPixbuf *pPixbufWithAlpha = pixbuf;
	if (! gdk_pixbuf_get_has_alpha (pixbuf)) // on lui rajoute un canal alpha s'il n'en a pas.
		pPixbufWithAlpha = gdk_pixbuf_add_alpha (pixbuf, FALSE, 255, 255, 255);  // TRUE <=> les pixels blancs deviennent transparents.

	// On pre-multiplie chaque composante par le alpha (necessaire pour libcairo).
	int iNbChannels = gdk_pixbuf_get_n_channels (pPixbufWithAlpha);
	int iRowstride = gdk_pixbuf_get_rowstride (pPixbufWithAlpha);
	guchar *p=0, *pixels = gdk_pixbuf_get_pixels (pPixbufWithAlpha);

	int w = gdk_pixbuf_get_width (pPixbufWithAlpha);
	int h = gdk_pixbuf_get_height (pPixbufWithAlpha);
	int x, y;
	int red, green, blue;
	float fAlphaFactor;
	for (y = 0; y < h; y ++) {
		for (x = 0; x < w; x ++) {
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
	gdk_pixbuf_unref (pPixbufWithAlpha);
	return surface_ini;
}


/* Fonction qui créée la fenêtre, la déplace à la position
   voulue, et la dimensionne */
void cid_create_main_window() {
	/* On crée la fenêtre */
	cid->cWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	/* On place, nomme, et dimenssione la fenetre */
	gtk_window_set_title (GTK_WINDOW (cid->cWindow), "cid");
	gtk_window_move (GTK_WINDOW(cid->cWindow), cid->iPosX, cid->iPosY);
	gtk_window_set_default_size (GTK_WINDOW(cid->cWindow), cid->iWidth, cid->iHeight);
	gtk_window_set_gravity (GTK_WINDOW (cid->cWindow), GDK_GRAVITY_STATIC);

	/* On affiche cid sur tous les bureaux, ou pas */
	if (cid->bAllDesktop)
		gtk_window_stick (GTK_WINDOW (cid->cWindow));
	/* On bloque le déplacement (marche pas :/), on enlève les
	   barre de titre et bordures, on empêche l'apparition dans
	   la taskbar et le pager, et on la garde en arrière-plan. */
	gtk_window_set_decorated(GTK_WINDOW(cid->cWindow), FALSE);
	gtk_window_set_keep_below (GTK_WINDOW (cid->cWindow), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (cid->cWindow), TRUE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (cid->cWindow), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (cid->cWindow), cid->cidHint);
	gtk_widget_set_app_paintable (cid->cWindow, TRUE);

	/* On s'abonne aux évènement */
	gtk_widget_add_events (cid->cWindow,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	
	/* On intercepte et traite les évènements */
	g_signal_connect (G_OBJECT(cid->cWindow), "button-press-event", G_CALLBACK(on_clic), NULL); // Clic de souris
	g_signal_connect (G_OBJECT(cid->cWindow), "button-release-event", G_CALLBACK(on_clic), NULL); // Clic de souris
	if (cid->bTesting && cid->bUnstable) {
		g_signal_connect (G_OBJECT(cid->cWindow), "enter-notify-event", G_CALLBACK(cid_focus), GINT_TO_POINTER(TRUE)); // On passe le curseur sur la fenêtre
		g_signal_connect (G_OBJECT(cid->cWindow), "leave-notify-event", G_CALLBACK(cid_focus), GINT_TO_POINTER(FALSE)); // Le curseur quitte la fenêtre
	}

	g_signal_connect (G_OBJECT (cid->cWindow), "expose-event", G_CALLBACK (cid_draw_window), NULL);
	g_signal_connect (G_OBJECT (cid->cWindow), "screen-changed", G_CALLBACK (cid_set_colormap), NULL);
	
	g_signal_connect (G_OBJECT(cid->cWindow), "delete-event", G_CALLBACK(_cid_quit), NULL); // On ferme la fenêtre
 
	cid_set_colormap (cid->cWindow, NULL, NULL);
	
	gtk_widget_show_all(cid->cWindow);
}

/* On détermine si un gestionnaire de composite est présent ou non */
void cid_set_colormap (GtkWidget *widget, GdkScreen *old_screen, gpointer userdata) {
	GdkScreen *screen = NULL;
	GdkColormap *colormap = NULL;
 
	screen = gtk_widget_get_screen (widget);
	colormap = gdk_screen_get_rgba_colormap (screen);
	if (colormap == NULL) {
		cid_message ("Your screen does not support alpha channels!\n");
		colormap = gdk_screen_get_rgb_colormap(screen);
		supports_alpha = FALSE;
	} else {
		cid_message ("Your screen supports alpha channels!\n");
		supports_alpha = TRUE;
	}
 
	gtk_widget_set_colormap (widget, colormap);
}

/* Fonction qui s'occupe de dessiner la fenêtre */
void cid_set_render (cairo_t *pContext, gpointer *pData) {		
	cairo_t *cr = pContext;
	cairo_surface_t *image;
	
	cairo_set_source_rgba (cr, cid->dRed, cid->dGreen, cid->dBlue, cid->dAlpha);
	

	/*
	if (CID_PARAMETERS.iPlayer==PLAYER_RHYTHMBOX) {
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 10.0);
		cairo_move_to (cr, 10.0, 135.0);
		cairo_show_text (cr, rhythmboxData.playing_title);
		cairo_fill_preserve (cr);
		cairo_set_line_width (cr, 2.56);
		cairo_stroke (cr);
	}
	*/
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
		
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	/*
	if (cid->iPlayer==PLAYER_RHYTHMBOX) {
		//text = gdk_cairo_create (widget->window);
		//cairo_save (cr);
		
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 10.0);
		cairo_move_to (cr, 10.0, 135.0);
		cairo_show_text (cr, rhythmboxData.playing_title);
		cairo_fill_preserve (cr);
		cairo_set_line_width (cr, 2.56);
		cairo_stroke (cr);
		//cairo_restore (cr);
	}
	*/
	
	// Si le lecteur est lance ou qu'on ne cache pas cid
	if ( musicData.opening || !cid->bHide ) {
		cairo_save (cr);
		
		gdouble theta = fabs (cid->dRotate+cid->dAngle);
		while (theta > 90) {
			theta -= 90;
		}
		
		gdouble alpha = atan (theta);
		gdouble opp = alpha * (cid->iHeight/2);
		
		gdouble scaledWidth = cid->iWidth/2 + opp;
		gdouble scaledHeight = cid->iHeight/2 - opp;
		
		//g_print ("theta : %f, alpha : %f, opp : %f\n",theta,alpha,opp);
	
		gdouble hyp = cid->bTesting && cid->bUnstable ? sqrt (pow(scaledWidth,2)+pow(scaledHeight,2)) : sqrt (pow(cid->iWidth,2)+pow(cid->iHeight,2));		
		
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
		
		cairo_set_source_surface (cr, cid->cSurface, 0, 0);
		cairo_paint (cr);
		cairo_restore (cr);
	}
/*
	if ( cid->bCurrentlyPlaying ) {
		cairo_save(cr);
		//GtkWidget *im = cid_get_image_widget("../data/play.svg");
		//GdkPixbuf *image = cid_get_pixbuf(im);
		//cairo_surface_t *play = cid_get_image_from_pixbuf(image);
		cairo_set_source_surface (cr, cid_get_image("../data/play.svg",16,16), 0, 0);
		cairo_paint (cr);
		cairo_restore (cr);
	}
*/	
	
	cairo_destroy (cr);
}

/* redessine la fenêtre */
void cid_draw_window (GtkWidget *pWidget, GdkEventExpose *event, gpointer *userdata) {
	//cairo_t* pCairoContext = NULL;

	cid->pContext = gdk_cairo_create (cid->cWindow->window);
	if (!cid->pContext)
		return;

	cid_set_render (cid->pContext, userdata);
	//cairo_destroy (pCairoContext);	
}

double cid_draw_frame (cairo_t *pCairoContext, double fRadius, double fLineWidth, double fFrameWidth, double fFrameHeight, double fDockOffsetX, double fDockOffsetY, int sens, double fInclination) { // la largeur est donnee par rapport "au fond".
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

cairo_surface_t *cid_create_surface_from_text_full (gchar *cText, cairo_t* pSourceContext, CidLabelDescription *pLabelDescription, double fMaxScale, int iMaxWidth, int *iTextWidth, int *iTextHeight, double *fTextXOffset, double *fTextYOffset) {
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

void cid_run_with_player (void) {
	/* On lance telle ou telle fonction selon le lecteur selectionne */
	switch (cid->iPlayer) {
		/* Amarok 1.4 */
		case PLAYER_AMAROK_1:
			cid_connect_to_amarok(cid->iInter);
			break;
		/* Rhythmbox */
		case PLAYER_RHYTHMBOX:
			/* Initialisation de DBus */
			if (rhythmbox_dbus_connect_to_bus()) {
				cid_debug ("\ndbus connected\n");
				cid_display_image(cid_rhythmbox_cover());
			} else {
				cid_exit (CID_EXIT_ERROR,"\nFailed to connect dbus...\n");
			}
			break;
		/* Sinon, on a un lecteur inconnu */
		default:
			cid_exit (CID_PLAYER_ERROR,"ERROR: \"%d\" is not recognize as a supported player\n",cid->iPlayer);
	}
}

void cid_display_init(int argc, char **argv) {
	/* Initialisation de Gtk */
	if (!cid->bRunning)
		cid->bRunning = gtk_init_check(&argc, &argv);
	if (!cid->bRunning)
		cid_exit (CID_GTK_ERROR,"Unable to load gtk context");

	/* On intercepte les signaux */
	signal (SIGINT, cid_interrupt); // ctrl+c

	if (cid->bSafeMode) {
		_cid_conf_panel(NULL,NULL);
	}
	/* On créé la fenêtre */
	cid_create_main_window();
	
	/* On lance le monitoring du player */
	cid_run_with_player();	
        
	/* Enfin on lance la boucle infinie Gtk */
	gtk_main();
}
