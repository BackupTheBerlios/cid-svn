/*
   *
   *                           cid-menu-factory.c
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/

//#include "cid.h"
#include "cid-menu-factory.h"
#include "cid-callbacks.h"
#include "cid-utilities.h"

extern CidMainContainer *cid;

void
cid_build_menu (void) 
{
    static gpointer *data = NULL;
    
    GtkWidget *menu = gtk_menu_new ();
    
    GtkWidget *pMenuItem, *image, *sep1, *sep2;
    pMenuItem = gtk_image_menu_item_new_with_label ("C.I.D.");
    gchar *cIconPath = g_strdup (CID_DEFAULT_IMAGE);
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, 32, 32, NULL);
    image = gtk_image_new_from_pixbuf (pixbuf);
    g_free (cIconPath);
    g_object_unref (pixbuf);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
    gtk_menu_shell_append  (GTK_MENU_SHELL (menu), pMenuItem);

    GtkWidget *pSubMenu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
    
    _add_entry_in_menu (_("Configure"), GTK_STOCK_PREFERENCES, _cid_conf_panel, pSubMenu);
    
    sep1 = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (pSubMenu), sep1);
    
    _add_entry_in_menu (_("About"), GTK_STOCK_ABOUT, _cid_about, pSubMenu);
    _add_entry_in_menu (_("Quit"), GTK_STOCK_QUIT, _cid_quit, pSubMenu);
    
    if (cid->bMonitorPlayer && cid->iPlayer!=PLAYER_NONE) 
    {
        sep2 = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), sep2);
    
        _add_entry_in_menu (_("Play/Pause"), NULL, cid->pMonitorList->p_fPlayPause, menu);
        _add_entry_in_menu (_("Next"), NULL, cid->pMonitorList->p_fNext, menu);
        _add_entry_in_menu (_("Previous"), NULL, cid->pMonitorList->p_fPrevious, menu);
    }
    
    gtk_widget_show_all (menu);

    gtk_menu_popup (GTK_MENU (menu),
                NULL,
                NULL,
                NULL,
                NULL,
                1,
                gtk_get_current_event_time ());
    
    //return menu;
}
