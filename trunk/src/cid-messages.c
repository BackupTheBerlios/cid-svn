/*
   *
   *                   cid-messages.c
   *                      -------
   *                Conky Images Display
   *             23/08/2008 - Benjamin SANS
   *             --------------------------
   *
   * Inspired by Cairo-dock
   * Original author: Cédric GESTES
*/

#define _INSIDE_CID_MESSAGES_C_

#include "cid.h"

char s_iLogColor = '0';
static GLogLevelFlags gLogLevel = 0;

/* #    'default'     => "\033[1m", */

/* #    'black'     => "\033[30m", */
/* #    'red'       => "\033[31m", */
/* #    'green'     => "\033[32m", */
/* #    'yellow'    => "\033[33m", */
/* #    'blue'      => "\033[34m", */
/* #    'magenta'   => "\033[35m", */
/* #    'cyan'      => "\033[36m", */
/* #    'white'     => "\033[37m", */


const char*_cid_log_level_to_string(const GLogLevelFlags loglevel)
{
  switch(loglevel)
  {
  case G_LOG_LEVEL_ERROR:
    return "\033[1;31mERROR   : \033[0m ";
  case G_LOG_LEVEL_WARNING:
    return "\033[1;38mwarning : \033[0m ";
  case G_LOG_LEVEL_MESSAGE:
    return "\033[1;32mmessage : \033[0m ";
  case G_LOG_LEVEL_INFO:
    return "\033[1;33minfo    : \033[0m ";
  case G_LOG_LEVEL_DEBUG:
    return "\033[1;35mdebug   : \033[0m ";
  }
  return "";
}

void cid_log_location(const GLogLevelFlags loglevel,
                     const char *file,
                     const char *func,
                     const int line,
                     const char *format,
                     ...)
{
  va_list args;

  if (loglevel > gLogLevel)
    return;
  g_print(_cid_log_level_to_string(loglevel));
  g_print("\033[0;37m(%s:%s:%d) \033[%cm \n  ", file, func, line, s_iLogColor);
  va_start(args, format);
  g_logv(G_LOG_DOMAIN, loglevel, format, args);
  va_end(args);
}

static void cid_log_handler(const gchar *log_domain,
                                   GLogLevelFlags log_level,
                                   const gchar *message,
                                   gpointer user_data)
{
  if (log_level > gLogLevel)
    return;
  g_print("%s\n", message);
}

void cid_log_init(gboolean bBlackTerminal)
{
  g_log_set_default_handler(cid_log_handler, NULL);
  s_iLogColor = (bBlackTerminal ? '1' : '0');
}

void cid_log_set_level(GLogLevelFlags loglevel)
{
  gLogLevel = loglevel;
}

void cid_exit (int code, const gchar *mess, ...) {
	va_list args;
	va_start (args, mess);
	gchar *cFullText = g_strdup_vprintf (mess, args);
	cid_error (cFullText);
	g_free (cFullText);
	va_end (args);
	cid_sortie (code);
}
