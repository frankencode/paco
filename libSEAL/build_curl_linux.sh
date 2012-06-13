#! /bin/sh
VER=7.19.5
cd ..
rm -rf curl-$VER
tar xjvf curl-$VER.tar.bz2
cd curl-$VER
CC='ccache gcc' ./configure \
  --prefix=/home/frank/Work/local || exit 1
#  --disable-verbose \
#  --disable-ftp \
#  --disable-tftp \
#  --disable-ldap \
#  --disable-telnet \
#  --disable-sspi \
#  --enable-cookies \
#  --enable-ipv6 || exit 1
echo "Check build options (cookies and ipv6 !), press [RETURN] to continue..."
read
make -j2 || exit 2
make install || exit 3

