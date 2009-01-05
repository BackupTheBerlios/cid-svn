/*
   *
   *                     cid-amazon.h
   *                       -------
   *                 Conky Images Display
   *              06/10/2008 - SANS Benjamin
   *             ----------------------------
   *
   *
*/
#ifndef __CID_AMAZON__
#define  __CID_AMAZON__

#include <stdio.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

#define LICENCE_KEY "0C3430YZ2MVJKQ4JEKG2"
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"
#define DEFAULT_XML_LOCATION "/tmp/cid_amazon.xml"
#define DEFAULT_DOWNLOADED_IMAGE_LOCATION "/tmp/default.jpg"

static void cid_process_node (xmlTextReaderPtr reader, gchar **cValue);

void cid_stream_file(const char *filename, gchar **cValue);

gboolean cid_get_xml_file (const gchar *artist, const gchar *album);

gboolean cid_download_missing_cover (const gchar *cURL, const gchar *cDestPath);

G_END_DECLS
#endif
