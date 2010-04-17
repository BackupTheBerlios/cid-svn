/*
   *
   *                    cid-struct.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
#ifndef __CID_STRUCT__
#define  __CID_STRUCT__

#include <gtk/gtk.h>

G_BEGIN_DECLS
///\______ Structures de donnees
typedef struct _CidMainContainer CidMainContainer;
typedef struct _CidLabelDescription CidLabelDescription;
typedef struct _CidControlFunctionsList CidControlFunctionsList;
typedef struct _CidDataTable CidDataTable;
typedef struct _CidDataContent CidDataContent;
typedef struct _CidDataCase CidDataCase;

typedef struct _CidModule CidModule;
typedef struct _CidModuleInterface CidModuleInterface;
typedef struct _CidModuleInstance CidModuleInstance;
typedef struct _CidVisitCard CidVisitCard;
typedef struct _CidInternalModule CidInternalModule;

///\______ Encapsulations
typedef void (* CidReadConfigFunc) (gchar *cConfFile, gpointer *data);
typedef void (* CidControlFunction) (void);
typedef void (* CidManagePlaylistFunction) (gchar *cSong);
typedef void (* CidDataAction) (CidDataCase *pCase, gpointer *pData);

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

/**
 * Structure de données représentant un tableau
 */
struct _CidDataTable {
    size_t length;
    CidDataCase *head;
    CidDataCase *tail;
};

struct _CidDataContent {
    union {
        gchar *string;
        gint iNumber;
        gboolean booleen;
    };
    GType type;
};

struct _CidDataCase {
    CidDataContent *content;
    CidDataCase *next;
    CidDataCase *prev;
};

/**
 * Liste des lecteurs supportés
 */
typedef enum {
    PLAYER_NONE,
    PLAYER_RHYTHMBOX,
    PLAYER_AMAROK_1,
    PLAYER_EXAILE,
    PLAYER_AMAROK_2,
    PLAYER_MPD 
} PlayerIndice;

/**
 * Liste des animations
 */
typedef enum {
    CID_ROTATE,
    CID_FADE_IN_OUT,
    CID_DIAPOSITIVE,
    CID_FOCUS_IN,
    CID_FOCUS_OUT
} AnimationType;

/**
 * Codes de retours spécifiques à CID
 */
typedef enum {
    CID_EXIT_SUCCESS=0,
    CID_ERROR_READING_FILE,
    CID_ERROR_READING_ARGS,
    CID_GTK_ERROR,
    CID_OPEN_GL_ERROR,
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
 * Differents etats du lecteur
 */
typedef enum {
    CID_PLAY,
    CID_PAUSE,
    CID_NEXT,
    CID_PREV
} StateSymbol;

/**
 * Couleurs disponibles pour les 'styles'
 */
typedef enum {
    CID_WHITE,
    CID_YELLOW,
    CID_RED
} SymbolColor;

/**
 * Fonctions de controle des lecteurs
 */
struct _CidControlFunctionsList {
    // fonction 'play/pause'
    CidControlFunction p_fPlayPause;
    // fonction 'next'
    CidControlFunction p_fNext;
    // fonction 'previous'
    CidControlFunction p_fPrevious;
    // fonction d'ajout a la playlist
    CidManagePlaylistFunction p_fAddToQueue;
};

/**
 * Cid main container
 */
struct _CidMainContainer {
    // fenêtre principale
    GtkWidget *pWindow;
    // panneau de configuration
    GtkWidget *pConfigPanel;
    
    ///\________ Toutes nos images
    // pochette
    cairo_surface_t *p_cSurface;
    // pochette precedente
    cairo_surface_t *p_cPreviousSurface;
    // croix pour le deplacement
    cairo_surface_t *p_cCross;
    // play
    cairo_surface_t *p_cPlay;
    // pause
    cairo_surface_t *p_cPause;
    // play big
    cairo_surface_t *p_cPlay_big;
    // pause big
    cairo_surface_t *p_cPause_big;
    // next
    cairo_surface_t *p_cNext;
    // prev
    cairo_surface_t *p_cPrev;

    // cairo context
    cairo_t *p_cContext;
    
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
    // temps avant de telecharger les pochettes manquantes
    gint iTimeToWait;
    // temps ecoule avant le telechargement
    gint iCheckIter;
    // taille des images "extras"
    gint iExtraSize;
    // taille des images prev/next
    gint iPrevNextSize;
    // taille des images play/pause big
    gint iPlayPauseSize;
    // position en x du curseur
    gint iCursorX;
    // position en y du curseur
    gint iCursorY;
    // vitesse de l'animation
    gint iAnimationSpeed;
    
    // avancement de l'animation
    gdouble dAnimationProgress;
    
    // pipe
    guint iPipe;
    
    // lecteur a monitorer
    PlayerIndice iPlayer;
    // type d'animation
    AnimationType iAnimationType;
    // taille des covers à télécharger
    ImageSizes iImageSize;
    // fonctions de monitoring
    CidControlFunctionsList *pMonitorList;
    // couleur des symboles
    SymbolColor iSymbolColor;
    
    // type de fenêtre
    GdkWindowTypeHint iHint;
    
    // couleur de la fenêtre
    gdouble *dColor;
    // couleur au survol
    gdouble *dFlyingColor;
    // couleur de police
    gdouble *dPoliceColor;
    // couleur de contour de police
    gdouble *dOutlineTextColor;
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
    // variation focus
    gdouble dFocusVariation;
    // taille de la police
    gdouble dPoliceSize;
    // alpha pour le fade in
    gdouble dFadeInOutAlpha;
    
    // image par défaut
    gchar *cDefaultImage;
    // fichier de conf
    gchar *cConfFile;
    // verbosité du programme
    gchar *cVerbosity;
    
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
    // animation en cours ?
    gboolean bAnimation;
    // animation de focus en cours ?
    gboolean bFocusAnimation;
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
    // options instables ?
    gboolean bUnstable;
    // survol de cid ?
    gboolean bCurrentlyFlying;
    // afficher le statut
    gboolean bPlayerState;
    // on affiche un masque au survol ?
    gboolean bMask;
    // afficher le titre ?
    gboolean bDisplayTitle;
    // CID en mode de secours ?
    gboolean bSafeMode;
    // La fenetre de conf est ouverte ?
    gboolean bBlockedWidowActive;
    // On a active/desactive les options instables ?
    gboolean bChangedTestingConf;
    // Est-ce qu'un panneau de configuration est deja present ?
    gboolean bConfFilePanel;
    // On a lance un pipe ?
    gboolean bPipeRunning;
    // Afficher les controles ?
    gboolean bDisplayControl;
    // Passe cid au premier plan lors d'un survol ?
    gboolean bShowAbove;
    // On ne fait que configurer cid ?
    gboolean bConfigPanel;

    
    // taille de la couleur
    gsize gColorSize;
    // taille de la couleur de survol
    gsize gFlyingColorSize;
    // taille de la couleur de police
    gsize gPlainTextSize;
    // taille de la couleur de contour de police
    gsize gOutlineTextSize;
    
    // keyFile
    GKeyFile *pKeyFile;
    
    ///\__ Dimensions de l'ecran
    int XScreenWidth;
    int XScreenHeight;
    
    /// MPD specific config
    gchar *mpd_dir;
    gchar *mpd_host;
    gchar *mpd_pass;
    gint mpd_port;
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
