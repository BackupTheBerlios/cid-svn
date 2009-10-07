/*****************************************************************************************************
**
** Program:
**    Conky Images Display
**
** License :
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License, version 2 or above.
**    If you don't know what that means take a look at:
**       http://www.gnu.org/licenses/licenses.html#GPL
**
** Original idea :
**    Charlie MERLAND, July 2008.
**
*************************************************************
** Authors:
**    Charlie MERLAND
**    Benjamin SANS <sans_ben@yahoo.fr>
**
** Notes :
**    Originally conceived to use it with conky to display amarok's cover 
**    on desktop.
**    The project was re-write separatly for rhythmbox.
**    In the end, we deceided to merge our two programs "cleanly" with a DBus
**    support to add amarok2 and other players in the future...
**
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details. 
**
*******************************************************************************/
/*
   *
   *                                 cid.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid.h"
#include <X11/Xlib.h>

CidMainContainer *cid;

static gchar *cLaunchCommand = NULL;
static Display *s_XDisplay = NULL;

static int _cid_xerror_handler (Display * pDisplay, XErrorEvent *pXError)
{
    cid_debug ("Erreur (%d, %d, %d) lors d'une requete X sur %d", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
    return 0;
}

void cid_get_X_infos (void) {
    s_XDisplay = XOpenDisplay (0);
    
    XSetErrorHandler (_cid_xerror_handler);
    
    Screen *XScreen = XDefaultScreenOfDisplay (s_XDisplay);
    int XScreenWidth  = WidthOfScreen (XScreen);
    int XScreenHeight = HeightOfScreen (XScreen);
    
    g_print ("%dx%d\n",XScreenWidth,XScreenHeight);
}
void cid_init (CidMainContainer *pCid) {    
    
    pCid->pVerbosity = NULL;
    
    pCid->bTesting = FALSE;
    
    pCid->dAngle = 0;
    
    pCid->iCurrentlyDrawing = 0;
    
    pCid->pConfFile = g_strdup_printf("%s/.config/cid/%s",g_getenv("HOME"),CID_CONFIG_FILE);
    
    pCid->cidHint = GDK_WINDOW_TYPE_HINT_DOCK;
    
    pCid->pKeyFile = NULL;
}

/* Methode appelée pour relancer cid en cas de plantage */
void cid_intercept_signal (int signal) {
    cid_warning ("Attention : cid has crashed (sig %d).\nIt will be restarted now.\n", signal);
    execl ("/bin/sh", "/bin/sh", "-c", cLaunchCommand, NULL);  // on ne revient pas de cette fonction.
    cid_error ("Sorry, couldn't restart cid");
    cid_sortie (CID_EXIT_ERROR);
}

void cid_run_with_player (void) {
    if (cid->iPlayer != PLAYER_NONE)
        cid->pMonitorList = g_new0 (CidControlFunctionsList,1);
    /* On lance telle ou telle fonction selon le lecteur selectionne */
    switch (cid->iPlayer) {
        /* Amarok 1.4 */
        case PLAYER_AMAROK_1:
            cid_build_amarok_menu ();
            cid_connect_to_amarok(cid->iInter);
            break;
        /* Amarok 2 */
        case PLAYER_AMAROK_2:
            cid_build_amarok_2_menu ();
            if (amarok_2_dbus_connect_to_bus()) {
                cid_debug ("\ndbus connected\n");
                cid_display_image((gchar *)cid_amarok_2_cover());
            } else {
                cid_exit (CID_EXIT_ERROR,"\nFailed to connect dbus...\n");
            }
            break;
        /* Rhythmbox */
        case PLAYER_RHYTHMBOX:
            cid_build_rhythmbox_menu ();
            /* Initialisation de DBus */
            if (rhythmbox_dbus_connect_to_bus()) {
                cid_debug ("\ndbus connected\n");
                cid_display_image((gchar *)cid_rhythmbox_cover());
            } else {
                cid_exit (CID_EXIT_ERROR,"\nFailed to connect dbus...\n");
            }
            break;
        /* Exaile */
        case PLAYER_EXAILE:
            cid_build_exaile_menu ();
            cid_connect_to_exaile(cid->iInter);
            break;
        /* None */
        case PLAYER_NONE:
            cid_display_image(NULL);
            break;
        /* Sinon, on a un lecteur inconnu */
        default:
            cid_exit (CID_PLAYER_ERROR,"ERROR: \"%d\" is not recognize as a supported player\n",cid->iPlayer);
    }
}

/* Methode initialisant les signaux à intercepter */
void cid_set_signal_interception (void) {
    signal (SIGSEGV, cid_intercept_signal);  // Segmentation violation
    signal (SIGFPE, cid_intercept_signal);  // Floating-point exception
    signal (SIGILL, cid_intercept_signal);  // Illegal instruction
    signal (SIGABRT, cid_intercept_signal);  // Abort
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
        
    /* Enfin on lance la boucle Gtk */
    gtk_main();
}

/* Fonction principale */

int main ( int argc, char **argv ) {        

    cid = g_new0(CidMainContainer,1);
    
    int i;
    GString *sCommandString = g_string_new (argv[0]);
    for (i = 1; i < argc; i ++) {
        g_string_append_printf (sCommandString, " %s", argv[i]);
    }
    g_string_append_printf (sCommandString, " -s");
    cLaunchCommand = sCommandString->str;
    g_string_free (sCommandString, FALSE);
    
    cid_log_set_level(0);
    
    cid_init(cid);
    
    cid_set_signal_interception ();

    // On internationalise l'appli.
    bindtextdomain (CID_GETTEXT_PACKAGE, CID_LOCALE_DIR);
    bind_textdomain_codeset (CID_GETTEXT_PACKAGE, "UTF-8");
    textdomain (CID_GETTEXT_PACKAGE);
    
    cid_read_parameters (argc,argv);
    
    cid_read_config (cid->pConfFile, NULL);
    cid->bChangedTestingConf = cid->bTesting && cid->bUnstable;
    
    if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();

    // La on lance la boucle GTK
    //\___ FIXME Pas bien :/
//    cid_display_init (argc,argv);
    cid_display_init (0,NULL);
    
    // Si on est ici, c'est qu'on a coupé la boucle GTK
    // Du coup, on en profite pour faire un peu de ménage
    // histoire de pas laisser la baraque dans un sale etat
    cid_key_file_free();
    cid_free_musicData();
    cid_free_main_structure (cid);
    
    g_print ("Bye !\n");

    cid_get_X_infos();
    
    return CID_EXIT_SUCCESS;
    
}
