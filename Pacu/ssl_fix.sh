#!/bin/sh

if [ $(uname -m) == "x86_64" ]; then
	LIBPATH=/usr/lib64
else
	LIBPATH=/usr/lib
fi

ln -s $LIBPATH/libssl.so.0.9.8* ../lib/libssl.so.0.9.8
ln -s $LIBPATH/libcrypto.so.0.9.8* ../lib/libcrypto.so.0.9.8
