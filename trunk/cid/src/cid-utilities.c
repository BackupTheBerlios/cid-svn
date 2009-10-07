/*
   *
   *                  cid-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-utilities.h"
#include "cid-struct.h"
#include "cid-callbacks.h"
#include "cid-messages.h"
#include "cid-constantes.h"

#include <X11/Xlib.h>

extern CidMainContainer *cid;

static Display *s_XDisplay = NULL;

/* Fonction de sortie en cas d'erreur, avec affichage d'un
   éventuel message d'erreur */
int 
cid_sortie(int code) 
{
    cid_disconnect_player ();
    
    if (cid->bRunning)
        gtk_main_quit ();
    return (code);
}

void 
cid_disconnect_player () 
{
    cid_disconnect_from_amarok();
    rhythmbox_dbus_disconnect_from_bus();
    cid_disconnect_from_exaile();
    amarok_2_dbus_disconnect_from_bus();
}

void 
cid_free_musicData(void) 
{
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

void 
cid_free_main_structure (CidMainContainer *pCid) 
{
    if (pCid->pWindow)
        gtk_widget_destroy(pCid->pWindow);
    if (pCid->cSurface)
        cairo_surface_destroy(pCid->cSurface);
    if (pCid->cPreviousSurface)
        cairo_surface_destroy(pCid->cPreviousSurface);
    if (pCid->cCross)
        cairo_surface_destroy(pCid->cCross);
    if (pCid->cPlay)
        cairo_surface_destroy(pCid->cPlay);
    if (pCid->cPause)
        cairo_surface_destroy(pCid->cPause);
    if (pCid->cPlay_big)
        cairo_surface_destroy(pCid->cPlay_big);
    if (pCid->cPause_big)
        cairo_surface_destroy(pCid->cPause_big);
    if (pCid->cNext)
        cairo_surface_destroy(pCid->cPrev);
    if (pCid->pConfFile)
        g_free(pCid->pConfFile);
    if (pCid->pVerbosity)
        g_free(pCid->pVerbosity);
        
    pCid->pWindow = NULL;
    pCid->cSurface = NULL;
    pCid->cPreviousSurface = NULL;
    pCid->cCross = NULL;
    pCid->cPlay = NULL;
    pCid->cPause = NULL;
    pCid->cPlay_big = NULL;
    pCid->cPause_big = NULL;
    pCid->cPrev = NULL;
    pCid->pConfFile = NULL;
    pCid->pVerbosity = NULL;
    
    g_free (pCid);
    pCid = NULL;
}
    

/*
int 
cid_read_string(char* chaine) 
{
    if (strcmp(chaine, "-d\0" ) == 0 || strcmp(chaine, "--debug\0" ) == 0) return CID_DEBUG_MODE;
    if (strcmp(chaine, "-h\0" ) == 0 || strcmp(chaine, "--help\0" ) == 0 || strcmp(chaine, "-?\0" ) == 0) return CID_HELP_MENU;
    if (strcmp(chaine, "-T\0" ) == 0 || strcmp(chaine, "--testing\0" ) == 0) return CID_TESTING_MODE;
    if (strcmp(chaine, "-c\0" ) == 0) return CID_CHANGE_CONFIG_FILE;
    if (strcmp(chaine, "-v\0" ) == 0 || strcmp(chaine, "--version\0" ) == 0) return CID_GIVE_VERSION;
    if (strcmp(chaine, "-l\0" ) == 0 || strcmp(chaine, "--log\0" ) == 0) return CID_SET_VERBOSITY;
    return 0;
}
*/

void 
cid_read_parameters (int argc, char **argv) 
{
    
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
    g_option_context_set_summary (context,_("Conky Images Display is a programm written in C.\n\
Its goal is to display the cover of the song which \
is currently playing in the player you chooseon the \
desktop like conky does with other informations.\n\
You can use it with the following options:\n"));
    g_option_context_add_main_entries (context, TableDesOptions, NULL);
    g_option_context_parse (context, &argc, &argv, &erreur);
    
    if (erreur != NULL) 
    {
        g_print ("ERROR : %s\n", erreur->message);
        exit (CID_ERROR_READING_ARGS);
    }
    
    if (bSafeMode) 
    {
        cid->bSafeMode = TRUE;
        g_print ("Safe Mode\n");
    }
    
    if (bDebugMode) 
    {
        g_free (cid->pVerbosity);
        cid->pVerbosity = g_strdup ("debug");
    }

    _cid_set_verbosity (cid->pVerbosity);

    if (bPrintVersion) 
    {
        g_print ("Version: %s\n",CID_VERSION);
        exit (CID_EXIT_SUCCESS);
    }
    
    if (bCafe) 
    {
        g_print ("Please insert coin.\n");
        exit (CID_EXIT_SUCCESS);
    }

    if (bTestingMode) 
    {
        cid->bTesting = TRUE;
    }
    
    int i=0;
    gboolean bEasterEggs = FALSE;
    for (; i<argc; i++) 
    {
        if (strcmp(argv[i], "coin-coin" ) == 0) 
        {
            g_print ("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMWXOdlcccldONMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMO'         ..;xWMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMWl          .;;. ;XMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMd                 .KMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMM:  ...      ...    :MMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMM: 'od:   'dkxd,    .NMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMc l.,O,  O0''lK'    KMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMo c..ddllkx  ;K,    0MMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMx 'xxkO00OOkx0d.    xMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMO.oxkO0000Okkkk.    :WMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMK ,xxxkkkxxkO0O' .;' cNMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMWc cK0OOOOO0KNWWX,     'XMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMK, ;KWWXKXXNWMMMMMX'     .OMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMWo  dMMMMMWWMMMMMMMMM0.      cNMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMX:  ,NMMMWWNWMMMMMWWWWWd       ,KMMMMMMMMMMMM\nMMMMMMMMMMMMMMO.  .xXWWMMWNMMMMMMWNNNXNo  .     dMMMMMMMMMMM\nMMMMMMMMMMMMMX.  .kWMMMMMMMMMMMMMMMMMWNNo   .    kMMMMMMMMMM\nMMMMMMMMMMMMWc   OWMMMMMMMMMMMMMMMMMMMMMWc . .   .KMMMMMMMMM\nMMMMMMMMMMMWo   dMMMMMMMWWMMMMMMMMMMMMMMMX.   .   'NMMMMMMMM\nMMMMMMMMMMMd . ,NMMMMMMMNWMMMMMMMMMMMMMMMW'        dMMMMMMMM\nMMMMMMMMMN:  . dMMMMMMMMNWMMMMMMMMMMMMMMMM,        :MMMMMMMM\nMMMMMMMMMO   ..kMMMMMMMMNWMMMMMMMMMMMMMMMM,  .     :MMMMMMMM\nMMMMMMMMMKloxl;cKMMMMMMMNWMMMMMMMMMMMMMNXK'     ...xMMMMMMMM\nMMMMMMMWKddkkkkl':kNMMMMWWMMMMMMMMMMMMM0Ok;      ;kOWMMMMMMM\nMMMNXKOdodxkkkkkd. .dXMMMMMMMMMMMMMMMN0kkkl.....ckOONMMMMMMM\nMMNkkkkxxkkkkkkkkd'  .:XMMMMMMMMMMMMMXOxxkxolooxkOOkkKWMMMMM\nMMNxxkkkOOOkkkkkkxxc  .0MMMMMMMMMMMMMO;lxkkkkkkkOOOOOkO0XMMM\nMMXodkkOOOOOOOOOkkkxoxXMMMMMMMMMMMXk: .oxkOOOOOkkkkOOOOOOWMM\nMMKodxkkkOOOOOOOOOOkxdloOKXKK0kdl,    ,dkkOkkkkkkkxxkO0XWMMM\nMMXxdoooddddxxkkkkkkxoc.              ;dkkkkkkxxxk0XNWMMMMMM\nMMMMWNK0Okdollloodddolc;codxxxxxxxxddlcldxxxxxdkKWMMMMMMMMMM\nMMMMMMMMMMMWNX0xoccclxXMMMMMMMMMMMMMMMXdccllokKWMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMWNNWMMMMMMMMMMMMMMMMMMMWNNWWMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n");
            exit (CID_EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "coin" ) == 0) 
        {
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
        if (strcmp(argv[i], "dev" ) == 0) 
        {
            g_print("/!\\ CAUTION /!\\\nDevelopment mode !\n");
            //g_free (cid->pConfFile);
            //g_free (cid->pDefaultImage);
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

void 
cid_play_sound (const gchar *cSoundPath) 
{
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

void 
cid_launch_web_browser (const gchar *cURL) 
{
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

gboolean 
cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...) 
{
    g_return_val_if_fail (cCommandFormat != NULL, FALSE);
    
    va_list args;
    va_start (args, cWorkingDirectory);
    gchar *cCommand = g_strdup_vprintf (cCommandFormat, args);
    va_end (args);
    cid_debug ("%s (%s , %s)", __func__, cCommand, cWorkingDirectory);
    
    gchar *cBGCommand;
    if (cCommand[strlen (cCommand)-1] != '&') 
    {
        cBGCommand = g_strconcat (cCommand, " &", NULL);
        g_free (cCommand);
    }
    else
        cBGCommand = cCommand;
    GError *erreur = NULL;
    GThread* pThread = g_thread_create ((GThreadFunc) _cid_launch_threaded, cBGCommand, FALSE, &erreur);
    if (erreur != NULL) 
    {
        cid_warning ("couldn't launch this command (%s)", erreur->message);
        g_error_free (erreur);
        g_free (cBGCommand);
        return FALSE;
    }
    return TRUE;
}

gchar *
cid_toupper (gchar *cSrc) 
{
    register int t;
    gchar *cRes = (gchar *)malloc (sizeof(gchar)*strlen(cSrc));

    for(t=0; cSrc[t]; ++t)  
    {
        cRes[t] = toupper(cSrc[t]);
    }
    return cRes;
}

CidDataTable *
cid_datatable_new (void)
{
    CidDataTable *res = g_new0(CidDataTable,1);
    if (res != NULL)
    {
        res->type = G_TYPE_STRING;
        res->length = 0;
        res->head = NULL;
        res->tail = NULL;
    }
    return res;
}

CidDataCase *
cid_datacase_new (void)
{
    CidDataCase *ret = g_new0(CidDataCase,1);
    if (ret != NULL) 
    {
        ret->content = NULL;
        ret->next = NULL;
        ret->prev = NULL;
    }
    return ret;
}

CidDataContent *
cid_datacontent_new (GType iType, void *value)
{
    CidDataContent *ret = g_new0(CidDataContent,1);
    if (ret != NULL)
    {
        ret->type = iType;
        switch (iType) {
            case G_TYPE_STRING:
                ret->value.string = (gchar *) value;
            case G_TYPE_INT:
                ret->value.iNumber = (gint) value;
            case G_TYPE_BOOLEAN:
                ret->value.booleen = (gboolean) value;
            default:
                return NULL;
        }
    }
    return ret;
}

gboolean
cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2)
{
    g_return_val_if_fail(d1 == NULL || d2 == NULL,FALSE);
    if (d1->type != d2->type)
        return FALSE;
    switch (d1->type) {
        case G_TYPE_STRING:
            return g_strcmp0(d1->value.string,d2->value.string) == 0;
        case G_TYPE_INT:
            return d1->value.iNumber == d2->value.iNumber;
        case G_TYPE_BOOLEAN:
            return d1->value.booleen == d2->value.booleen;
    }
}

CidDataTable *
cid_datatable_append(CidDataTable *p_list, CidDataContent *data)
{
    if (p_list != NULL) /* On vérifie si notre liste a été allouée */
    {
        CidDataCase *p_new = cid_datacase_new(); /* Création d'un nouveau node */
        if (p_new != NULL) /* On vérifie si le malloc n'a pas échoué */
        {
            p_new->content = data; /* On 'enregistre' notre donnée */
            p_new->next = NULL; /* On fait pointer p_next vers NULL */
            if (p_list->tail == NULL) /* Cas où notre liste est vide (pointeur vers fin de liste à  NULL) */
            {
                p_new->prev = NULL; /* On fait pointer p_prev vers NULL */
                p_list->head = p_new; /* On fait pointer la tête de liste vers le nouvel élément */
                p_list->tail = p_new; /* On fait pointer la fin de liste vers le nouvel élément */
            }
            else /* Cas où des éléments sont déjà présents dans notre liste */
            {
                p_list->tail->next = p_new; /* On relie le dernier élément de la liste vers notre nouvel élément (début du chaînage) */
                p_new->prev = p_list->tail; /* On fait pointer p_prev vers le dernier élément de la liste */
                p_list->tail = p_new; /* On fait pointer la fin de liste vers notre nouvel élément (fin du chaînage: 3 étapes) */
            }
            p_list->length++; /* Incrémentation de la taille de la liste */
        }
    }
    return p_list; /* on retourne notre nouvelle liste */
}

CidDataTable *
cid_datatable_prepend(CidDataTable *p_list, CidDataContent *data)
{
    if (p_list != NULL)
    {
        CidDataCase *p_new = cid_datacase_new();
        if (p_new != NULL)
        {
            p_new->content = data;
            p_new->prev = NULL;
            if (p_list->tail == NULL)
            {
                p_new->next = NULL;
                p_list->head = p_new;
                p_list->tail = p_new;
            }
            else
            {
                p_list->head->prev = p_new;
                p_new->next = p_list->head;
                p_list->head = p_new;
            }
            p_list->length++;
       }
    }
    return p_list;
}

CidDataTable *
cid_datatable_insert(CidDataTable *p_list, CidDataContent *data, int position)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    p_list = cid_datatable_append(p_list, data);
                }
                else if (p_temp->prev == NULL)
                {
                    p_list = cid_datatable_prepend(p_list, data);
                }
                else
                {
                    CidDataCase *p_new = cid_datacase_new();
                    if (p_new != NULL)
                    {
                        p_new->content = data;
                        p_temp->next->prev = p_new;
                        p_temp->prev->next = p_new;
                        p_new->prev = p_temp->prev;
                        p_new->next = p_temp;
                        p_list->length++;
                    }
                }
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
    return p_list;
}

void
cid_delete_datatable(CidDataTable **p_list)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        while (p_temp != NULL)
        {
            //printf("%s\n",p_temp->content->string);
            CidDataCase *p_del = p_temp;
            p_temp = p_temp->next;
            free(p_del);
        }
        free(*p_list), *p_list = NULL;
    }
}

void
cid_datatable_foreach (CidDataTable *p_list, CidDataAction func)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        while (p_temp != NULL)
        {
            func (p_temp);
            p_temp = p_temp->next;
        }
    }
}

void
cid_datacase_print (CidDataCase *pCase)
{
    if (pCase != NULL)
    {
        switch (pCase->content->type) {
            case G_TYPE_STRING:
                g_print ("%s\n",pCase->content->value.string);
            case G_TYPE_INT:
                g_print ("%d\n",pCase->content->value.iNumber);
            case G_TYPE_BOOLEAN:
                g_print ("%s\n",pCase->content->value.booleen ? "TRUE" : "FALSE");
        }
    }
}

CidDataTable *
cid_datatable_remove(CidDataTable *p_list, CidDataContent *data)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        int found = 0;
        while (p_temp != NULL && !found)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                if (p_temp->next == NULL)
                {
                    p_list->tail = p_temp->prev;
                    p_list->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    p_list->head = p_temp->next;
                    p_list->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                free(p_temp);
                p_list->length--;
                found = 1;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
    return p_list;
}

CidDataTable *
cid_datatable_remove_all(CidDataTable *p_list, CidDataContent *data)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        while (p_temp != NULL)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                CidDataCase *p_del = p_temp;
                p_temp = p_temp->next;
                if (p_del->next == NULL)
                {
                    p_list->tail = p_del->prev;
                    p_list->tail->next = NULL;
                }
                else if (p_del->prev == NULL)
                {
                    p_list->head = p_del->next;
                    p_list->head->prev = NULL;
                }
                else
                {
                    p_del->next->prev = p_del->prev;
                    p_del->prev->next = p_del->next;
                }
                free(p_del);
                p_list->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
    return p_list;
}

CidDataTable *
cid_datatable_remove_id(CidDataTable *p_list, int position)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    p_list->tail = p_temp->prev;
                    p_list->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    p_list->head = p_temp->next;
                    p_list->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                free(p_temp);
                p_list->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
    return p_list;
}

size_t
cid_datatable_length(CidDataTable *p_list)
{
    size_t ret = 0;
    if (p_list != NULL)
    {
        ret = p_list->length;
    }
    return ret;
}


CidDataTable *
cid_create_datatable (GType iDataType, ...)
{
    CidDataTable *res = cid_datatable_new();
    res->type = iDataType;
    va_list args;
    va_start(args,iDataType);
    void *current;
    while ((current = va_arg(args,void *)) != NULL) {
        CidDataContent *tmp = NULL;
        switch (iDataType) {
            case G_TYPE_BOOLEAN:
                tmp = cid_datacontent_new_boolean(current);
            case G_TYPE_STRING:
                tmp = cid_datacontent_new_string(current);
            case G_TYPE_INT:
                tmp = cid_datacontent_new_int(current);
        }
        res = cid_datatable_append(res,tmp);
    }
    return res;
}

static int 
_cid_xerror_handler (Display * pDisplay, XErrorEvent *pXError) 
{
    cid_debug ("Erreur (%d, %d, %d) lors d'une requete X sur %d", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
    return 0;
}

static void 
cid_get_X_infos (void) 
{
    s_XDisplay = XOpenDisplay (0);
    
    XSetErrorHandler (_cid_xerror_handler);
    
    Screen *XScreen = XDefaultScreenOfDisplay (s_XDisplay);
    cid->XScreenWidth  = WidthOfScreen (XScreen);
    cid->XScreenHeight = HeightOfScreen (XScreen);
    
    //g_print ("%dx%d\n",XScreenWidth,XScreenHeight);
}

void 
cid_check_position (void) 
{
    cid_get_X_infos();
    if (cid->iPosX > (cid->XScreenWidth - cid->iWidth)) cid->iPosX = (cid->XScreenWidth - cid->iWidth);
    if (cid->iPosY > (cid->XScreenHeight - cid->iHeight)) cid->iPosY = (cid->XScreenHeight - cid->iHeight);
    if (cid->iPosX < 0) cid->iPosX = 0;
    if (cid->iPosY < 0) cid->iPosY = 0;
}
