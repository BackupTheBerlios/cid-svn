/*
   *
   *                   cid-messages.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
   * Inspired by Cairo-dock
   * Original author: Cédric GESTES
*/

#ifndef __CID_MESSAGES__
#define  __CID_MESSAGES__

# include <gtk/gtk.h>
G_BEGIN_DECLS

# ifndef _INSIDE_CID_MESSAGES_C_
  extern GLogLevelFlags gLogLevel;
# endif

/**
 * internal function
 * @param log level
 * @param filename where the messge comes from
 * @param function where the message comes from
 * @param line where the message comes from
 * @param message to display (viariable nomber of paramters)
 */
void cid_log_location(const GLogLevelFlags loglevel,
                     const char *file,
                     const char *func,
                     const int line,
                     const char *format,
                     ...);

/**
 * initialise the log system
 * @param do we use the log system
 */
void cid_log_init(gboolean bBlackTerminal);

/**
 * set the verbosity level
 * @param verbosity level
 */
void cid_log_set_level(GLogLevelFlags loglevel);

#define cid_error(...)                                                  \
  cid_log_location(G_LOG_LEVEL_ERROR, __FILE__, __PRETTY_FUNCTION__, __LINE__,__VA_ARGS__)

#define cid_warning(...)                                                \
  cid_log_location(G_LOG_LEVEL_WARNING, __FILE__, __PRETTY_FUNCTION__, __LINE__,__VA_ARGS__)

#define cid_message(...)                                                \
  cid_log_location(G_LOG_LEVEL_MESSAGE, __FILE__, __PRETTY_FUNCTION__, __LINE__,__VA_ARGS__)

#define cid_debug(...)                                                  \
  cid_log_location(G_LOG_LEVEL_DEBUG, __FILE__, __PRETTY_FUNCTION__, __LINE__,__VA_ARGS__)

#define cid_info(...)                                                  \
  cid_log_location(G_LOG_LEVEL_INFO, __FILE__, __PRETTY_FUNCTION__, __LINE__,__VA_ARGS__)

/**
 * sort du programme avec le code de retour donné
 * en affichant le message d'erreur passé en paramètre
 * @param code de retour
 * @param message d'erreur à nombre variable de paramètres
 */
void cid_exit (int code, const gchar *mess, ...);

G_END_DECLS
#endif 
