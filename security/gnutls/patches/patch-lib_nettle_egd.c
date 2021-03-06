$NetBSD: patch-lib_nettle_egd.c,v 1.2 2013/11/29 22:55:29 wiz Exp $

http://lists.gnupg.org/pipermail/gnutls-devel/2013-November/006588.html

--- lib/nettle/egd.c.orig	2013-11-10 17:59:14.000000000 +0000
+++ lib/nettle/egd.c
@@ -155,12 +155,10 @@ int _rndegd_connect_socket(void)
 
 	fd = socket(LOCAL_SOCKET_TYPE, SOCK_STREAM, 0);
 	if (fd == -1) {
-		_gnutls_debug_log("can't create unix domain socket: %s\n",
-				  strerror(errno));
+		_gnutls_debug_log("can't create unix domain socket\n");
 		return -1;
 	} else if (connect(fd, (struct sockaddr *) &addr, addr_len) == -1) {
-		_gnutls_debug_log("can't connect to EGD socket `%s': %s\n",
-				  name, strerror(errno));
+		_gnutls_debug_log("can't connect to EGD socket `%s'\n", name);
 		close(fd);
 		fd = -1;
 	}
@@ -202,13 +200,11 @@ int _rndegd_read(int *fd, void *_output,
 	buffer[1] = nbytes;
 
 	if (do_write(*fd, buffer, 2) == -1)
-		_gnutls_debug_log("can't write to the EGD: %s\n",
-				  strerror(errno));
+		_gnutls_debug_log("can't write to the EGD\n");
 
 	n = do_read(*fd, buffer, 1);
 	if (n == -1) {
-		_gnutls_debug_log("read error on EGD: %s\n",
-				  strerror(errno));
+		_gnutls_debug_log("read error on EGD\n");
 		do_restart = 1;
 		goto restart;
 	}
@@ -217,8 +213,7 @@ int _rndegd_read(int *fd, void *_output,
 	if (n) {
 		n = do_read(*fd, buffer, n);
 		if (n == -1) {
-			_gnutls_debug_log("read error on EGD: %s\n",
-					  strerror(errno));
+			_gnutls_debug_log("read error on EGD\n");
 			do_restart = 1;
 			goto restart;
 		}
@@ -240,12 +235,10 @@ int _rndegd_read(int *fd, void *_output,
 		buffer[0] = 2;	/* blocking */
 		buffer[1] = nbytes;
 		if (do_write(*fd, buffer, 2) == -1)
-			_gnutls_debug_log("can't write to the EGD: %s\n",
-					  strerror(errno));
+			_gnutls_debug_log("can't write to the EGD\n");
 		n = do_read(*fd, buffer, nbytes);
 		if (n == -1) {
-			_gnutls_debug_log("read error on EGD: %s\n",
-					  strerror(errno));
+			_gnutls_debug_log("read error on EGD\n");
 			do_restart = 1;
 			goto restart;
 		}
