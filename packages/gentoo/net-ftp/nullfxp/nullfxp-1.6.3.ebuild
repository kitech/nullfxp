# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils qt4 flag-o-matic

DESCRIPTION="Cross platform SSH client with GUI."
HOMEPAGE="http://www.qtchina.net"
SRC_URI="http://nchc.dl.sourceforge.net/sourceforge/${PN}/${P}.tar.bz2"
RESTRICT="primaryuri"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~ppc ~amd64"

IUSE="ssh-server openssl gcrypt"
RDEPEND="dev-libs/openssl
         >=x11-libs/qt-4.3.1
		 "
		 
DEPEND="${RDEPEND}"

src_compile() {
    PWD=`pwd`
    cd src/libssh2/
    ./configure --disable-shared --disable-debug --disable-rpath
    rm -v Makefile src/Makefile
    echo ${S}
    cd ${S}
	qmake
	emake || die "make failed"
}

src_install() {
	emake DESTDIR=${D} install || die
	dodoc COPYING AUTHORS README
}
