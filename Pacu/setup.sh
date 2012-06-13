#! /bin/bash

ARCH=$(uname -m)

PWD_SAVED=$PWD
cd $(dirname $0)
PACKAGE=$PWD
cd $PWD_SAVED

cat 1> Pacu.desktop <<EOF
[Desktop Entry]
Version=1.2
Encoding=UTF-8
Name=Pacu
Type=Application
Terminal=false
Icon=$PACKAGE/shared/pacu.svg
Exec=$PACKAGE/$ARCH/Pacu %F
GenericName=
EOF

chmod ugo+rx Pacu.desktop
