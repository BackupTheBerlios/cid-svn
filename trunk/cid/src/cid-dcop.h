#ifndef __CID_DCOP__
#define  __CID_DCOP__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _CIDError CIDError;

static gchar *CID_DCOP_MESSAGES[] = {"Cannot open pipe for '%s'",
                                     "Cannot read pipe for '%s'",
                                     "Unknow error for '%s'"};

typedef enum {
    CID_DCOP_UNREACHABLE=1,
    CID_DCOP_CANT_READ_PIPE,
    CID_DCOP_CRITICAL
} cidErrorCode;

struct _CIDError {
    cidErrorCode code;
    gchar *message;
};

/**
 * Lecture d'une chaine de caracteres sur un pipe
 */
gchar *cid_dcop_get_string_with_error_full (const gchar *cCommand, gchar *cDefault, CIDError **error);
#define cid_dcop_get_string(cCommand) cid_dcop_get_string_with_error_full(cCommand,NULL,NULL)
#define cid_dcop_get_string_full(cCommand,cDefault) cid_dcop_get_string_with_error_full(cCommand,cDefault,NULL)

/**
 * Lecture d'un entier sur un pipe
 */
gint cid_dcop_get_int_with_error_full (const gchar *cCommand, gint iDefault, CIDError **error);
#define cid_dcop_get_int(cCommand) cid_dcop_get_int_with_error_full(cCommand,-1,NULL)
#define cid_dcop_get_int_full(cCommand,iDefault) cid_dcop_get_int_with_error_full(cCommand,iDefault,NULL)

/**
 * Lecture d'un booleen sur un pipe
 */
gboolean cid_dcop_get_boolean_with_error_full (const gchar *cCommand, gboolean bDefault, CIDError **error);
#define cid_dcop_get_boolean(cCommand) cid_dcop_get_boolean_with_error_full(cCommand,FALSE,NULL)
#define cid_dcop_get_boolean_full(cCommand,bDefault) cid_dcop_get_boolean_with_error_full(cCommand,bDefault,NULL)

/**
 * Permet de liberer une erreur
 */
void cid_free_error (CIDError *error);

G_END_DECLS

#endif
