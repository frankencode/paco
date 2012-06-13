#! /bin/sh

BUILD=release
SRC=$PWD
FTL_MAJOR=0
FTL_MINOR=0
FTL_PATCH=9
CODETIPS_MAJOR=0
CODETIPS_MINOR=0
CODETIPS_PATCH=1
VIDE_MAJOR=0
VIDE_MINOR=1
VIDE_PATCH=0

# ---------------------------------------------

function findQt()
{
	n=0
	l=$(qmake -v)
	for i in $l; do
		if [ $n -eq 0 ]; then
			if [ $i == "Qt" ]; then
				let n=n+1
			fi
		elif [ $n -eq 1 ]; then
			if [ $i == "version" ]; then
				let n=n+1
			fi
		elif [ $n -eq 2 ]; then
			let n=0
			IFS_OLD=$IFS
			IFS='.'
			for j in $i; do
				if [ $n -eq 0 ]; then
					QT_MAJOR=$j
				elif [ $n -eq 1 ]; then
					QT_MINOR=$j
				elif [ $n -eq 2 ]; then
					QT_PATCH=$j
				fi
				let n=n+1
			done
			IFS=$IFS_OLD
			let n=3
		fi
		if [ $n -eq 3 ]; then
			if [ $i == "in" ]; then
				let n=n+1
			fi
		elif [ $n -eq 4 ]; then
			QT_PATH=$i/..
			let n=5
		fi
	done
}

findQt

FTL_VERSION=$FTL_MAJOR.$FTL_MINOR.$FTL_PATCH
CODETIPS_VERSION=$CODETIPS_MAJOR.$CODETIPS_MINOR.$CODETIPS_PATCH
VIDE_VERSION=$VIDE_MAJOR.$VIDE_MINOR.$VIDE_PATCH
QT_VERSION=$QT_MAJOR.$QT_MINOR.$QT_PATCH

EXE=$BUILD_ROOT/Pacu/bin/Pacu
ARCH=$(uname -m)

echo "Increment BuildNumber (QxActivation.cpp), product version (main.cpp / Info.plist) and library versions before release!"
echo "(Press [ENTER] to continue)"
read

$BUILD_ROOT/libSEAL/bin/embed --strip $EXE || exit

cd $SRC/..
rm -rf $IMAGE_NAME/$ARCH
mkdir $IMAGE_NAME
mkdir $IMAGE_NAME/$ARCH
mkdir $IMAGE_NAME/shared

cd $IMAGE_NAME
cp $SRC/../LICENSE COPYING
rsync -av --delete \
--exclude 'manual.txt' \
--exclude 'manual.sh' \
--exclude 'manual.css' \
$SRC/Resources/manual/ shared/manual/
rm -rf *.desktop
rsync -av --delete --exclude '.git*' $SRC/../charcoal/ shared/charcoal/
# rsync -av --delete --exclude '.git*' $SRC/qt-patches/ shared/qt-patches/
cp $SRC/logo/pacu.svg shared
cp $SRC/setup.sh .
cp $EXE $ARCH
# cp $BUILD_ROOT/libftl/lib/libftl.so.$FTL_VERSION $ARCH
cp $BUILD_ROOT/libcodetips/lib/libcodetips.so.$CODETIPS_VERSION $ARCH || exit 1
cp $BUILD_ROOT/libcodetips/lib/libcodetipsclient.so.$CODETIPS_VERSION $ARCH || exit 1
cp $BUILD_ROOT/libVIDE/lib/libvide.so.$VIDE_VERSION $ARCH || exit 1
cp $QT_PATH/lib/libQtCore.so.$QT_VERSION $ARCH || exit 1
cp $QT_PATH/lib/libQtGui.so.$QT_VERSION $ARCH || exit 1
cp $QT_PATH/lib/libQtScript.so.$QT_VERSION $ARCH || exit 1
cd $ARCH
ln -s Pacu pacu
chrpath -r '$ORIGIN' Pacu || exit
chrpath -c Pacu || exit
$BUILD_ROOT/libSEAL/bin/embed ./Pacu
# ln -s libftl.so.$FTL_VERSION libftl.so.$FTL_MAJOR
# ln -s libftl.so.$FTL_VERSION libftl.so
ln -s libcodetips.so.$CODETIPS_VERSION libcodetips.so.$CODETIPS_MAJOR
ln -s libcodetips.so.$CODETIPS_VERSION libcodetips.so
ln -s libcodetipsclient.so.$CODETIPS_VERSION libcodetipsclient.so.$CODETIPS_MAJOR
ln -s libcodetipsclient.so.$CODETIPS_VERSION libcodetipsclient.so
ln -s libvide.so.$VIDE_VERSION libvide.so.$VIDE_MAJOR
ln -s libvide.so.$VIDE_VERSION libvide.so
ln -s libQtCore.so.$QT_VERSION libQtCore.so.$QT_MAJOR
ln -s libQtCore.so.$QT_VERSION libQtCore.so
ln -s libQtGui.so.$QT_VERSION libQtGui.so.$QT_MAJOR
ln -s libQtGui.so.$QT_VERSION libQtGui.so
ln -s libQtScript.so.$QT_VERSION libQtScript.so.$QT_MAJOR
ln -s libQtScript.so.$QT_VERSION libQtScript.so
cp $SRC/COPYING.* .

cd $SRC/..
tar -cv $IMAGE_NAME | bzip2 > $IMAGE_NAME.tar.bz2
