/*
   *
   *                    cid-struct.h
   *                       -------
   *                 Conky Images Display
   *              06/10/2008 - SANS Benjamin
   *             ----------------------------
   *
   *
*/
#ifndef __CID_STRUCT__
#define  __CID_STRUCT__

G_BEGIN_DECLS

typedef struct _CidMainContainer CidMainContainer;

typedef struct _CidLabelDescription CidLabelDescription;

typedef void (* CidReadConfigFunc) (gchar *cConfFile, gpointer data);

#define rhythmboxData musicData

/**
 * Structure de données utilisée pour stocker les informations
 * fournies par Rhythmbox et Amarok
 */
struct data {
	gchar *playing_uri;
	gchar *playing_artist;
	gchar *playing_album;
	gchar *playing_title;
	gchar *playing_cover;

	guint playing_track;
	guint playing_duration;
	guint iSidCheckCover;

	gboolean cover_exist;
	gboolean playing;
	gboolean opening;
} musicData;

/*
typedef enum {
	CID_DEBUG_MODE=1,
	CID_HELP_MENU,
	CID_TESTING_MODE,
	CID_CHANGE_CONFIG_FILE,
	CID_GIVE_VERSION,
	CID_SET_VERBOSITY
} cidParamList;
*/

/**
 * Liste des lecteurs supportés
 */
typedef enum {
	PLAYER_RHYTHMBOX,
	PLAYER_AMAROK_1
} PlayerIndice;

/**
 * Liste des animations
 */
typedef enum {
	CID_ROTATE,
	CID_FOCUS_IN,
	CID_FOCUS_OUT
} AnimationType;

/**
 * Codes de retours spécifiques à CID
 */
typedef enum {
	CID_EXIT_SUCCESS=EXIT_SUCCESS,
	CID_ERROR_READING_FILE,
	CID_ERROR_READING_ARGS,
	CID_GTK_ERROR,
	CID_PLAYER_ERROR,
	CID_EXIT_ERROR
} codesRetours;

/**
 * Tailles possible pour le téléchargement des pochettes
 */
typedef enum {
	MEDIUM_IMAGE,
	LARGE_IMAGE
} ImageSizes;

/**
 * Options à récupérer dans cid.conf
 */
typedef struct {
	gboolean hide;
	gboolean corners;
	gboolean lock;
	gboolean allDesktop;
	gboolean animation;
	gboolean monitoring;
	gboolean threaded;
	gchar *image;
	//gchar *player_name;
	PlayerIndice iPlayer;
	AnimationType iAnimationType;
	gdouble *couleur;
	gdouble *couleurSurvol;
	gdouble rotation;
	gint pos_x;
	gint pos_y;
	gint size_x;
	gint size_y;
	gint inter;
} FileSettings;

/**
 * Cid main container
 */
struct _CidMainContainer {
	// fenêtre principale
	GtkWidget *cWindow;
	
	// buffer de l'image à dessiner
	GdkPixmap *pPixmap;
	
	// pochette
	cairo_surface_t *cSurface;
	
	// cairo context
	cairo_t *pContext;
	
	// largeur de cid
	gint iWidth;
	// hauteur de cid
	gint iHeight;
	// position en x
	gint iPosX;
	// position en y
	gint iPosY;
	// interval par défaut
	gint iInter;
	// currently drawing
	gint iCurrentlyDrawing;
	
	// numéro du player
	PlayerIndice iPlayer;
	AnimationType iAnimationType;

	// type de fenêtre
	GdkWindowTypeHint cidHint;
	
	// couleur de la fenêtre
	gdouble *dColor;
	// couleur au survol
	gdouble *dFlyingColor;
	// angle de cid
	gdouble dRotate;
	// opacité de la fenêtre
	gdouble dAlpha;
	// rouge
	gdouble dRed;
	// vert
	gdouble dGreen;
	// bleu
	gdouble dBlue;
	// current animation angle
	gdouble dAngle;
	
	// image par défaut
	gchar *pDefaultImage;
	// fichier de conf
	gchar *pConfFile;
	// lecteur à monitorer
	//gchar *pPlayer;
	// verbosité du programme
	gchar *pVerbosity;
	
	// caché ?
	gboolean bHide;
	// ne pas couper les angles ?
	gboolean bKeepCorners;
	// en mode testing ?
	gboolean bTesting;
	// déplacer en cliquant ?
	gboolean bLockPosition;
	// afficher sur tous les bureaux ?
	gboolean bAllDesktop;
	// télécharger les pochettes manquantes ?
	gboolean bDownload;
	// CID lancé ?
	gboolean bRunning;
	// animation en cour ?
	gboolean bAnimation;
	// on autorise l'animation ?
	gboolean bRunAnimation;
	// on autorise le monitoring du player ?
	gboolean bMonitorPlayer;
	// lecteur en lecture ?
	gboolean bCurrentlyPlaying;
	// animation threadee ?
	gboolean bThreaded;
	// mode developpeur ?
	gboolean bDevMode;
	
	// taille de la couleur
	gsize gColorSize;
	// taille de la couleur de survol
	gsize gFlyingColorSize;
	
	// keyFile
	GKeyFile *pKeyFile;
	
	// taille des covers à télécharger
	ImageSizes cImageSize;
};

struct _CidLabelDescription {
	// Taille de la police (et hauteur du texte en pixels).
	gint iSize;
	// Police de caracteres.
	gchar *cFont;
	// Epaisseur des traits.
	PangoWeight iWeight;
	// Style du trace (italique ou droit).
	PangoStyle iStyle;
	// Couleur de debut du dégradé.
	gdouble fColorStart[3];
	// Couleur de fin du dégradé.
	gdouble fColorStop[3];
	// TRUE ssi le dégradé est du haut vers le bas.
	gboolean bVerticalPattern;
	// Couleur du fond.
	gdouble fBackgroundColor[4];
};

G_END_DECLS
#endif
