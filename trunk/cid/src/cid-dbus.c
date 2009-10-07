/*
   *
   *                    simple-dbus.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
   *    Source originale: Cairo-dock
   *    Auteur origial: Adrien Pilleboue
*/
#include "cid-dbus.h"
#include "cid-messages.h"

static DBusGConnection *s_pSessionConnexion = NULL;
static DBusGConnection *s_pSystemConnexion = NULL;
static DBusGProxy *s_pDBusProxy = NULL;

DBusGConnection *
get_session_connection (void) 
{    if (s_pSessionConnexion == NULL) 
    {
        GError *erreur = NULL;
        s_pSessionConnexion = dbus_g_bus_get (DBUS_BUS_SESSION, &erreur);
        if (erreur != NULL) 
        {
            cid_warning ("Attention : %s\n", erreur->message);
            g_error_free (erreur);
            s_pSessionConnexion = NULL;
        }
    }
    return s_pSessionConnexion;
}

DBusGConnection *
get_system_connection (void) 
{    if (s_pSystemConnexion == NULL) 
    {
        GError *erreur = NULL;
        s_pSystemConnexion = dbus_g_bus_get (DBUS_BUS_SYSTEM, &erreur);
        if (erreur != NULL) 
        {
            cid_warning ("Attention : %s\n", erreur->message);
            g_error_free (erreur);
            s_pSystemConnexion = NULL;
        }
    }
    return s_pSystemConnexion;
}

DBusGProxy *
get_main_proxy (void) 
{    if (s_pDBusProxy == NULL) 
    {
        s_pDBusProxy = create_new_session_proxy (DBUS_SERVICE_DBUS,
            DBUS_PATH_DBUS,
            DBUS_INTERFACE_DBUS);
    }
    return s_pDBusProxy;
}

void 
register_service_name (const gchar *cServiceName) 
{
    DBusGProxy *pProxy = get_main_proxy ();
    if (pProxy == NULL)
        return ;
    GError *erreur = NULL;
    int request_ret;
    org_freedesktop_DBus_request_name (pProxy, cServiceName, 0, &request_ret, &erreur);
    if (erreur != NULL) 
    {
        cid_warning ("Unable to register service: %s\n", erreur->message);
        g_error_free (erreur);
    }
}

gboolean 
dbus_is_enabled (void) 
{
    return (get_session_connection () != NULL && get_system_connection () != NULL);
}


DBusGProxy *
create_new_session_proxy (const char *name, const char *path, const char *interface) 
{    DBusGConnection *pConnection = get_session_connection ();
    if (pConnection != NULL)
        return dbus_g_proxy_new_for_name (
            pConnection,
            name,
            path,
            interface);
    else
        return NULL;
}

DBusGProxy *
create_new_system_proxy (const char *name, const char *path, const char *interface) 
{    DBusGConnection *pConnection = get_system_connection ();
    if (pConnection != NULL)
        return dbus_g_proxy_new_for_name (
            pConnection,
            name,
            path,
            interface);
    else
        return NULL;
}

gboolean 
dbus_detect_application (const gchar *cName) 
{
    DBusGProxy *pProxy = get_main_proxy ();
    if (pProxy == NULL)
        return FALSE;
    
    gchar **name_list = NULL;
    gboolean bPresent = FALSE;
    
    if(dbus_g_proxy_call (pProxy, "ListNames", NULL,
        G_TYPE_INVALID,
        G_TYPE_STRV,
        &name_list,
        G_TYPE_INVALID))
    {
        cid_debug ("detection du service %s ...\n", cName);
        int i;
        for (i = 0; name_list[i] != NULL; i ++) 
        {
            if (strcmp (name_list[i], cName) == 0) 
            {
                bPresent = TRUE;
                break;
            }
        }
    }
    g_strfreev (name_list);
    return bPresent;
}

gboolean 
dbus_get_boolean (DBusGProxy *pDbusProxy, const gchar *cParameter) 
{
    GError *erreur = NULL;
    gboolean bValue = FALSE;
    dbus_g_proxy_call (pDbusProxy, cParameter, &erreur,
        G_TYPE_INVALID,
        G_TYPE_BOOLEAN, &bValue,
        G_TYPE_INVALID);
    if (erreur != NULL) 
    {
        cid_warning ("Attention : %s\n", erreur->message);
        g_error_free (erreur);
    }
    return bValue;
}

guint 
dbus_get_uinteger (DBusGProxy *pDbusProxy, const gchar *cParameter) 
{    GError *erreur = NULL;
    guint iValue = 0;
    dbus_g_proxy_call (pDbusProxy, cParameter, &erreur,
        G_TYPE_INVALID,
        G_TYPE_UINT, &iValue,
        G_TYPE_INVALID);
    if (erreur != NULL) 
    {
        cid_warning ("Attention : %s\n", erreur->message);
        g_error_free (erreur);
    }
    return iValue;
}

gchar *
dbus_get_string (DBusGProxy *pDbusProxy, const gchar *cParameter) 
{
    GError *erreur = NULL;
    gchar *cValue = NULL;
    dbus_g_proxy_call (pDbusProxy, cParameter, &erreur,
        G_TYPE_INVALID,
        G_TYPE_STRING, &cValue,
        G_TYPE_INVALID);
    if (erreur != NULL) 
    {
        cid_warning ("Attention : %s\n", erreur->message);
        g_error_free (erreur);
        return NULL;
    }
    return cValue;
}

void 
dbus_call (DBusGProxy *pDbusProxy, const gchar *cCommand) 
{
    dbus_g_proxy_call_no_reply (pDbusProxy, cCommand,
        G_TYPE_INVALID,
        G_TYPE_INVALID);
}

void 
dbus_call_boolean (DBusGProxy *pDbusProxy, const gchar *cCommand, gboolean bAction ) 
{
    dbus_g_proxy_call_no_reply (pDbusProxy, cCommand,
        G_TYPE_BOOLEAN,
        bAction,
        G_TYPE_INVALID);
}
