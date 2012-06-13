SRC=$PWD
EXE=$BUILD_ROOT/Pacu/bin/Pacu
FTL=$BUILD_ROOT/libftl/lib/libftl.0.$DYLIB
CODETIPS=$BUILD_ROOT/libcodetips/lib/libcodetips.0.$DYLIB
CODETIPSCLIENT=$BUILD_ROOT/libcodetips/lib/libcodetipsclient.0.$DYLIB
VIDE=$BUILD_ROOT/libVIDE/lib/libvide.0.$DYLIB
CHARCOAL=$SRC/../charcoal
BUNDLE=$PWD/../Pacu.app
MAX_SIZE=80m
EXE_NAME=$(basename $EXE)
FTL_NAME=$(basename $FTL)
CODETIPS_NAME=$(basename $CODETIPS)
VIDE_NAME=$(basename $VIDE)
INSTALL_NAME_TOOL=$(which install_name_tool)

echo "Increment BuildNumber (QxActivation.cpp), version number (main.cpp / Info.plist) and library versions before release!"
echo "(Press [ENTER] to continue)"
read

if ping -c1 apple.com > /dev/null 2>&1; then
	xmllint --valid Info.plist --dtdvalid http://www.apple.com/DTDs/PropertyList-1.0.dtd --noout || exit 1
else
	echo "Warning, omitting validation of Info.plist (offline?)";
fi

rm -rf $BUNDLE
mkdir $BUNDLE
cd $BUNDLE

mkdir Contents
cd Contents

rsync -av --delete \
--exclude 'manual.txt' \
--exclude 'manual.sh' \
--exclude 'manual.css' \
--exclude '.DS_Store' \
$SRC/Resources/ Resources/
cp $SRC/Info.plist .

mkdir MacOS
$BUILD_ROOT/libSEAL/bin/embed --strip $EXE || exit 1
cp $EXE MacOS

mkdir Frameworks
# cp $FTL Frameworks
cp $CODETIPS Frameworks
cp $CODETIPSCLIENT Frameworks
cp $VIDE Frameworks
$INSTALL_NAME_TOOL -id @executable_path/../Frameworks/$VIDE_NAME Frameworks/$VIDE_NAME ||exit 2
$INSTALL_NAME_TOOL -change $VIDE @executable_path/../Frameworks/$VIDE_NAME MacOS/$EXE_NAME || exit 3
$INSTALL_NAME_TOOL -id @executable_path/../Frameworks/$CODETIPS_NAME Frameworks/$CODETIPS_NAME || exit 4
$INSTALL_NAME_TOOL -change $CODETIPS @executable_path/../Frameworks/$CODETIPS_NAME Frameworks/$CODETIPS_NAME || exit 5
# $INSTALL_NAME_TOOL -id @executable_path/../Frameworks/$FTL_NAME Frameworks/$FTL_NAME
# $INSTALL_NAME_TOOL -change $FTL @executable_path/../Frameworks/$FTL_NAME Frameworks/$VIDE_NAME
# $INSTALL_NAME_TOOL -change $FTL @executable_path/../Frameworks/$FTL_NAME MacOS/$EXE_NAME

rsync -av --delete --exclude '.git*' --exclude '.DS_Store' $CHARCOAL/ $BUNDLE/Contents/Resources/charcoal/
# rsync -av --delete --exclude '.git*' --exclude '.DS_Store' $SRC/qt-patches/ $BUNDLE/Contents/Resources/qt-patches/

cd $BUNDLE/..
macdeployqt $(basename $BUNDLE)
rm -rf $BUNDLE/Contents/Frameworks/QtNetwork*

cp $SRC/COPYING.* $BUNDLE/Contents/Frameworks

$BUILD_ROOT/libSEAL/bin/embed $BUNDLE/Contents/MacOS/$(basename $EXE)

otool -L $BUNDLE/Contents/MacOS/$(basename $EXE)

cd $BUNDLE/..
ROOT=$EXE_NAME-$BUNDLE_VERSION
rm -rf $ROOT
mkdir $ROOT
cp $SRC/../LICENSE $ROOT
cp $BUNDLE/Contents/Resources/Buy\ Now.webloc $ROOT
mv $BUNDLE $ROOT
ln -s Pacu.app/Contents/Resources/manual/manual.html $ROOT/Manual
ln -s /Applications $ROOT/Applications
IMAGE_NAME_TMP=$IMAGE_NAME\_tmp
rm -rf $IMAGE_NAME_TMP.dmg
rm -rf $IMAGE_NAME.dmg
hdiutil create -format UDRW -srcfolder $ROOT -size $MAX_SIZE $IMAGE_NAME_TMP.dmg

echo ""
echo "Now it's up to you to apply the package styling to $PWD/$IMAGE_NAME_TMP.dmg"
echo "Press <RETURN> when ready..."
read

hdiutil convert $IMAGE_NAME_TMP.dmg -format UDBZ -o $IMAGE_NAME.dmg # UDZO for zlib compressed
hdiutil internet-enable -no $IMAGE_NAME.dmg
du -h $IMAGE_NAME.dmg
