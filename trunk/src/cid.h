/*
   *
   *                              cid.h
   *                             -------
   *                       Conky Images Display
   *             05/10/2008 - Charlie MERLAND / Benjamin SANS
   *             --------------------------------------------
   *
*/

#ifndef __CID__
#define  __CID__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <libintl.h>
#include <locale.h>

#include <pthread.h>

#include "cid-amarok.h"
#include "cid-exaile.h"
#include "cid-rhythmbox.h"
#include "cid-dbus.h"
#include "cid-main.h"
#include "cid-utilities.h"
#include "cid-messages.h"
#include "cid-config.h"
#include "cid-struct.h"
#include "cid-amazon.h"
#include "cid-callbacks.h"
#include "cid-menu-factory.h"
#include "cid-conf-panel-factory.h"
#include "cid-animation.h"
#include "cid-amarok2.h"

G_BEGIN_DECLS

#define _(string) gettext (string)

CidMainContainer *cid;

/* Alias pour récupérer l'URI de l'image par défaut */
#define DEFAULT_IMAGE cid->pDefaultImage

/**
 * Intercpte un signal afin de le traiter
 * @param signal signal intercepté
 */
void cid_intercept_signal (int signal);

/**
 * Sert à définir les signaux à intercepter
 */
void cid_set_signal_interception (void);

G_END_DECLS
#endif
