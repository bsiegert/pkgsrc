# $NetBSD: Makefile,v 1.23 2013/10/10 14:42:05 ryoon Exp $

.include "../../meta-pkgs/xfce4/Makefile.common"

DISTNAME=	xfconf-${XFCE4_VERSION}
PKGREVISION=	21
CATEGORIES=	devel
COMMENT=	Xfce client-server configuration storage and query system

PKGCONFIG_OVERRIDE=	xfconf/libxfconf-0.pc.in

PERL5_CONFIGURE=	no
PERL5_PACKLIST=		auto/Xfce4/Xfconf/.packlist

CONFIGURE_ARGS+=	--with-perl-options=${MAKE_PARAMS}

MAKE_DIRS+=	${PREFIX}/include/xfce4

BUILDLINK_API_DEPENDS.dbus+=		dbus>=1.0.0
BUILDLINK_API_DEPENDS.dbus-glib+=	dbus-glib>=0.72

.include "../../devel/p5-glib2/buildlink3.mk"
.include "../../sysutils/dbus/buildlink3.mk"
.include "../../sysutils/dbus-glib/buildlink3.mk"
.include "../../x11/libxfce4util/buildlink3.mk"
.include "../../lang/perl5/module.mk"
.include "../../mk/bsd.pkg.mk"
