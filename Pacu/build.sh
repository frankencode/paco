#!/bin/bash

if [ $(uname) == "Darwin" ]; then
. ./build_options_Darwin.sh
else
. ./build_options_$(uname)_$(uname -m).sh
fi

CONFIG_PATH=$PWD/config

DYLIB="so"
if [ $(uname) == "Darwin" ]; then
	DYLIB="dylib"
fi

if [ $(uname) == "Darwin" ]; then
	BUILD_ROOT=$BUILD_PREFIX/$BUILD_TYPE$BUILD_TARGET
else
	BUILD_ROOT=$BUILD_PREFIX/$BUILD_TYPE$BUILD_TARGET$(uname -m)
fi
# TMPDIR=$BUILD_PREFIX/$BUILD_TYPE$BUILD_TARGET/tmp

if [ ! -e $BUILD_ROOT ]; then
	# easing daily build tree bootstraping
	CONFIGURE=1
	BUILD_CURL=1
	BUILD_FTL=1
	BUILD_SEAL=1
	BUILD_VIDE=1
	BUILD_PTE=1
	BUILD_PACU=1
	EMBED_SEAL=1
fi

if [ $BUNDLE_ONLY -eq 1 ]; then
	CONFIGURE=0
	BUILD_CURL=0
	BUILD_FTL=0
	BUILD_SEAL=0
	BUILD_VIDE=0
	BUILD_PTE=0
	BUILD_PACU=0
	EMBED_SEAL=0
	TEST_RUN=0
	AUTOINCREMENT_BUILD_NUMBER=0
fi

mkdir -p $BUILD_ROOT
cd $BUILD_ROOT

if [ $BUILD_CURL -eq 1 ]; then
	CURL_SOURCE=$BUILD_ROOT/$(basename $(basename $CURL_PACKAGE .tar.gz) .tar.bz2)
	if [ ! -e $CURL_SOURCE ]; then
		tar xjf $CURL_PACKAGE
	fi
	mkdir -p $BUILD_ROOT/curl
	mkdir -p $BUILD_ROOT/curl-build
	cd $BUILD_ROOT/curl-build
	if [ $CONFIGURE -eq 1 ]; then
		if [ $(uname) == "Darwin" ]; then
			ARCHS=''
			IFS_SAVED=$IFS
			IFS=';'
			for i in $OSX_ARCHS; do
				if [ ! $ARCHS == '' ]; then
					ARCHS="$ARCHS "
				fi
				ARCHS="$ARCHS-arch $i"
			done
			IFS=$IFS_SAVED
			if [ $OSX_ARCHS == "i386" ]; then
				ARCHS="-m32"
			fi
			cat 1>>configure.patch <<-FIN
3316c3316
< #define OS "\${host}"
---
> #define OS "universal-\${host_vendor}-\${host_os}"
FIN
			patch $CURL_SOURCE/configure configure.patch

			CFLAGS="-Os -isysroot $OSX_SYSROOT -mmacosx-version-min=$OSX_TARGET $ARCHS" \
			LDFLAGS="-Wl,-syslibroot,$OSX_SYSROOT,-macosx_version_min,$OSX_TARGET $ARCHS" \
			$CURL_SOURCE/configure \
				--disable-dependency-tracking \
				--enable-static \
				--disable-shared \
				--prefix=$BUILD_ROOT/curl \
				--with-ssl=$OSX_SYSROOT/usr \
				--disable-verbose \
				--disable-ftp \
				--disable-tftp \
				--disable-ldap \
				--disable-telnet \
				--disable-sspi \
				--enable-cookies \
				--enable-ipv6 || exit 1

			cat 1>>config.h.ed <<-FIN
/SIZEOF_LONG/c
#ifdef __LP64__
#define SIZEOF_LONG 8
#else /* !__LP64__ */
#define SIZEOF_LONG 4
#endif /* __LP64__ */
.
/SIZEOF_SIZE_T/c
#ifdef __LP64__
#define SIZEOF_SIZE_T 8
#else /* !__LP64__ */
#define SIZEOF_SIZE_T 4
#endif /* __LP64__ */
.
/SIZEOF_TIME_T/c
#ifdef __LP64__
#define SIZEOF_TIME_T 8
#else /* !__LP64__ */
#define SIZEOF_TIME_T 4
#endif /* __LP64__ */
.
/SIZEOF_VOIDP/c
#ifdef __LP64__
#define SIZEOF_VOIDP 8
#else /* !__LP64__ */
#define SIZEOF_VOIDP 4
#endif /* __LP64__ */
.
w
FIN
			cat 1>>curlbuild.h.ed <<-FIN
/define CURL_SIZEOF_LONG/c
#ifdef __LP64__
#define CURL_SIZEOF_LONG 8
#else /* !__LP64__ */
#define CURL_SIZEOF_LONG 4
#endif /* __LP64__ */
.
w
FIN
			ed - $BUILD_ROOT/curl-build/lib/config.h < config.h.ed || exit 1
			ed - $BUILD_ROOT/curl-build/src/config.h < config.h.ed || exit 1
			ed - $BUILD_ROOT/curl-build/include/curl/curlbuild.h < curlbuild.h.ed || exit 1
		else
			$CURL_SOURCE/configure \
				--prefix=$BUILD_ROOT/curl \
				--with-gnutls=/usr \
				--without-ssl \
				--without-zlib \
				--disable-ldap || exit 1
		fi
	fi
	make clean || exit 2
	make $MAKEFLAGS || exit 3
	make -k install
fi

function osxOptions()
{
	# export MACOSX_DEPLOYMENT_TARGET=10.4
	if [ $(uname) == "Darwin" ]; then
		cat 1>>CacheInit.cmake <<-FIN
SET(CMAKE_OSX_SYSROOT $OSX_SYSROOT CACHE PATH "" FORCE)
SET(CMAKE_OSX_DEPLOYMENT_TARGET $OSX_TARGET CACHE STRING "" FORCE)
SET(CMAKE_OSX_ARCHITECTURES $OSX_ARCHS CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS "-isysroot $OSX_SYSROOT -mmacosx-version-min=$OSX_TARGET" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS "-isysroot $OSX_SYSROOT -mmacosx-version-min=$OSX_TARGET" CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-macosx_version_min,$OSX_TARGET" CACHE STRING "" FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-macosx_version_min,$OSX_TARGET" CACHE STRING "" FORCE)
FIN
	fi
}

function genericFlags()
{
	if [ $VERBOSE_MAKEFILE -eq 1 ]; then
		cat 1>>CacheInit.cmake <<-FIN
SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "" FORCE)
FIN
	else
		cat 1>>CacheInit.cmake <<-FIN
SET(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" FORCE)
FIN
	fi
}

if [ $BUILD_FTL -eq 1 ]; then
	mkdir -p $BUILD_ROOT/libftl
	cd $BUILD_ROOT/libftl
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(CMAKE_CXX_FLAGS "-fPIC" CACHE STRING "" FORCE)
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/libftl || exit
	fi
	make $MAKEFLAGS ftl || exit
fi

if [ $BUILD_SEAL -eq 1 ]; then
	mkdir -p $BUILD_ROOT/libSEAL
	cd $BUILD_ROOT/libSEAL
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
SET(CMAKE_MODULE_PATH $SOURCE_PREFIX/libftl/cmake CACHE PATH "" FORCE)
SET(FTL_INCLUDE_DIR $SOURCE_PREFIX/libftl CACHE PATH "" FORCE)
SET(FTL_LIBRARY $BUILD_ROOT/libftl/lib/libftl.a CACHE PATH "" FORCE)
SET(CURL_INCLUDE_DIR $BUILD_ROOT/curl/include CACHE PATH "" FORCE)
SET(CURL_LIBRARY $BUILD_ROOT/curl/lib/libcurl.a CACHE PATH "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/libSEAL || exit
	fi
	make $MAKEFLAGS || exit
fi

if [ $BUILD_CODETIPS -eq 1 ]; then
	mkdir -p $BUILD_ROOT/libcodetips
	cd $BUILD_ROOT/libcodetips
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
SET(CMAKE_MODULE_PATH $SOURCE_PREFIX/libftl/cmake CACHE PATH "" FORCE)
SET(FTL_INCLUDE_DIR $SOURCE_PREFIX/libftl CACHE PATH "" FORCE)
SET(FTL_LIBRARY $BUILD_ROOT/libftl/lib/libftl.a CACHE PATH "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/libcodetips || exit
	fi
	make $MAKEFLAGS || exit
fi

if [ $BUILD_VIDE -eq 1 ]; then
	mkdir -p $BUILD_ROOT/libVIDE
	cd $BUILD_ROOT/libVIDE
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
SET(CMAKE_MODULE_PATH $SOURCE_PREFIX/libftl/cmake CACHE PATH "" FORCE)
SET(FTL_INCLUDE_DIR $SOURCE_PREFIX/libftl CACHE PATH "" FORCE)
SET(FTL_LIBRARY $BUILD_ROOT/libftl/lib/libftl.a CACHE PATH "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/libVIDE || exit
	fi
	make $MAKEFLAGS vide || exit
fi

if [ $BUILD_PTE -eq 1 ]; then
	mkdir -p $BUILD_ROOT/libPTE
	cd $BUILD_ROOT/libPTE
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
SET(CMAKE_MODULE_PATH
	$SOURCE_PREFIX/libftl/cmake
	$SOURCE_PREFIX/libcodetips/cmake
	CACHE PATH "" FORCE
)
SET(FTL_INCLUDE_DIR $SOURCE_PREFIX/libftl CACHE PATH "" FORCE)
SET(FTL_LIBRARY $BUILD_ROOT/libftl/lib/libftl.a CACHE PATH "" FORCE)
SET(CODETIPS_INCLUDE_DIR $SOURCE_PREFIX/libcodetips CACHE PATH "" FORCE)
SET(CODETIPS_LIBRARY $BUILD_ROOT/libcodetips/lib/libcodetips.$DYLIB CACHE PATH "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/libPTE || exit
	fi
	make $MAKEFLAGS pte || exit
fi

if [ $BUNDLE -eq 1 ]; then
	if [ $AUTOINCREMENT_BUILD_NUMBER -eq 1 ]; then
		PWD_SAVE=$PWD
		cd $SOURCE_PREFIX/Pacu
		BN=$(cat BuildNumber)
		let BN=BN+1
		echo "Autoincrementing build number to $BN"
		echo "<Press any key to continue>"
		read
		echo "$BN" > BuildNumber
		cd $PWD_SAVED
		BUILD_PACU=1
	fi
fi

if [ $BUILD_PACU -eq 1 ]; then
	mkdir -p $BUILD_ROOT/Pacu
	cd $BUILD_ROOT/Pacu
	if [ $CONFIGURE -eq 1 ]; then
		cat 1>CacheInit.cmake <<-FIN
SET(CMAKE_BUILD_TYPE $BUILD_TYPE CACHE STRING "" FORCE)
SET(CMAKE_MODULE_PATH
	$SOURCE_PREFIX/libftl/cmake
	$SOURCE_PREFIX/libcodetips/cmake
	$SOURCE_PREFIX/libSEAL/cmake
	$SOURCE_PREFIX/libVIDE/cmake
	$SOURCE_PREFIX/libPTE/cmake
	CACHE PATH "" FORCE
)
SET(FTL_INCLUDE_DIR $SOURCE_PREFIX/libftl CACHE PATH "" FORCE)
SET(FTL_LIBRARY $BUILD_ROOT/libftl/lib/libftl.a CACHE PATH "" FORCE)
SET(CODETIPS_INCLUDE_DIR $SOURCE_PREFIX/libcodetips CACHE PATH "" FORCE)
SET(CODETIPS_LIBRARY $BUILD_ROOT/libcodetips/lib/libcodetips.$DYLIB CACHE PATH "" FORCE)
SET(SEAL_INCLUDE_DIR $SOURCE_PREFIX/libSEAL CACHE PATH "" FORCE)
SET(SEAL_LIBRARY $BUILD_ROOT/libSEAL/lib/libseal.a CACHE PATH "" FORCE)
SET(VIDE_INCLUDE_DIR $SOURCE_PREFIX/libVIDE CACHE PATH "" FORCE)
SET(VIDE_LIBRARY $BUILD_ROOT/libVIDE/lib/libvide.$DYLIB CACHE PATH "" FORCE)
SET(PTE_INCLUDE_DIR $SOURCE_PREFIX/libPTE CACHE PATH "" FORCE)
SET(PTE_LIBRARY $BUILD_ROOT/libPTE/lib/libpte.a CACHE PATH "" FORCE)
SET(CURL_INCLUDE_DIR $BUILD_ROOT/curl/include CACHE PATH "" FORCE)
SET(CURL_LIBRARY $BUILD_ROOT/curl/lib/libcurl.a CACHE PATH "" FORCE)
SET(PACU_BUILD_TARGET $BUILD_TARGET CACHE PATH "" FORCE)
FIN
		osxOptions
		genericFlags
		cmake -C CacheInit.cmake $SOURCE_PREFIX/Pacu || exit
	fi
	make $MAKEFLAGS || exit
fi

if [ $EMBED_SEAL -eq 1 ]; then
	$BUILD_ROOT/libSEAL/bin/embed $BUILD_ROOT/Pacu/bin/Pacu
fi

if [ $TEST_RUN -eq 1 ]; then
	echo "Executing $TEST_CMD $BUILD_ROOT/Pacu/bin/Pacu $TEST_PARAM"
	$TEST_CMD $BUILD_ROOT/Pacu/bin/Pacu $TEST_PARAM
fi

if [ $BUNDLE -eq 1 ]; then
	cd $SOURCE_PREFIX/Pacu

	if [ $(uname) == "Darwin" ]; then
		. ./bundle_mac.sh
	else
		. ./bundle_linux.sh
	fi
fi
