#! /bin/sh
# TARGET=silver:~
TARGET=mini:\~/Work
FLAGS="-aHv --exclude .git --exclude BuildNumber --delete"
rsync $FLAGS ../libftl/ $TARGET/libftl/
rsync $FLAGS ../libVIDE/ $TARGET/libVIDE/
rsync $FLAGS ../libSEAL/ $TARGET/libSEAL/
rsync $FLAGS ../libPTE/ $TARGET/libPTE/
rsync $FLAGS ../Pacu/ $TARGET/Pacu/
rsync $FLAGS ../charcoal/ $TARGET/charcoal/
