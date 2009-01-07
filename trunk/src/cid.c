/*****************************************************************************************************
**
** Program:
**    Conky Images Display
**
** License :
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License, version 3 or above.
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
**    In the end, we deceided to merge our two programs "clearly" with a DBus
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
   *             05/10/2008 - Charlie MERLAND / Benjamin SANS
   *             --------------------------------------------
   *
*/

#include "cid.h"

static gchar *cLaunchCommand = NULL;

void cid_init (CidMainContainer *pCid) {	
	
	//pCid->pPlayer=NULL;
	
	pCid->pVerbosity=NULL;
	
	pCid->bHide=FALSE;
	
	pCid->bTesting=FALSE;
	
	pCid->iCheckIter=0;
	
	pCid->iInter=5 * 1000;
	
	pCid->iPosX=0;
	
	pCid->iPosY=0;
	
	pCid->iWidth=150;
	
	pCid->iHeight=150;
	
	pCid->gColorSize=0;
	
	pCid->dAngle=0;
	
	pCid->iCurrentlyDrawing=0;
	
	pCid->bAnimation=FALSE;
	
	pCid->pConfFile = g_strdup_printf("%s/.config/cid/%s",g_getenv("HOME"),CID_CONFIG_FILE);
	
	pCid->pDefaultImage = g_strdup(CID_DEFAULT_IMAGE);
	
	pCid->cidHint = GDK_WINDOW_TYPE_HINT_DOCK;
	
	pCid->pKeyFile = NULL;
}

/* Methode appelée pour relancer cid en cas de plantage */
void cid_intercept_signal (int signal) {
	cid_warning ("Attention : cid has crashed (sig %d).\nIt will be restarted now.\n", signal);
	cid_sortie (CID_EXIT_ERROR);
	execl ("/bin/sh", "/bin/sh", "-c", cLaunchCommand, NULL);  // on ne revient pas de cette fonction.
	cid_error ("Sorry, couldn't restart cid");
}

/* Methode initialisant les signaux à intercepter */
void cid_set_signal_interception (void) {
	signal (SIGSEGV, cid_intercept_signal);  // Segmentation violation
	signal (SIGFPE, cid_intercept_signal);  // Floating-point exception
	signal (SIGILL, cid_intercept_signal);  // Illegal instruction
	//signal (SIGABRT, cid_intercept_signal);  // Abort
}

/* Fonction principale */

int main ( int argc, char **argv ) {		
	cid = malloc (sizeof *cid);
	
	int i;
	GString *sCommandString = g_string_new (argv[0]);
	for (i = 1; i < argc; i ++)
	{
		g_string_append_printf (sCommandString, " %s", argv[i]);
	}
	g_string_append_printf (sCommandString, " -s");
	cLaunchCommand = sCommandString->str;
	g_string_free (sCommandString, FALSE);
	
	cid_log_set_level(0);
	
	cid_init(cid);
	
	cid_set_signal_interception ();
	
	cid_read_parameters (argc,argv);

	// On internationalise l'appli.
	bindtextdomain (CID_GETTEXT_PACKAGE, CID_LOCALE_DIR);
	bind_textdomain_codeset (CID_GETTEXT_PACKAGE, "UTF-8");
	textdomain (CID_GETTEXT_PACKAGE);
	
	cid_read_config (cid->pConfFile, NULL);
	cid->bChangedTestingConf = cid->bTesting && cid->bUnstable;
	
	if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
	
	cid_display_init (0,NULL);

	return CID_EXIT_SUCCESS;
	
}

