/*
   *
   *                     cid-amazon.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
//#include "cid.h"
#include "cid-cover.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"

#include <curl/curl.h>

extern CidMainContainer *cid;

gboolean bCurrentlyDownloading = FALSE;
gboolean bCurrentlyDownloadingXML = FALSE;

#ifdef LIBXML_READER_ENABLED

int ret;
int flag, found;

gchar *URL;
gchar *TAB_IMAGE_SIZES[] = {"medium","large"};

/**
 * Lit les noeuds du fichier en cours de parsage
 * @param reader le noeud courrant
 * @param imageSize Noeud que l'on cherche
 */
static void 
cid_process_node (xmlTextReaderPtr reader, gchar **cValue) 
{
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
        name = BAD_CAST "--";

        value = xmlTextReaderConstValue(reader);
        if ((strcmp(name,TAB_IMAGE_SIZES[cid->iImageSize])==0 || flag) && !found) 
        {
            //printf("node: %s ", name);
        if (value == NULL)
            cid_message ("");
        else {
            cid_message ("%s\n", value);
            found=TRUE;
            *cValue=g_strdup(value);
        }
        if (strcmp(name,"#text")!=0) 
        {
            flag=TRUE;
        } 
        else 
        {
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
void 
cid_stream_file(const char *filename, gchar **cValue) 
{
    /*
    * this initialize the library and check potential ABI mismatches
    * between the version it was compiled for and the actual shared
    * library used.
    */
    LIBXML_TEST_VERSION
    
    xmlTextReaderPtr reader;
    flag = FALSE;
    found=FALSE;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) 
    {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) 
        {
            cid_process_node (reader,cValue);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) 
        {
            cid_warning ("%s : failed to parse\n", filename);
        }
    } 
    else 
    {
        cid_warning ("Unable to open %s\n", filename);
    }
    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
}

void
cid_test_xml ()
{
    xmlDocPtr doc;
    doc = xmlParseFile(TEST_XML);
    if (doc == NULL) {
        fprintf(stderr, "Document XML invalide\n");
        return;
    }
    // Initialisation de l'environnement XPath
    xmlXPathInit();
    // Création du contexte
    xmlXPathContextPtr ctxt = xmlXPathNewContext(doc); // doc est un xmlDocPtr représentant notre catalogue
    if (ctxt == NULL) {
        fprintf(stderr, "Erreur lors de la création du contexte XPath\n");
        return;
    }
    // Evaluation de l'expression XPath
    char *cXPath = NULL;
    asprintf (&cXPath, "//image[@size='%s']/text()",TAB_IMAGE_SIZES[cid->iImageSize]);
    xmlXPathObjectPtr xpathRes = xmlXPathEvalExpression(cXPath, ctxt);
    //xmlXPathObjectPtr xpathRes = xmlXPathEvalExpression("count(//image[size=large])", ctxt);
    free (cXPath);
    if (xpathRes == NULL) {
        fprintf(stderr, "Erreur sur l'expression XPath\n");
        return;
    }
    
    // Manipulation du résultat
    if (xpathRes->type == XPATH_NODESET) {
        int i;
        printf("Produits dont le prix est inférieur à 10 Euros :\n");
        for (i = 0; i < xpathRes->nodesetval->nodeNr; i++) {
            xmlNodePtr n = xpathRes->nodesetval->nodeTab[i];
            if (n->type == XML_TEXT_NODE || n->type == XML_CDATA_SECTION_NODE) {
                printf("- %s\n", n->content);
            }
        }
    }
    
    /*
    if (xpathRes->type == XPATH_NUMBER) {
        printf("Nombre de produits dans le catalogue : %.0f\n", xmlXPathCastToNumber(xpathRes));
    }
    */
    // Libération de la mémoire
    xmlXPathFreeObject(xpathRes);
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(doc);
}

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    if (g_strcasecmp("Unknown",artist)==0 || g_strcasecmp("Unknown",album)==0 ||
        g_strcasecmp("Inconnu",artist)==0 || g_strcasecmp("Inconnu",album)==0)
        return FALSE;
        
    gchar *ar = g_strdup(artist);
    gchar *al = g_strdup(album);
    
    cid_str_replace_all_seq(&ar," éèàç","+eeac");
    
    gchar *cFileToDownload = g_strdup_printf("%s%s%s&Artist=%s&Album=%s",AMAZON_API_URL_1,LICENCE_KEY,AMAZON_API_URL_2,artist,album);
    gchar *cTmpFilePath = g_strdup (DEFAULT_XML_LOCATION);
    
    /*
    gchar *cCommand = g_strdup_printf ("rm %s >/dev/null 2>&1", DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    if (!system (cCommand)) return FALSE;
    g_free (cCommand);
    */
    cid_remove_file (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    //cCommand = g_strdup_printf ("wget \"%s\" -O '%s-bis' -t 2 -T 2 > /dev/null 2>&1 && mv %s-bis %s", cFileToDownload, cTmpFilePath, cTmpFilePath, cTmpFilePath);
    //cid_debug ("%s\n",cCommand);
    //system (cCommand);
    //cid_launch_command (cCommand);
    CURL *handle = curl_easy_init(); 
    curl_easy_setopt(handle, CURLOPT_URL, "http://annonces.ebay.fr/");
    FILE * fp = fopen("test.html", "w"); 
    curl_easy_setopt(handle,  CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(handle,  CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_perform(handle);
    fclose(fp);
    curl_easy_cleanup(handle);

    bCurrentlyDownloadingXML = TRUE;
    //g_free (cCommand);
    g_free (cTmpFilePath);
    g_free (cFileToDownload);
    return TRUE;
}

gboolean 
cid_download_missing_cover (const gchar *cURL, const gchar *cDestPath) 
{
    gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s-bis' -t 2 -T 2 > /dev/null 2>&1 && mv %s-bis %s", cURL, cDestPath, cDestPath, cDestPath);
    cid_debug ("%s\n",cCommand);
    //system (cCommand);
    cid_launch_command (cCommand);
    bCurrentlyDownloading = TRUE;
    g_free (cCommand);
    /*
    cCommand = g_strdup_printf ("rm %s >/dev/null 2>&1", DEFAULT_XML_LOCATION);
    if (!system (cCommand)) return FALSE;
    g_free (cCommand);
    */
    cid_remove_file (DEFAULT_XML_LOCATION);
    return TRUE;
}

#else

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    return FALSE;
}

#endif

