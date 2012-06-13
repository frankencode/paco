#!/bin/bash

BUILD_TYPE=Release # Release, MinSizeRel, Debug, RelWithDebInfo
BUILD_TARGET=Cocoa # Carbon, Cocoa
OSX_SYSROOT=/Developer/SDKs/MacOSX10.5.sdk
OSX_TARGET=10.5 # 10.4, 10.5
if [ $BUILD_TYPE == "Debug" ]; then
	OSX_ARCHS="x86_64"
	export PATH=/Users/frank/Work/qt-gitorious/qt-4.7-x86_64-reduced/bin:$PATH
else
	OSX_ARCHS="x86_64;i386;ppc" # ppc;i386;x86_64
	export PATH=/Users/frank/Work/qt-gitorious/qt-4.7/bin:$PATH
fi
BUILD_PREFIX=~/tmp
CONFIGURE=1
VERBOSE_MAKEFILE=0
BUILD_CURL=1
BUILD_FTL=1
BUILD_SEAL=1
BUILD_CODETIPS=1
BUILD_VIDE=1
BUILD_PTE=1
BUILD_PACU=1
EMBED_SEAL=1
TEST_RUN=0
if [ $BUILD_TYPE == "Debug" ]; then
	TEST_CMD="" # "", "gdb", "arch -ppc"
else
	TEST_CMD=""
fi
TEST_PARAM=""
BUNDLE=1
BUNDLE_ONLY=1
BUNDLE_VERSION=1.2.3
AUTOINCREMENT_BUILD_NUMBER=1
# IMAGE_NAME=pacu-$BUNDLE_VERSION-cocoa-x86_64
IMAGE_NAME=Pacu-$BUNDLE_VERSION
MAKEFLAGS=-j2
SOURCE_PREFIX=~/project
CURL_PACKAGE=~/project/curl-7.19.5.tar.bz2
