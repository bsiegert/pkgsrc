# $NetBSD: Makefile,v 1.3 2013/12/10 14:15:49 jperkin Exp $

DISTNAME=	sslscan-1.8.2
CATEGORIES=	security
MASTER_SITES=	${MASTER_SITE_SOURCEFORGE:=sslscan/}
EXTRACT_SUFX=	.tgz

MAINTAINER=	tron@NetBSD.org
HOMEPAGE=	http://sourceforge.net/projects/sslscan/
COMMENT=	Fast SSL Scanner
LICENSE=	gnu-gpl-v3

INSTALLATION_DIRS+=	bin
INSTALLATION_DIRS+=	${PKGMANDIR}/man1

LDFLAGS.SunOS+=	-lsocket -lnsl

pre-install:
	${RM} -f ${WRKSRC}/INSTALL

.include "../../security/openssl/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
