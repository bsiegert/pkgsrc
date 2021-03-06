# $NetBSD: Makefile,v 1.13 2013/07/12 10:45:04 jperkin Exp $
#

DISTNAME=	apache-tomcat-${TOMCAT_VER}
PKGREVISION=	1
CATEGORIES=	www
MASTER_SITES=	${MASTER_SITE_APACHE:=tomcat/tomcat-6/v${TOMCAT_VER}/bin/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://tomcat.apache.org/
COMMENT=	Implementation of Java Servlet and JavaServer Pages technologies
LICENSE=	apache-2.0

CONFLICTS+=	jakarta-tomcat55-[0-9]*
CONFLICTS+=	jakarta-tomcat-[0-9]*

NO_BUILD=	yes
USE_LANGUAGES=	# none
USE_JAVA=	run
USE_JAVA2=	1.5
USE_TOOLS+=	pax

PKG_DESTDIR_SUPPORT=	destdir

.include "../../mk/bsd.prefs.mk"

TOMCAT_VER=		6.0.37
TOMCAT_HOME=		${PREFIX}/share/tomcat
EGDIR=			${PREFIX}/share/examples/tomcat
DOCDIR=			${PREFIX}/share/doc/tomcat
RCD_SCRIPTS=		tomcat
TOMCAT_USER?=		tomcat
TOMCAT_GROUP?=		tomcat
USERGROUP_PHASE=	pre-install
PKG_GROUPS=		${TOMCAT_GROUP}
PKG_USERS=		${TOMCAT_USER}:${TOMCAT_GROUP}
PKG_GROUPS_VARS+=	TOMCAT_GROUP
PKG_USERS_VARS+=	TOMCAT_USER
PKG_HOME.tomcat=	${TOMCAT_HOME}
PKG_SHELL.tomcat=	${SH}
PKG_SYSCONFDIR.tomcat=	${TOMCAT_HOME}/conf
OWN_DIRS+=		${PKG_SYSCONFDIR.tomcat}
FILES_SUBST+=		JAVA_HOME=${PKG_JAVA_HOME} TOMCAT_HOME=${TOMCAT_HOME} \
			TOMCAT_USER=${TOMCAT_USER} TOMCAT_GROUP=${TOMCAT_GROUP}

PRIVATE_EGFILES=	server.xml tomcat-users.xml
PUBLIC_EGFILES=		catalina.policy catalina.properties \
			context.xml logging.properties web.xml
DOCFILES=		LICENSE NOTICE RELEASE-NOTES RUNNING.txt
WORK_DIRS=		work temp logs

.for f in ${PRIVATE_EGFILES}
CONF_FILES_PERMS+=	${EGDIR}/${f} ${PKG_SYSCONFDIR.tomcat}/${f} \
			${TOMCAT_USER} ${TOMCAT_GROUP} 0640
.endfor

.for f in ${PUBLIC_EGFILES}
CONF_FILES_PERMS+=	${EGDIR}/${f} ${PKG_SYSCONFDIR.tomcat}/${f} \
			${TOMCAT_USER} ${TOMCAT_GROUP} 0644
.endfor

.for d in ${WORK_DIRS}
OWN_DIRS_PERMS+=	${TOMCAT_HOME}/${d} ${TOMCAT_USER} ${TOMCAT_GROUP} 0755
.endfor

INSTALLATION_DIRS+=	${TOMCAT_HOME} ${EGDIR} ${DOCDIR}

do-install:
.for d in ${WORK_DIRS}
	${RM} -rf ${WRKSRC}/${d}
.endfor

.for f in ${DOCFILES}
	${INSTALL_DATA} ${WRKSRC}/${f} ${DESTDIR}${DOCDIR}/${f}
	${RM} -f ${WRKSRC}/${f}
.endfor

.for f in ${PRIVATE_EGFILES}
	${INSTALL_DATA} ${WRKSRC}/conf/${f} ${DESTDIR}${EGDIR}/${f}
	${RM} -f ${WRKSRC}/conf/${f}
.endfor

.for f in ${PUBLIC_EGFILES}
	${INSTALL_DATA} ${WRKSRC}/conf/${f} ${DESTDIR}${EGDIR}/${f}
	${RM} -f ${WRKSRC}/conf/${f}
.endfor

	${FIND} ${WRKSRC} -name \*.exe -type f -print | ${XARGS} ${RM} -f
	${FIND} ${WRKSRC} -name \*.bat -type f -print | ${XARGS} ${RM} -f

	cd ${WRKSRC} && ${PAX} -rw -pm . ${DESTDIR}${TOMCAT_HOME}

	${FIND} ${DESTDIR}${TOMCAT_HOME} -type d -print | \
		${XARGS} ${CHMOD} ${PKGDIRMODE}
	${FIND} ${DESTDIR}${TOMCAT_HOME} -type f -print | \
		${XARGS} ${CHMOD} ${SHAREMODE}
	${FIND} ${DESTDIR}${TOMCAT_HOME} -type f -name \*.sh -print | \
		${XARGS} ${CHMOD} ${BINMODE}

	${CHOWN} -R ${TOMCAT_USER}:${TOMCAT_GROUP} \
		${DESTDIR}${TOMCAT_HOME}/webapps
	${CHOWN} ${TOMCAT_USER}:${TOMCAT_GROUP} \
		${DESTDIR}${TOMCAT_HOME}/conf

.include "../../mk/java-vm.mk"
.include "../../mk/bsd.pkg.mk"
