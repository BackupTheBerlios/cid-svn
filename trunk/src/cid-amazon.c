/*
   *
   *                     cid-amazon.c
   *                       -------
   *                 Conky Images Display
   *              14/10/2008 - SANS Benjamin
   *             ----------------------------
   *
   *
*/
#include "cid.h"

#ifdef LIBXML_READER_ENABLED

int ret;
int flag, found;

gchar *URL;

/**
 * Lit les noeuds du fichier en cours de parsage
 * @param reader le noeud courrant
 * @param imageSize Noeud que l'on cherche
 */
static void processNode (xmlTextReaderPtr reader, const char *imageSize) {
	const xmlChar *name, *value;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

		value = xmlTextReaderConstValue(reader);
		if ((strcmp(name,imageSize[cid->cImageSize])==0 || flag) && !founded) {
			//printf("node: %s ", name);
		if (value == NULL)
			cid_message ("");
		else {
			cid_message ("%s\n", value);
			found=TRUE;
			URL=g_strdup(value);
		}
		if (strcmp(name,"#text")!=0) {
			flag=TRUE;
		} else {
			flag=FALSE;
		}
	}
}

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
static void streamFile(const char *filename, const char *imageSize) {
	xmlTextReaderPtr reader;
	flag = FALSE;
	found=FALSE;

	reader = xmlReaderForFile(filename, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			processNode(reader,imageSize);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			cid_warning ("%s : failed to parse\n", filename);
		}
	} else {
		cid_warning ("Unable to open %s\n", filename);
	}
}

static boolean getXMLFile (const gchar *artist, const gchar *album) {
	gchar *cFileToDownload = g_strdup_printf("%s%s%s&Artist=%s&Album=%s",AMAZON_API_URL_1,LICENCE_KEY,AMAZON_API_URL_2,artist,album);
	gchar *cTmpFilePath = g_strdup (DEFAULT_XML_LOCATION);
	int fds = mkstemp (cTmpFilePath);
	if (fds == -1)
	{
		g_free (cTmpFilePath);
		return FALSE;
	}
	
	gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s' -t 2 -T 2", cFileToDownload, cTmpFilePath);
	system (cCommand);
	g_free (cCommand);
	close(fds);
	g_free(cTmpFilePath);
	return TRUE;
}

/*
int main(int argc, char **argv) {
	if (argc != 2)
		return(1);

	/*
	* this initialize the library and check potential ABI mismatches
	* between the version it was compiled for and the actual shared
	* library used.
	*/
//	LIBXML_TEST_VERSION

//	streamFile(argv[1]);

	/*
	 * Cleanup function for the XML library.
	 */
//	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
//	xmlMemoryDump();
//	return(0);
//}

#endif

