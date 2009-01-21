/*
   *
   *                  cid-utilities.c
   *                      -------
   *                Conky Images Display
   *             05/10/2008 - Benjamin SANS
   *             --------------------------
   *
*/

#include "cid.h"

/* Fonction de sortie en cas d'erreur, avec affichage d'un
   éventuel message d'erreur */

int cid_sortie(int code) {
	cid_disconnect_player ();
	
	if (cid->bRunning)
		gtk_main_quit ();
	return (code);
}

int cid_player_evaluation (gchar *player) {
	if (g_strcasecmp(player,"amarok")==0)
		return cid->iPlayer=PLAYER_AMAROK_1;
	else if (g_strcasecmp(player,"rhythmbox")==0)
		return cid->iPlayer=PLAYER_RHYTHMBOX;
	return -1;
}

void cid_disconnect_player () {
	//switch (cid->iPlayer) {
	//	case PLAYER_AMAROK_1:
	//		rhythmbox_dbus_disconnect_from_bus();
	//		break;
	//	case PLAYER_RHYTHMBOX:
	//		cid_disconnect_from_amarok();
	//		break;
	//}
	cid_disconnect_from_amarok();
	rhythmbox_dbus_disconnect_from_bus();
	cid_disconnect_from_exaile();
	amarok_2_dbus_disconnect_from_bus();
}

void cid_free_musicData(void) {
	if (musicData.playing_uri!=NULL)
		g_free (musicData.playing_uri);
	if (musicData.playing_artist!=NULL)
		g_free (musicData.playing_artist);
	if (musicData.playing_album!=NULL)
		g_free (musicData.playing_album);
	if (musicData.playing_title!=NULL)
		g_free (musicData.playing_title);
	if (musicData.playing_cover!=NULL)
		g_free (musicData.playing_cover);
		
	musicData.playing_uri = NULL;
	musicData.playing_artist = NULL;
	musicData.playing_album = NULL;
	musicData.playing_title = NULL;
	musicData.playing_cover = NULL;

	musicData.playing_track = 0;
	musicData.playing_duration = 0;
	musicData.iSidCheckCover = 0;

	musicData.cover_exist = FALSE;
	musicData.playing = FALSE;
	musicData.opening = FALSE;
}

/*
int cid_read_string(char* chaine) {
	if (strcmp(chaine, "-d\0" ) == 0 || strcmp(chaine, "--debug\0" ) == 0) return CID_DEBUG_MODE;
	if (strcmp(chaine, "-h\0" ) == 0 || strcmp(chaine, "--help\0" ) == 0 || strcmp(chaine, "-?\0" ) == 0) return CID_HELP_MENU;
	if (strcmp(chaine, "-T\0" ) == 0 || strcmp(chaine, "--testing\0" ) == 0) return CID_TESTING_MODE;
	if (strcmp(chaine, "-c\0" ) == 0) return CID_CHANGE_CONFIG_FILE;
	if (strcmp(chaine, "-v\0" ) == 0 || strcmp(chaine, "--version\0" ) == 0) return CID_GIVE_VERSION;
	if (strcmp(chaine, "-l\0" ) == 0 || strcmp(chaine, "--log\0" ) == 0) return CID_SET_VERBOSITY;
	return 0;
}
*/

void cid_read_parameters (int argc, char **argv) {
	
	GError *erreur=NULL;
	gboolean bPrintVersion = FALSE, bTestingMode = FALSE, bDebugMode = FALSE, bSafeMode = FALSE, bCafe = FALSE;
	
	GOptionEntry TableDesOptions[] =
	{
		{"log", 'l', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cid->pVerbosity,
			_("log verbosity (debug,info,message,warning,error) default is warning."), NULL},
		{"config", 'c', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME,
			&cid->pConfFile,
			_("load CID with this config file instead of ~/.config/cid/cid.conf."), NULL},
		{"testing", 'T', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bTestingMode,
			_("runs CID in testing mode. (some unstable options might be running)"), NULL},
		{"debug", 'd', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bDebugMode,
			_("runs CID in debug mode. (equivalent to '-l debug')"), NULL},
		{"safe", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bSafeMode,
			_("runs CID in safe mode."), NULL},
		{"cafe", 'C', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bCafe,
			_("do you want a cup of coffee ?"), NULL},
		{"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bPrintVersion,
			_("print version and quit."), NULL}
	};

	GOptionContext *context = g_option_context_new ("");
	g_option_context_set_summary (context,_("Conky Images Display is a programm written in C.\nIts goal is to display the cover of the song which is currently playing in the player you choose on the desktop like conky does with other informations.\nYou can use it with the following options:\n"));
	g_option_context_add_main_entries (context, TableDesOptions, NULL);
	g_option_context_parse (context, &argc, &argv, &erreur);
	
	if (erreur != NULL)
	{
		g_print ("ERROR : %s\n", erreur->message);
		exit (CID_ERROR_READING_ARGS);
	}
	
	if (bSafeMode) {
		cid->bSafeMode = TRUE;
		g_print ("Safe Mode\n");
	}
	
	if (bDebugMode) {
		g_free (cid->pVerbosity);
		cid->pVerbosity = g_strdup ("debug");
	}

	_cid_set_verbosity (cid->pVerbosity);

	if (bPrintVersion) {
		g_print ("Version: %s\n",CID_VERSION);
		exit (CID_EXIT_SUCCESS);
	}
	
	if (bCafe) {
		g_print ("Please insert coin.\n");
		exit (CID_EXIT_SUCCESS);
	}

	if (bTestingMode) {
		cid->bTesting = TRUE;
		cid->pConfFile = g_strdup_printf("%s/.config/cid/%s",g_getenv("HOME"),SVN_CONF_FILE);
	}
	
		int i=0;
	gboolean bEasterEggs = FALSE;
	for (; i<argc; i++) {
		if (strcmp(argv[i], "coin-coin" ) == 0) {
			g_print ("\
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMWXOdlcccldONMMMMMMMMMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMMMMO'         ..;xWMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMWl          .;;. ;XMMMMMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMMd                 .KMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMM:  ...      ...    :MMMMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMM: 'od:   'dkxd,    .NMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMc l.,O,  O0''lK'    KMMMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMMo c..ddllkx  ;K,    0MMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMx 'xxkO00OOkx0d.    xMMMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMMO.oxkO0000Okkkk.    :WMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMK ,xxxkkkxxkO0O' .;' cNMMMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMWc cK0OOOOO0KNWWX,     'XMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMK, ;KWWXKXXNWMMMMMX'     .OMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMWo  dMMMMMWWMMMMMMMMM0.      cNMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMX:  ,NMMMWWNWMMMMMWWWWWd       ,KMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMO.  .xXWWMMWNMMMMMMWNNNXNo  .     dMMMMMMMMMMM\nMMMMMMMMMMMMMX.  .kWMMMMMMMMMMMMMMMMMWNNo   .    kMMMMMMMMMM\n\
MMMMMMMMMMMMWc   OWMMMMMMMMMMMMMMMMMMMMMWc . .   .KMMMMMMMMM\nMMMMMMMMMMMWo   dMMMMMMMWWMMMMMMMMMMMMMMMX.   .   'NMMMMMMMM\n\
MMMMMMMMMMMd . ,NMMMMMMMNWMMMMMMMMMMMMMMMW'        dMMMMMMMM\nMMMMMMMMMN:  . dMMMMMMMMNWMMMMMMMMMMMMMMMM,        :MMMMMMMM\n\
MMMMMMMMMO   ..kMMMMMMMMNWMMMMMMMMMMMMMMMM,  .     :MMMMMMMM\nMMMMMMMMMKloxl;cKMMMMMMMNWMMMMMMMMMMMMMNXK'     ...xMMMMMMMM\n\
MMMMMMMWKddkkkkl':kNMMMMWWMMMMMMMMMMMMM0Ok;      ;kOWMMMMMMM\nMMMNXKOdodxkkkkkd. .dXMMMMMMMMMMMMMMMN0kkkl.....ckOONMMMMMMM\n\
MMNkkkkxxkkkkkkkkd'  .:XMMMMMMMMMMMMMXOxxkxolooxkOOkkKWMMMMM\nMMNxxkkkOOOkkkkkkxxc  .0MMMMMMMMMMMMMO;lxkkkkkkkOOOOOkO0XMMM\n\
MMXodkkOOOOOOOOOkkkxoxXMMMMMMMMMMMXk: .oxkOOOOOkkkkOOOOOOWMM\nMMKodxkkkOOOOOOOOOOkxdloOKXKK0kdl,    ,dkkOkkkkkkkxxkO0XWMMM\n\
MMXxdoooddddxxkkkkkkxoc.              ;dkkkkkkxxxk0XNWMMMMMM\nMMMMWNK0Okdollloodddolc;codxxxxxxxxddlcldxxxxxdkKWMMMMMMMMMM\n\
MMMMMMMMMMMWNX0xoccclxXMMMMMMMMMMMMMMMXdccllokKWMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMWNNWMMMMMMMMMMMMMMMMMMMWNNWWMMMMMMMMMMMMMMM\n\
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n");
			exit (CID_EXIT_SUCCESS);
		}
		if (strcmp(argv[i], "coin" ) == 0) {
			g_print("\
Congratulation ! You just found my Easter Egg :)  \n\
                        `                         \n\
                   :sdNMMMNhs:                    \n                 `hMMMMMMMMhsmm/                  \n                 yMMMMMMMMNmNMMM/                 \n                 NMmdmNMMNddmNMMN`                \n\
                 Nho+oMMy:+/+mMMM-                \n                 dsmd-dh+/Nd-yMMM/                \n                 hm+o:::::o+:mMMMo                \n                 sy+/::-::///dMNNd                \n\
                 +d/+//////:-yMdhNs`              \n                -mh.-::::-.` `hMNMMh.             \n              `oNy.  `.``     `dMMMMd:            \n             -hMh`   ``        .NMMMMNo`          \n\
            /NMN:`   ``     ``.`+NNNMMMd.         \n           -NMN/`              ``/NNmNMMd.        \n          `dNN/                   oNmNNMMh        \n          ymMo      `             `MMMmMMM/       \n\
        `yMmN`      `              NMMmMMMm       \n        oMNdd       `              mMNNMMMN       \n        /soyy-      `            `.mNNNNmmh       \n      `:+///+ys:`   `           .:/hMMMMms/`      \n\
  `--:++//////yNd+.            .://ohddh+:/-      \n  -////////////sNMm:           -++//+++//://:.    \n  -+///::://////oho.         `:hy+/:::/::::::/:.  \n  :o////::::://////-``````-/smMMo//://///////:.   \n\
  :++++++////////+odNmmmmNNMMMMNo/////////-.`     \n   ``.-:/+ooo++++osyo+////////+os++//++/.         \n           `.:/++/.             -/++/:.           \n                                                  \n\
What's that ? \n\
Hum... I'd say it's a kinda Penguin ! \n\
");
			exit (CID_EXIT_SUCCESS);
		}
		if (strcmp(argv[i], "dev" ) == 0) {
			g_print("/!\\ CAUTION /!\\\nDevelopment mode !\n");
			g_free (cid->pConfFile);
			g_free (cid->pDefaultImage);
			cid->pConfFile = g_strdup(TESTING_FILE);
			DEFAULT_IMAGE = g_strdup(TESTING_COVER);
			cid->bDevMode = TRUE;
		}
	}
}
	
void _cid_set_verbosity(gchar *cVerbosity)
{
	if (!cVerbosity)
		cid_log_set_level(G_LOG_LEVEL_WARNING);
	else if (!strcmp(cVerbosity, "debug"))
		cid_log_set_level(G_LOG_LEVEL_DEBUG);
	else if (!strcmp(cVerbosity, "info"))
		cid_log_set_level(G_LOG_LEVEL_INFO);
	else if (!strcmp(cVerbosity, "message"))
		cid_log_set_level(G_LOG_LEVEL_MESSAGE);
	else if (!strcmp(cVerbosity, "warning"))
		cid_log_set_level(G_LOG_LEVEL_WARNING);
	else if (!strcmp(cVerbosity, "error"))
		cid_log_set_level(G_LOG_LEVEL_ERROR);
	else {
		cid_log_set_level(G_LOG_LEVEL_WARNING);
		cid_warning("bad verbosity option: default to warning");
	}
}

void cid_play_sound (const gchar *cSoundPath) {
	cid_debug ("%s (%s)", __func__, cSoundPath);
	if (cSoundPath == NULL)
	{
		cid_warning ("No sound to play, halt.");
		return;
	}
	
	GError *erreur = NULL;
	gchar *cSoundCommand = NULL;
	if (g_file_test ("/usr/bin/play", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("play \"%s\"", cSoundPath);
		
	else if (g_file_test ("/usr/bin/aplay", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("aplay \"%s\"", cSoundPath);
	
	else if (g_file_test ("/usr/bin/paplay", G_FILE_TEST_EXISTS))
		cSoundCommand = g_strdup_printf("paplay \"%s\"", cSoundPath);
	
	cid_launch_command (cSoundCommand);
	
	g_free (cSoundCommand);
}

void cid_launch_web_browser (const gchar *cURL) {
	cid_debug ("%s (%s)", __func__, cURL);
	if (cURL == NULL)
	{
		cid_warning ("No web site to visit.");
		return;
	}
	
	GError *erreur = NULL;
	gchar *cURLCommand = NULL;
	if (g_file_test ("/usr/bin/firefox", G_FILE_TEST_EXISTS))
		cURLCommand = g_strdup_printf("firefox \"%s\"", cURL);
		
	else if (g_file_test ("/usr/bin/opera", G_FILE_TEST_EXISTS))
		cURLCommand = g_strdup_printf("opera \"%s\"", cURL);
	
	else if (g_file_test ("/usr/bin/safary", G_FILE_TEST_EXISTS))
		cURLCommand = g_strdup_printf("safary \"%s\"", cURL);
		
	else if (g_file_test ("/usr/bin/konqueror", G_FILE_TEST_EXISTS))
		cURLCommand = g_strdup_printf("konqueror \"%s\"", cURL);
	
	cid_launch_command (cURLCommand);
	
	g_free (cURLCommand);
}

gboolean cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...) {
	g_return_val_if_fail (cCommandFormat != NULL, FALSE);
	
	va_list args;
	va_start (args, cWorkingDirectory);
	gchar *cCommand = g_strdup_vprintf (cCommandFormat, args);
	va_end (args);
	cid_debug ("%s (%s , %s)", __func__, cCommand, cWorkingDirectory);
	
	gchar *cBGCommand;
	if (cCommand[strlen (cCommand)-1] != '&') {
		cBGCommand = g_strconcat (cCommand, " &", NULL);
		g_free (cCommand);
	}
	else
		cBGCommand = cCommand;
	GError *erreur = NULL;
	GThread* pThread = g_thread_create ((GThreadFunc) _cid_launch_threaded, cBGCommand, FALSE, &erreur);
	if (erreur != NULL)	{
		cid_warning ("couldn't launch this command (%s)", erreur->message);
		g_error_free (erreur);
		g_free (cBGCommand);
		return FALSE;
	}
	return TRUE;
}
