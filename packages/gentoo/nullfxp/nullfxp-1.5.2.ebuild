# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit qt4 eutils flag-o-matic

DESCRIPTION="cross platform SSH client."
HOMEPAGE="http://www.qtchina.net"
SRC_URI="http://nchc.dl.sourceforge.net/sourceforge/${PN}/${P}.tar.gz"
RESTRICT="primaryuri"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~ppc ~amd64"

IUSE="ssh-server"
RDEPEND="dev-libs/openssl
			>=x11-libs/qt-4.3.1
		  "
		 
DEPEND="${RDEPEND}"


src_compile() {
	qmake
	emake || die "make failed"
}

src_install() {
	emake DESTDIR=${D} install || die
	dodoc COPYING AUTHORS README
}
