#!/bin/sh

read -p "Enter applet's name : " AppletName
if test -e "$AppletName"; then
    echo "Directory $AppletName already exists here; delete it before."
    exit 1
fi
export LibName=`echo $AppletName | tr "-" "_"`
export UpperName=`echo $LibName | tr "[a-z]" "[A-Z]"`
export LowerName=`echo $LibName | tr "[A-Z]" "[a-z]"`

read -p "Enter your name : " MyName
read -p "Enter an e-mail adress to contact you for bugs or congratulations : " MyMail
read -p "Enter the default label of your applet (Just type enter to leave it empty for the moment) :" AppletLabel


echo "creation de l'arborescence de l'applet $AppletName ..."
cp -r template "$AppletName"
find "$AppletName" -name ".svn" -execdir rm -rf .svn \; > /dev/null


cd "$AppletName"
sed -i "s/CID_APPLET_NAME/$AppletName/g" configure.ac
sed -i "s/CID_MY_NAME/$MyName/g" configure.ac
sed -i "s/CID_MY_MAIL/$MyMail/g" configure.ac
sed -i "s/CID_PKG/$UpperName/g" configure.ac
sed -i "s/pkgdatadir/${LowerName}datadir/g" configure.ac
sed -i "s/pkguserdirname/${LowerName}userdirname/g" configure.ac


cd ../src
sed -i "s/CID_APPLET_NAME/$AppletName/g" Makefile.am
sed -i "s/CID_LIB_NAME/$LibName/g" Makefile.am
sed -i "s/CID_PKG/$UpperName/g" Makefile.am
sed -i "s/pkgdatadir/${LowerName}datadir/g" Makefile.am

sed -i "s/CID_MY_NAME/$MyName/g" applet-init.c
sed -i "s/CID_APPLET_NAME/$AppletName/g" applet-init.c


cd ../po
sed -i "s/CID_APPLET_NAME/$AppletName/g" fr.po
sed -i "s/CID_MY_NAME/$MyName/g" fr.po
sed -i "s/CID_PKG/$UpperName/g" Makevars
sed -i "s/CID_PKG/$UpperName/g" Makefile.in.in


cd ..

autoreconf -isvf && ./configure --prefix=/usr && make

echo "now it's your turn ! type 'sudo make install' to install it."
