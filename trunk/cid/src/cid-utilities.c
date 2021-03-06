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
#include <errno.h>
#ifdef HAVE_LIBCRYPT
/* libC crypt */
#include "crypt.h"

static char DES_crypt_key[64] =
{
    1,0,0,1,1,1,0,0, 1,0,1,1,1,0,1,1, 1,1,0,1,0,1,0,1, 1,1,0,0,0,0,0,1,
    0,0,0,1,0,1,1,0, 1,1,1,0,1,1,1,0, 1,1,1,0,0,1,0,0, 1,0,1,0,1,0,1,1
}; 
#endif

extern CidMainContainer *cid;
extern int ret;

static Display *s_XDisplay = NULL;

/* Fonction de sortie en cas d'erreur, avec affichage d'un
   éventuel message d'erreur */
int 
cid_sortie(int code) 
{
    cid_disconnect_player ();
    
    if (cid->bRunning)
        gtk_main_quit ();
    ret = code;
    return code;
}

void 
cid_disconnect_player () 
{
    cid_disconnect_from_amarok();
    rhythmbox_dbus_disconnect_from_bus();
    cid_disconnect_from_exaile();
    amarok_2_dbus_disconnect_from_bus();
    cid_disconnect_from_mpd();
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
    if (pCid->p_cSurface)
        cairo_surface_destroy(pCid->p_cSurface);
    if (pCid->p_cPreviousSurface)
        cairo_surface_destroy(pCid->p_cPreviousSurface);
    if (pCid->p_cCross)
        cairo_surface_destroy(pCid->p_cCross);
    if (pCid->p_cPlay)
        cairo_surface_destroy(pCid->p_cPlay);
    if (pCid->p_cPause)
        cairo_surface_destroy(pCid->p_cPause);
    if (pCid->p_cPlay_big)
        cairo_surface_destroy(pCid->p_cPlay_big);
    if (pCid->p_cPause_big)
        cairo_surface_destroy(pCid->p_cPause_big);
    if (pCid->p_cNext)
        cairo_surface_destroy(pCid->p_cPrev);
    if (pCid->cConfFile)
        g_free(pCid->cConfFile);
    if (pCid->cVerbosity)
        g_free(pCid->cVerbosity);
        
    pCid->pWindow = NULL;
    pCid->p_cSurface = NULL;
    pCid->p_cPreviousSurface = NULL;
    pCid->p_cCross = NULL;
    pCid->p_cPlay = NULL;
    pCid->p_cPause = NULL;
    pCid->p_cPlay_big = NULL;
    pCid->p_cPause_big = NULL;
    pCid->p_cPrev = NULL;
    pCid->cConfFile = NULL;
    pCid->cVerbosity = NULL;
    
    g_free (pCid);
    pCid = NULL;
}
    
void
cid_copy_file (const gchar *cSrc, const gchar *cDst)
{
    if (!cSrc || !cDst)
    {
        cid_warning ("Unable to copy file due to missing field ! (%s,%s)",cSrc,cDst);
        return;
    }
    FILE *src = fopen (cSrc,"rb");
    if (!src)
    {
        cid_warning ("Unable to open file: %s",cSrc);
        return;
    }
    FILE *dst = fopen (cDst,"wb");
    if (!dst)
    {
        cid_warning ("Unable to open file: %s",cDst);
        return;
    }
    char buffer[256];
    while (fgets(buffer,256,src) != NULL)
    {
        fprintf(dst,buffer);
    }
    fclose (src);
    fclose (dst);
}

void
cid_remove_file (const gchar* cFilePath)
{
    if (remove(cFilePath) == -1)
    {
        cid_warning ("Error while removing %s: %s",cFilePath,strerror (errno));
    }
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
    
    GError *erreur = NULL;
    gboolean bPrintVersion = FALSE, bTestingMode = FALSE, bDebugMode = FALSE, bSafeMode = FALSE, bCafe = FALSE, bConfigPanel = FALSE;
    gchar *cConfFile = NULL;
    
    GOptionEntry TableDesOptions[] =
    {
        {"log", 'l', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
            &cid->cVerbosity,
            dgettext (CID_GETTEXT_PACKAGE, "log verbosity (debug,info,message,warning,error) default is warning."), NULL},
        {"config", 'c', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME,
            &cConfFile,
            dgettext (CID_GETTEXT_PACKAGE, "load CID with this config file instead of ~/.config/cid/cid.conf."), NULL},
        {"testing", 'T', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bTestingMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in testing mode. (some unstable options might be running)"), NULL},
        {"debug", 'd', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bDebugMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in debug mode. (equivalent to '-l debug')"), NULL},
        {"edit", 'e', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bConfigPanel,
            dgettext (CID_GETTEXT_PACKAGE, "open CID's configuration panel."), NULL},
        {"safe", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bSafeMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in safe mode."), NULL},
        {"cafe", 'C', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bCafe,
            dgettext (CID_GETTEXT_PACKAGE, "do you want a cup of coffee?"), NULL},
        {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bPrintVersion,
            dgettext (CID_GETTEXT_PACKAGE, "print version and quit."), NULL}
    };

    GOptionContext *context = g_option_context_new ("");
    g_option_context_set_summary (context,dgettext (CID_GETTEXT_PACKAGE, "Conky Images Display is a programm written in C.\n\
Its goal is to display the cover of the song which \
is currently playing in the player you chooseon the \
desktop like conky does with other informations.\n\
You can use it with the following options:\n"));
    g_option_context_add_main_entries (context, TableDesOptions, NULL);
    g_option_context_parse (context, &argc, &argv, &erreur);
    
    if (erreur != NULL) 
    {
        cid_exit (CID_ERROR_READING_ARGS, "ERROR : %s\n", erreur->message);
    }
    
    if (bSafeMode) 
    {
        cid->bSafeMode = TRUE;
        fprintf (stdout,"Safe Mode\n");
    }
    
    if (bConfigPanel)
    {
        cid->bConfigPanel = TRUE;
        cid->bSafeMode = TRUE;
    }
    
    if (bDebugMode) 
    {
        if (cid->cVerbosity)
            g_free (cid->cVerbosity);
        cid->cVerbosity = g_strdup ("debug");
    }

    if (bPrintVersion) 
    {
        gchar *env;
        env = g_malloc (20*sizeof(*env));
#ifdef HAVE_E17
                        snprintf (env,20,"e17");
#else
                        snprintf (env,20,"gnome and/or kde");
#endif
        fprintf (stdout,"Version: %s\n"
                        "Compiled for: %s\n"
                        ,CID_VERSION
                        ,env);
        g_free (env);
        exit (CID_EXIT_SUCCESS);
    }
    
    if (bCafe) 
    {
        fprintf (stdout,"Please insert coin.\n");
        exit (CID_EXIT_SUCCESS);
    }

    if (bTestingMode) 
    {
        cid->bTesting = TRUE;
    }
    
    int i=0;
    for (; i<argc; i++) 
    {
        if (strcmp(argv[i], "coin-coin" ) == 0) 
        {
            fprintf (stdout,COIN_COIN);
            exit (CID_EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "coin" ) == 0) 
        {
            fprintf (stdout,COIN);
            exit (CID_EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "dev" ) == 0) 
        {
            fprintf (stdout,"/!\\ CAUTION /!\\\nDevelopment mode !\n");
            if (cid->cConfFile)
                g_free (cid->cConfFile);
            if (DEFAULT_IMAGE)
                g_free (DEFAULT_IMAGE);
            cid->cConfFile = g_strdup_printf ("%s/%s", TESTING_DIR, TESTING_FILE);
            DEFAULT_IMAGE = g_strdup_printf ("%s/%s", TESTING_DIR, TESTING_COVER);
            cid->bDevMode = TRUE;
        }
    }
    
    if (cConfFile != NULL)
    {
        if (cid->cConfFile)
            g_free (cid->cConfFile);
        cid->cConfFile = g_strdup (cConfFile);
        g_free (cConfFile);
    }
}
    
void 
cid_set_verbosity (gchar *cVerbosity)
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
        switch (iType) 
        {
            case G_TYPE_STRING:
                ret->string = NULL;
                ret->string = g_malloc0(strlen((gchar *) value)*sizeof(gchar)+1);
                strcpy(ret->string, (gchar *) value);
                break;
            case G_TYPE_INT:
                ret->iNumber = (gint) value;
                break;
            case G_TYPE_BOOLEAN:
                ret->booleen = (gboolean) value;
                break;
            default:
                g_free(ret);
                return NULL;
        }
    }
    return ret;
}

gboolean
cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2)
{
    if (d1 == NULL || d2 == NULL)
        return FALSE;
    if (d1->type != d2->type)
        return FALSE;
    switch (d1->type) 
    {
        case G_TYPE_STRING:
            return g_strcmp0(d1->string,d2->string) == 0;
        case G_TYPE_INT:
            return d1->iNumber == d2->iNumber;
        case G_TYPE_BOOLEAN:
            return d1->booleen == d2->booleen;
    }
}

void
cid_datatable_append(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL) 
    {
        CidDataCase *p_new = cid_datacase_new(); 
        if (p_new != NULL) 
        {
            p_new->content = data; 
            p_new->next = NULL; 
            if ((*p_list)->tail == NULL)
            {
                p_new->prev = NULL; 
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->tail->next = p_new;
                p_new->prev = (*p_list)->tail;
                (*p_list)->tail = p_new;
            }
            (*p_list)->length++;
        }
    }
}

void
cid_datatable_prepend(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_new = cid_datacase_new();
        if (p_new != NULL)
        {
            p_new->content = data;
            p_new->prev = NULL;
            if ((*p_list)->tail == NULL)
            {
                p_new->next = NULL;
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->head->prev = p_new;
                p_new->next = (*p_list)->head;
                (*p_list)->head = p_new;
            }
            (*p_list)->length++;
       }
    }
}

void
cid_datatable_insert(CidDataTable **p_list, CidDataContent *data, gint position)
{
    if (*p_list != NULL)
    {
        if (position < 0)
        {
            cid_datatable_prepend(p_list,data);
            return;
        }
        else 
        if (position > (gint) cid_datatable_length(*p_list))
        {
            cid_datatable_append(p_list,data);
            return;
        }
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    cid_datatable_append(p_list, data);
                }
                else if (p_temp->prev == NULL)
                {
                    cid_datatable_prepend(p_list, data);
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
                        (*p_list)->length++;
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
}

void
cid_free_datacase_full (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL)
    {
        cid_free_datacontent(pCase->content);
        g_free(pCase);
    }
}

void
cid_free_datacontent_full (CidDataContent *pContent, gpointer *pData)
{
    if (pContent != NULL)
    {
        if (pContent->type == G_TYPE_STRING && pContent->string != NULL)
            g_free(pContent->string);
        g_free(pContent);
    }
}

void
cid_free_datatable (CidDataTable **p_list)
{
    if (*p_list != NULL)
    {
        cid_datatable_foreach(*p_list,(CidDataAction) cid_free_datacase_full, NULL);
        g_free(*p_list), *p_list = NULL;
    }
}

void
cid_datatable_foreach (CidDataTable *p_list, CidDataAction func, gpointer *pData)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        gboolean bCreateData = FALSE;
        if (pData == NULL)
        {
            pData = g_new(gpointer,1);
            bCreateData = TRUE;
        }
        gint cpt = 1;
        while (p_temp != NULL)
        {
            pData[0] = GINT_TO_POINTER(cpt);
            CidDataCase *p_del = g_malloc0(sizeof(*p_temp));
            memcpy(p_del,p_temp,sizeof(*p_temp));
            func (p_temp, pData);
            p_temp = p_del->next;
            g_free(p_del);
            cpt++;
        }
        if (bCreateData)
        {
            g_free(pData);
        }
    }
}

void
cid_datacase_print (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL && pCase->content != NULL)
    {
        switch (pCase->content->type) 
        {
            case G_TYPE_STRING:
                fprintf (stdout,"%s\n",pCase->content->string);
                break;
            case G_TYPE_INT:
                fprintf (stdout,"%d\n",pCase->content->iNumber);
                break;
            case G_TYPE_BOOLEAN:
                fprintf (stdout,"%s\n",pCase->content->booleen ? "TRUE" : "FALSE");
                break;
        }
    }
}

static void
cid_datacase_replace (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL)
    {
        gchar **c_tmp = pData[2];
        if (GPOINTER_TO_INT(pData[0]) < GPOINTER_TO_INT(pData[1]))
        {
            g_sprintf(*c_tmp,"%s%s%s",*c_tmp,pCase->content->string,pData[3]);
        } 
        else
        {
            g_sprintf(*c_tmp,"%s%s",*c_tmp,pCase->content->string);
        }
    }
}

void
cid_datatable_remove(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        gboolean found = FALSE;
        while (p_temp != NULL && !found)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                if (p_temp->next == NULL)
                {
                    cid_free_datacase((*p_list)->tail);
                    (*p_list)->tail = p_temp->prev;
                    cid_free_datacase((*p_list)->tail->next);
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    cid_free_datacase((*p_list)->head);
                    (*p_list)->head = p_temp->next;
                    cid_free_datacase((*p_list)->tail->prev);
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
                found = TRUE;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_all(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        while (p_temp != NULL)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                CidDataCase *p_del = p_temp;
                p_temp = p_temp->next;
                if (p_del->next == NULL)
                {
                    (*p_list)->tail = p_del->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_del->prev == NULL)
                {
                    (*p_list)->head = p_del->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_del->next->prev = p_del->prev;
                    p_del->prev->next = p_del->next;
                }
                cid_free_datacase(p_del);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_id(CidDataTable **p_list, gint position)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    (*p_list)->tail = p_temp->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    (*p_list)->head = p_temp->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
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
    GType iCurrType = iDataType;
    va_list args;
    va_start(args,iDataType);
    void *current;
    while ((current = va_arg(args,void *)) != G_TYPE_INVALID) {
        CidDataContent *tmp = NULL;
        if ((GType) current == G_TYPE_BOOLEAN || (GType) current == G_TYPE_INT || (GType) current == G_TYPE_STRING)
        {
            iCurrType = (GType) current;
            continue;
        }
        switch (iCurrType) 
        {
            case G_TYPE_BOOLEAN:
                tmp = cid_datacontent_new_boolean(current);
                break;
            case G_TYPE_STRING:
                tmp = cid_datacontent_new_string(current);
                break;
            case G_TYPE_INT:
                tmp = cid_datacontent_new_int(current);
                break;
            default:
                iCurrType = (GType) current;
        }
        cid_datatable_append(&res,tmp);
    }
    va_end(args);
    return res;
}

CidDataTable *
cid_create_sized_datatable_with_default_full (size_t iSize, GType iType, void *value)
{
    size_t cpt = 0;
    CidDataTable *res = cid_datatable_new();
    for (;cpt<iSize;cpt++)
    {
        CidDataContent *tmp = cid_datacontent_new(iType, value);
        cid_datatable_append(&res,tmp);
    }
    return res;
}

void
cid_str_replace_all (gchar **string, const gchar *sFrom, const gchar *sTo)
{
    if (*string == NULL)
        return;
    gchar **tmp = g_strsplit(*string,sFrom,0);
    CidDataTable *t_temp = cid_datatable_new();
    while (*tmp != NULL)
    {
        cid_datatable_append(&t_temp,cid_datacontent_new_string(*tmp));
        tmp++;
    }
    size_t size = cid_datatable_length(t_temp);
    if (size < 2)
    {
        cid_free_datatable(&t_temp);
        return;
    }
    *string = g_malloc0((strlen(*string)+((strlen(sTo)-strlen(sFrom))*size))*sizeof(gchar)+1);
    gpointer *pData = g_new(gpointer,4);
    pData[0] = GINT_TO_POINTER(0);
    pData[1] = GINT_TO_POINTER(size);
    pData[2] = string;
    pData[3] = (gchar *)g_strdup(sTo);
    cid_datatable_foreach(t_temp,(CidDataAction)cid_datacase_replace,pData);
    cid_free_datatable(&t_temp);
    g_free (pData[3]);
    g_free (pData);
}

void
cid_str_replace_all_seq (gchar **string, gchar *seqFrom, gchar *seqTo)
{
//    if (strlen(seqFrom) != strlen(seqTo))
//        return;
    /*
    while (*seqFrom != '\0' && *seqTo != '\0')
    {
        gchar *from = g_malloc0(2*sizeof(gchar)), *to = g_malloc0(2*sizeof(gchar));
        g_sprintf(from,"%c",*seqFrom);
        g_sprintf(to,"%c",*seqTo);
        g_print("from: %s, to: %s\n",from,to);
        cid_str_replace_all (string,from,to);
        g_free(from), from = NULL;
        g_free(to), to = NULL;
        seqFrom++;
        seqTo++;
    }
    */
    for(;*seqFrom != '\0';seqFrom++)
    {
        fprintf (stdout,"%c\n",*seqFrom);
    }
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

void 
cid_decrypt_string( const gchar *cEncryptedString,  gchar **cDecryptedString )
{
    if( !cEncryptedString || *cEncryptedString == '\0' )
    {
        *cDecryptedString = g_strdup( "" );
        return;
    }
#ifdef HAVE_LIBCRYPT
    gchar *input = g_strdup(cEncryptedString);
    gchar *shifted_input = input;
    gchar **output = cDecryptedString; 

    gchar *current_output = NULL;
    if( output && input && strlen(input)>0 )
    {
        *output = g_malloc( (strlen(input)+1)/3+1 );
        current_output = *output;
    }
    else
    {
        if( input )
        {
            g_free(input);
        }
        return;
    }

    gchar *last_char_in_input = input + strlen(input);
//  g_print( "Password (before decrypt): %s\n", input );

    for( ; shifted_input < last_char_in_input; shifted_input += 16+8, current_output += 8 )
    {
        guint block[8];
        gchar txt[64];
        gint i = 0, j = 0;
        gchar current_letter = 0;
    
        memset( txt, 0, 64 );

        shifted_input[16+8-1] = 0; // cut the string

        sscanf( shifted_input, "%X-%X-%X-%X-%X-%X-%X-%X",
        &block[0], &block[1], &block[2], &block[3], &block[4], &block[5], &block[6], &block[7] );

        // process the eight first characters of "input"
        for( i = 0; i < 8 ; i++ )
            for ( j = 0; j < 8; j++ )
                txt[i*8+j] = block[i] >> j & 1;
    
        setkey( DES_crypt_key );
        encrypt( txt, 1 );  // decrypt

        for ( i = 0; i < 8; i++ )
        {
            current_output[i] = 0;
            for ( j = 0; j < 8; j++ )
            {
                current_output[i] |= txt[i*8+j] << j;
            }
        }
    }

    *current_output = 0;

//  g_print( "Password (after decrypt): %s\n", *output );

    g_free( input );

#else
    *cDecryptedString = g_strdup( cEncryptedString );
#endif
}

void 
cid_encrypt_string( const gchar *cDecryptedString,  gchar **cEncryptedString )
{
    if( !cDecryptedString || strlen(cDecryptedString) == 0 )
    {
        *cEncryptedString = g_strdup( "" );
        return;
    }
    
#ifdef HAVE_LIBCRYPT
    const gchar *input = cDecryptedString;
    gchar **output = cEncryptedString;
    guint input_length = 0;

    gchar *current_output = NULL;
    if( output && input && strlen(input)>0 )
    {
        // for each block of 8 characters, we need 24 bytes.
        guint nbBlocks = strlen(input)/8+1;
    
        *output = g_malloc( nbBlocks*24+1 );
        current_output = *output;
    }
    else
    {
        return;
    }

    const gchar *last_char_in_input = input + strlen(input);

//  g_print( "Password (before encrypt): %s\n", input );

    for( ; input < last_char_in_input; input += 8, current_output += 16+8 )
    {
        gchar txt[64];
        gint i = 0, j = 0;
        gchar current_letter = 0;
    
        memset( txt, 0, 64 );
    
        // process the eight first characters of "input"
        for( i = 0; i < strlen(input) && i < 8 ; i++ )
            for ( j = 0; j < 8; j++ )
                txt[i*8+j] = input[i] >> j & 1;
    
        setkey( DES_crypt_key );
        encrypt( txt, 0 );  // encrypt

        for ( i = 0; i < 8; i++ )
        {
            current_letter = 0;
            for ( j = 0; j < 8; j++ )
            {
                current_letter |= txt[i*8+j] << j;
            }
            snprintf( current_output + i*3, 4, "%02X-", current_letter );
        }
    }

    *(current_output-1) = 0;

//  g_print( "Password (after encrypt): %s\n", *output );
#else
    *cEncryptedString = g_strdup( cDecryptedString );
#endif
}

gchar *
_url_encode (const gchar * str)
{
    const gchar * s = str;
    char * t = NULL;
    char * ret;
    char * validChars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.!~*'()";
    char * isValidChar;
    int lenght = 0;
    // calcul de la taille de la chaine urlEncodée
    do{
        isValidChar = (char *) strchr(validChars, *s); // caractère valide?
        if(!isValidChar)
            lenght+=3; // %xx : 3 caractères
        else
            lenght++;  // sinon un seul
    }while(*++s); // avance d'un cran dans la chaine. Si on est pas à la fin, on continue...
    s = str;
    t = g_new (gchar, lenght + 1); // Allocation à la bonne taille
    ret = t;
    //encodage
    do{
        isValidChar = (char *) strchr(validChars, *s);
        if(!isValidChar)
            sprintf(t, "%%%2X", *s), t+=3;
        else
            sprintf(t, "%c", *s), t++;
    }while(*++s);
    *t = 0; // 0 final
    return ret;
}
