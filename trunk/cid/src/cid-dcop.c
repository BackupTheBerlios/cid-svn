#include <stdio.h>

#include "cid-dcop.h"

/***************************************************************************/
/// Fonctions de lecture de pipe dcop

FILE *popen(const char *command, const char *type);

int pclose(FILE *stream);   

static gchar *setMessage(const gchar *cCommand, gint iCode) {
    return g_strdup_printf(CID_DCOP_MESSAGES[iCode-1],cCommand);
}

static void setError (CIDError **error, gint iCode, const gchar *cCommand) {
    if (*error)
        cid_free_error(*error);
    *error = g_new0(CIDError,1);
    (*error)->message = setMessage(cCommand,iCode);
    (*error)->code    = iCode;
    
}

gchar *cid_dcop_get_string_with_error_full (const gchar *cCommand, gchar *cDefault, CIDError **error) {
    FILE *pPipe = popen (cCommand,"r");
    if (!pPipe) {
        setError(error,CID_DCOP_UNREACHABLE, cCommand);
        return cDefault;
    }
    gchar *cRead = (gchar *) malloc (512*sizeof(gchar));
    if (!fgets(cRead,512,pPipe)) {
        setError(error,CID_DCOP_CANT_READ_PIPE, cCommand);
        g_free(cRead);
        pclose(pPipe);
        return cDefault;
    }
    pclose(pPipe);
    strtok (cRead,"\n");
    return cRead;
}

gint cid_dcop_get_int_with_error_full (const gchar *cCommand, gint iDefault, CIDError **error) {
    FILE *pPipe = popen (cCommand,"r");
    if (!pPipe) {
        setError(error,CID_DCOP_UNREACHABLE, cCommand);
        return iDefault;
    }
    gchar *cRead = (gchar *) malloc (128*sizeof(gchar));
    if (!fgets(cRead,128,pPipe)) {
        setError(error,CID_DCOP_CANT_READ_PIPE, cCommand);
        g_free(cRead);
        pclose(pPipe);
        return iDefault;
    }
    pclose(pPipe);
    gint iRet = (gint)atoi(cRead);
    g_free(cRead);
    return iRet;
}

gboolean cid_dcop_get_boolean_with_error_full (const gchar *cCommand, gboolean bDefault, CIDError **error) {
    FILE *pPipe = popen (cCommand,"r");
    if (!pPipe) {
        setError(error,CID_DCOP_UNREACHABLE, cCommand);
        return bDefault;
    }
    gchar *cRead = (gchar *) malloc (64*sizeof(gchar));
    if (!fgets(cRead,64,pPipe)) {
        setError(error,CID_DCOP_CANT_READ_PIPE, cCommand);
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

void cid_free_error (CIDError *error) {
    error->code = -1;
    if (error->message)
        g_free (error->message);
}
