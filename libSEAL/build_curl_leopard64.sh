#! /bin/sh
VER=7.19.5
cd ..
rm -rf curl-$VER
tar xvf curl-$VER.tar
cd curl-$VER
CFLAGS='-m64' CC='ccache gcc' ./configure \
  --prefix=/Users/frank/Work/local_leopard64 \
  --disable-verbose \
  --disable-ftp \
  --disable-tftp \
  --disable-ldap \
  --disable-telnet \
  --disable-sspi \
  --enable-cookies \
  --enable-ipv6 || exit 1
echo "Check build options (cookies and ipv6 !), press [RETURN] to continue..."
read
make -j2 || exit 2
make install || exit 3

