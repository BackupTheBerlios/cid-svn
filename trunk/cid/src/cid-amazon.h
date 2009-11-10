/*
   *
   *                     cid-amazon.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
#ifndef __CID_AMAZON__
#define  __CID_AMAZON__

#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <gtk/gtk.h>
#include <curl/curl.h>

G_BEGIN_DECLS

#define LICENCE_KEY "0C3430YZ2MVJKQ4JEKG2"
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"
#define DEFAULT_XML_LOCATION "/tmp/cid_amazon.xml"
#define DEFAULT_DOWNLOADED_IMAGE_LOCATION "/tmp/default.jpg"

#define LAST_ID_KEY "941340dc17f3e2704c5b06ebf845e98e"
#define LAST_API_URL "http://ws.audioscrobbler.com/2.0/?method="
#define LAST_ARTIST "artist.getinfo"
#define LAST_ALBUM "album.getinfo"
//&api_key=
#define TEST_XML "../data/test.xml"

/**
 * Lit les noeuds du fichier en cours de parsage
 * @param reader le noeud courrant
 * @param imageSize Noeud que l'on cherche
 */
static void cid_process_node (xmlTextReaderPtr reader, gchar **cValue);

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
void cid_stream_file(const char *filename, gchar **cValue);

/**
 * Recupere le fichier xml sur amazon.
 * @param artist Nom de l'artiste.
 * @param album Nom de l'album.
 * @return succes du telechargement.
 */
gboolean cid_get_xml_file (const gchar *artist, const gchar *album);

/**
 * Recupere la pochette.
 * @param cURL URL de la pochette.
 * @param cDestPath Ou en enregistre la pochette telechargee.
 * @return succes du telechargement.
 */
gboolean cid_download_missing_cover (const gchar *cURL, const gchar *cDestPath);


void cid_test_xml ();

G_END_DECLS
#endif
