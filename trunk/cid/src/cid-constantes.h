/*
   *
   *                 cid-constantes.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_CONSTANTES__
#define  __CID_CONSTANTES__

G_BEGIN_DECLS

#define SECONDES * 1000
#define MINUTES * 60
#define HEURES * 60

#define CONFIG_WIDTH 800
#define CONFIG_HEIGHT 600

#define OLD_CONFIG_FILE ".cidrc"

/* Alias pour récupérer l'URI de l'image par défaut */
#define DEFAULT_IMAGE cid->cDefaultImage

#define TESTING_COVER "../data/default.svg"
#define TESTING_FILE "../data/cid.conf"

#define PLAYLIST_NAME "C.I.D."

#define FADE_VARIATION .15
#define IN_OUT_VARIATION .05

#define DEFAULT_SIZE 150
#define MAX_SIZE 1024
#define DEFAULT_TIMERS 5

#define COIN_COIN \
"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\
\nMMMMMMMMMMMMMMMMMMMMMMMWXOdlcccldONMMMMMMMMMMMMM\
MMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMO'    \
     ..;xWMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMM\
MMMMMMMMMMWl          .;;. ;XMMMMMMMMMMMMMMMMM\
MMMM\nMMMMMMMMMMMMMMMMMMMMd                 .\
KMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMM\
:  ...      ...    :MMMMMMMMMMMMMMMMMMMM\nMMM\
MMMMMMMMMMMMMMMMM: 'od:   'dkxd,    .NMMMMMMMMMMMM\
MMMMMMM\nMMMMMMMMMMMMMMMMMMMMc l.,O,  O0''lK'    \
KMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMo c..ddllkx  ;K\
,    0MMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMx '\
xxkO00OOkx0d.    xMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMM\
MMMMMMO.oxkO0000Okkkk.    :WMMMMMMMMMMMMMMMMMM\nMMMMM\
MMMMMMMMMMMMMMMK ,xxxkkkxxkO0O' .;' cNMMMMMMMMMMMMMMMM\
M\nMMMMMMMMMMMMMMMMMMMWc cK0OOOOO0KNWWX,     'XMMMMMMMMM\
MMMMMMM\nMMMMMMMMMMMMMMMMMMK, ;KWWXKXXNWMMMMMX'     .OMM\
MMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMWo  dMMMMMWWMMMMMMMMM0.      \
cNMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMX:  ,NMMMWWNWMMMMMWWWWW\
d       ,KMMMMMMMMMMMM\nMMMMMMMMMMMMMMO.  .xXWWMMWNMMMMMMWNNNXN\
o  .     dMMMMMMMMMMM\nMMMMMMMMMMMMMX.  .kWMMMMMMMMMMMMMMMMMWNN\
o   .    kMMMMMMMMMM\nMMMMMMMMMMMMWc   OWMMMMMMMMMMMMMMMMMMMMMW\
c . .   .KMMMMMMMMM\nMMMMMMMMMMMWo   dMMMMMMMWWMMMMMMMMMMMMMMMX\
.   .   'NMMMMMMMM\nMMMMMMMMMMMd . ,NMMMMMMMNWMMMMMMMMMMMMMMMW\
'        dMMMMMMMM\nMMMMMMMMMN:  . dMMMMMMMMNWMMMMMMMMMMMMMMMM\
,        :MMMMMMMM\nMMMMMMMMMO   ..kMMMMMMMMNWMMMMMMMMMMMMMMMM\
,  .     :MMMMMMMM\nMMMMMMMMMKloxl;cKMMMMMMMNWMMMMMMMMMMMMMNXK'     ...xMMMMMMMM\nMMMMMMMWKddkkkkl':kNMMMMWWMMMMMMMMMMMMM0Ok;      ;\
kOWMMMMMMM\nMMMNXKOdodxkkkkkd. .dXMMMMMMMMMMMMMMMN0kkkl.....\
ckOONMMMMMMM\nMMNkkkkxxkkkkkkkkd'  .:XMMMMMMMMMMMMMXOxxkxoloox\
kOOkkKWMMMMM\nMMNxxkkkOOOkkkkkkxxc\
  .0MMMMMMMMMMMMMO;lxkkkkkkkOOOOOkO0XMMM\
\nMMXodkkOOOOOOOOOkkkxoxXMMMMMMMMMMMXk\
: .oxkOOOOOkkkkOOOOOOWMM\nMMKodxkkkOOOOOOOOOOkxdloOKXKK0kdl\
,    ,dkkOkkkkkkkxxkO0XWMMM\nMMXxdoooddddxxkkkkkkxoc\
.              ;dkkkkkkxxxk0XNWMMMMMM\nMMMMWNK0Okdollloodddolc;codx\
xxxxxxxddlcldxxxxxdkKWMMMMMMMMMM\nMMMMMMMMMMMWNX0xoccclxXMMMMMMMMMM\
MMMMMXdccllokKWMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMWNNWMMMMMMMMMMMMMMM\
MMMMWNNWWMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\
MMMMMMMMMMMMMMMMMMMMM\n"

#define COIN \
"Congratulation ! You just found my Easter Egg :)  \n\
                        `                         \n\
                   :sdNMMMNhs:                    \n                 `hMMMMMMMMhsmm/                  \n                 yMMMMMMMMNmNMMM/                 \n                 NMmdmNMMNddmNMMN`                \n\
                 Nho+oMMy:+/+mMMM-                \n                 dsmd-dh+/Nd-yMMM/                \n                 hm+o:::::o+:mMMMo                \n                 sy+/::-::///dMNNd                \n\
                 +d/+//////:-yMdhNs`              \n                -mh.-::::-.` `hMNMMh.             \n              `oNy.  `.``     `dMMMMd:            \n             -hMh`   ``        .NMMMMNo`          \n\
            /NMN:`   ``     ``.`+NNNMMMd.         \n           -NMN/`              ``/NNmNMMd.        \n          `dNN/                   oNmNNMMh        \n          ymMo      `             `MMMmMMM/       \n\
        `yMmN`      `              NMMmMMMm       \n        oMNdd       `              mMNNMMMN       \n        /soyy-      `            `.mNNNNmmh       \n      `:+///+ys:`   `           .:/hMMMMms/`      \n\
  `--:++//////yNd+.            .://ohddh+:/-      \n  -////////////sNMm:           -++//+++//://:.    \n  -+///::://////oho.         `:hy+/:::/::::::/:.  \n  :o////::::://////-``````-/smMMo//://///////:.   \n\
  :++++++////////+odNmmmmNNMMMMNo/////////-.`     \n   ``.-:/+ooo++++osyo+////////+os++//++/.         \n           `.:/++/.             -/++/:.           \n                                                  \n\
What's that ? \n\
Hum... I'd say it's a kinda Penguin ! \n\
"

G_END_DECLS

#endif
