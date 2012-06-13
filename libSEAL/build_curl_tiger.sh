#! /bin/sh
VER=7.19.5
cd ..
rm -rf curl-$VER
tar xvf curl-$VER.tar
cd curl-$VER
CC='ccache gcc-4.2' ./configure \
  --prefix=/Users/frank/Work/local_tiger \
  --disable-verbose \
  --disable-ftp \
  --disable-tftp \
  --disable-ldap \
  --disable-telnet \
  --disable-sspi \
  --enable-cookies \
  --enable-ipv6 || exit 1
echo "Check build options (gcc, cookies and ipv6 !), press [RETURN] to continue..."
read
make -j2 || exit 2
make install || exit 3

