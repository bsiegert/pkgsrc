# $NetBSD: Makefile,v 1.14 2013/05/31 12:41:28 wiz Exp $

DISTNAME=	libintl-perl-1.23
PKGNAME=	p5-${DISTNAME:S/libintl-perl/Locale-libintl/}
PKGREVISION=	1
CATEGORIES=	misc devel perl5
MASTER_SITES=	${MASTER_SITE_PERL_CPAN:=Locale/}

MAINTAINER=	peter.schuller@infidyne.com
HOMEPAGE=	http://search.cpan.org/dist/libintl-perl/
COMMENT=	Perl internationalization library
LICENSE=	gnu-lgpl-v2.1

PKG_INSTALLATION_TYPES=	overwrite pkgviews

PERL5_PACKLIST=		auto/libintl-perl/.packlist
# Interpreter /bin/false
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/Messages.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/Recode.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/Recode/*.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/RecodeData.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/RecodeData/*.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/TextDomain.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/Util.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/gettext_dumb.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/gettext_pp.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/gettext_xs.pm
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/gettext_xs.pod
CHECK_INTERPRETER_SKIP+=${PERL5_SUB_INSTALLVENDORLIB}/Locale/libintlFAQ.pod

.include "../../devel/gettext-lib/buildlink3.mk"
.include "../../lang/perl5/module.mk"
.include "../../mk/bsd.pkg.mk"
