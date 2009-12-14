#include <stdio.h>

#include "cid-console-call.h"

/***************************************************************************/
/// Fonctions de lecture de pipe 

FILE *popen(const char *command, const char *type);

int pclose(FILE *stream);   

static gchar *
setMessage(const gchar *cCommand, gint iCode) 
{
    return g_strdup_printf(CID_CONSOLE_MESSAGES[iCode-1],cCommand);
}

static void 
setError (CIDError **error, gint iCode, const gchar *cCommand) 
{
    if (*error)
        cid_free_error(*error);
    *error = g_new0(CIDError,1);
    (*error)->message = setMessage(cCommand,iCode);
    (*error)->code    = iCode;
    
}

static gchar *
cleanCmd (const gchar *cCommand) 
{
    return g_strdup_printf("%s >/dev/null 2>&1",cCommand);
}

gchar *
cid_console_get_string_with_error_full (const gchar *cCommand, gchar *cDefault, CIDError **error) 
{
    gchar *cClean = cleanCmd(cCommand);
    FILE *pPipe = popen (cClean,"r");
    g_free (cClean);
    if (!pPipe) 
    {
        setError(error,CID_CONSOLE_UNREACHABLE, cCommand);
        return cDefault;
    }
    gchar *cRead = (gchar *) malloc (512*sizeof(gchar));
    if (!fgets(cRead,512,pPipe)) 
    {
        setError(error,CID_CONSOLE_CANT_READ_PIPE, cCommand);
        g_free(cRead);
        pclose(pPipe);
        return cDefault;
    }
    pclose(pPipe);
    strtok (cRead,"\n");
    return cRead;
}

gint 
cid_console_get_int_with_error_full (const gchar *cCommand, gint iDefault, CIDError **error) 
{
    gchar *cClean = cleanCmd(cCommand);
    FILE *pPipe = popen (cClean,"r");
    g_free (cClean);
    if (!pPipe) 
    {
        setError(error,CID_CONSOLE_UNREACHABLE, cCommand);
        return iDefault;
    }
    gchar *cRead = (gchar *) malloc (128*sizeof(gchar));
    if (!fgets(cRead,128,pPipe)) 
    {
        setError(error,CID_CONSOLE_CANT_READ_PIPE, cCommand);
        g_free(cRead);
        pclose(pPipe);
        return iDefault;
    }
    pclose(pPipe);
    gint iRet = (gint)atoi(cRead);
    g_free(cRead);
    return iRet;
}

gboolean 
cid_console_get_boolean_with_error_full (const gchar *cCommand, gboolean bDefault, CIDError **error) 
{
    gchar *cClean = cleanCmd(cCommand);
    FILE *pPipe = popen (cClean,"r");
    g_free (cClean);
    if (!pPipe) 
    {
        setError(error,CID_CONSOLE_UNREACHABLE, cCommand);
        return bDefault;
    }
    gchar *cRead = (gchar *) malloc (64*sizeof(gchar));
    if (!fgets(cRead,64,pPipe)) 
    {
        setError(error,CID_CONSOLE_CANT_READ_PIPE, cCommand);
        g_free(cRead);
        pclose(pPipe);
        return bDefault;
    }
    pclose(pPipe);
    strtok (cRead,"\n");
    gboolean bRet = g_strcasecmp(cRead,"true")==0;
    g_free(cRead);
    return bRet;
}
/***************************************************************************/

void 
cid_free_error (CIDError *error) 
{
    error->code = -1;
    if (error->message)
        g_free (error->message);
    g_free (error);
    error = NULL;
}
