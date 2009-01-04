/*
   *
   *                         cid-conf-panel-factory.h
   *                                -------
   *                          Conky Images Display
   *                              Benjamin SANS
   *                    --------------------------------
   *
*/
#ifndef __CID_CONF_PANEL_FACTORY__
#define __CID_CONF_PANEL_FACTORY__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

G_BEGIN_DECLS

typedef enum {
	CID_MODEL_NAME = 0,
	CID_MODEL_RESULT,
	CID_MODEL_DESCRIPTION_FILE,
	CID_MODEL_ACTIVE,
	CID_MODEL_ORDER,
	CID_MODEL_IMAGE,
	CID_MODEL_ICON,
	CID_MODEL_NB_COLUMNS
} _CidModelColumns;

void cid_config_panel_destroyed (void);

gboolean cid_edit_conf_file_with_panel (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain);

gboolean cid_edit_conf_file_core (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain);

GtkWidget *cid_generate_ihm_from_keyfile (GKeyFile *pKeyFile, const gchar *cTitle, GtkWindow *pParentWindow, GSList **pWidgetList, gboolean bApplyButtonPresent, gchar iIdentifier, gchar *cPresentedGroup, gchar *cGettextDomain, GPtrArray *pDataGarbage);

G_END_DECLS
#endif
