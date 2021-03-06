# $NetBSD: Makefile,v 1.25 2013/10/10 14:42:01 ryoon Exp $
#

DISTNAME=	gnome-chemistry-utils-0.12.13
PKGREVISION=	14
CATEGORIES=	biology
MASTER_SITES=	http://download-mirror.savannah.gnu.org/releases/gchemutils/0.12/
EXTRACT_SUFX=	.tar.bz2

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://gchemutils.nongnu.org/
COMMENT=	GChemPaint, a 2D chemical editor and other programs
LICENSE=	gnu-gpl-v2 AND gnu-fdl-v1.1

GNU_CONFIGURE=	yes
USE_LIBTOOL=	yes
USE_TOOLS+=	gmake intltool pkg-config msgfmt xgettext msgmerge perl
USE_LANGUAGES=	c c++

GCONF_SCHEMAS+=	gchemutils.schemas
GCONF_SCHEMAS+=	gchempaint-arrows.schemas
GCONF_SCHEMAS+=	gcrystal.schemas
GCONF_SCHEMAS+=	gchempaint.schemas

BUILDLINK_TRANSFORM+=rm:-DGTK_DISABLE_DEPRECATED
BUILDLINK_TRANSFORM+=rm:-ldl

CONFIGURE_ARGS+=	--disable-update-databases

.include "../../devel/GConf/schemas.mk"
.include "../../databases/shared-mime-info/buildlink3.mk"
.include "../../databases/gnome-mime-data/buildlink3.mk"
.include "../../textproc/gnome-doc-utils/buildlink3.mk"
.include "../../mk/omf-scrollkeeper.mk"
.include "../../graphics/gtkglext/buildlink3.mk"
.include "../../misc/goffice0.8/buildlink3.mk"
.include "../../sysutils/desktop-file-utils/desktopdb.mk"
.include "../../biology/chemical-mime-data/buildlink3.mk"
.include "../../biology/bodr/buildlink3.mk"
.include "../../biology/openbabel/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
