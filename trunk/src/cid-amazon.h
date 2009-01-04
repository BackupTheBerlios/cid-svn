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
#ifndef __CID_STRUCT__
#define  __CID_STRUCT__

#include <stdio.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

#define LICENCE_KEY 0C3430YZ2MVJKQ4JEKG2
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"
#define DEFAULT_XML_LOCATION "/tmp/cid_amazon.xml"

char **imageSize = ["MediumImage","LargeImage"];

static void processNode (xmlTextReaderPtr reader, const char *imageSize);

static void streamFile(const char *filename, const char *imageSize);

static boolean getXMLFile (const gchar *artist, const gchar *album);

void downloadCover (void);

G_END_DECLS
#endif
