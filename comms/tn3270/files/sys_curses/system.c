/*	$NetBSD: system.c,v 1.1.1.1 2010/01/17 01:33:24 dholland Exp $	*/
/*	From NetBSD: system.c,v 1.21 2006/10/07 17:27:57 elad Exp 	*/

/*-
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)system.c	4.5 (Berkeley) 4/26/91";
#else
__RCSID("$NetBSD: system.c,v 1.1.1.1 2010/01/17 01:33:24 dholland Exp $");
#endif
#endif /* not lint */

#include <sys/types.h>

#if     defined(pyr)
#define fd_set fdset_t
#endif  /* defined(pyr) */

/*
 * Wouldn't it be nice if these REALLY were in <sys/inode.h>?  Or,
 * equivalently, if <sys/inode.h> REALLY existed?
 */
#define	IREAD	00400
#define	IWRITE	00200

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../general/general.h"
#include "../ctlr/api.h"
#include "../api/api_exch.h"
#include "telextrn.h"
#include "externs.h"

#include "../general/globals.h"

#ifndef	FD_SETSIZE
/*
 * The following is defined just in case someone should want to run
 * this telnet on a 4.2 system.
 *
 */

#define	FD_SET(n, p)	((p)->fds_bits[0] |= (1<<(n)))
#define	FD_CLR(n, p)	((p)->fds_bits[0] &= ~(1<<(n)))
#define	FD_ISSET(n, p)	((p)->fds_bits[0] & (1<<(n)))
#define FD_ZERO(p)	((p)->fds_bits[0] = 0)

#endif

static int shell_pid = 0;
static char key[50];			/* Actual key */
static char *keyname;			/* Name of file with key in it */

static char *ourENVlist[200];		/* Lots of room */

static int
    sock = -1,				/* Connected socket */
    serversock;				/* Server (listening) socket */

static enum { DEAD, UNCONNECTED, CONNECTED } state;

static long
    storage_location;		/* Address we have */
static short
    storage_length = 0;		/* Length we have */
static int
    storage_must_send = 0,	/* Storage belongs on other side of wire */
    storage_accessed = 0;	/* The storage is accessed (so leave alone)! */

static long storage[1000];

static union REGS inputRegs;
static struct SREGS inputSregs;

extern int apitrace;

static void kill_connection(void);
static int nextstore(void);
static int doreject(char *);
static int doassociate(void);
static int getstorage(long, int, int);
static int doconnect(void);
static void child_died(int);

static void
kill_connection()
{
    state = UNCONNECTED;
    if (sock != -1) {
	(void) close(sock);
	sock = -1;
    }
}


static int
nextstore()
{
    struct storage_descriptor sd;

    if (api_exch_intype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	storage_length = 0;
	return -1;
    }
    storage_length = sd.length;
    storage_location = sd.location;
    if (storage_length > sizeof storage) {
	fprintf(stderr, "API client tried to send too much storage (%d).\n",
		storage_length);
	storage_length = 0;
	return -1;
    }
    if (api_exch_intype(EXCH_TYPE_BYTES, storage_length, (char *)storage)
							== -1) {
	storage_length = 0;
	return -1;
    }
    return 0;
}


static int
doreject(message)
char	*message;
{
    struct storage_descriptor sd;
    int length = strlen(message);

    if (api_exch_outcommand(EXCH_CMD_REJECTED) == -1) {
	return -1;
    }
    sd.length = length;
    if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	return -1;
    }
    if (api_exch_outtype(EXCH_TYPE_BYTES, length, message) == -1) {
	return -1;
    }
    return 0;
}


/*
 * doassociate()
 *
 * Negotiate with the other side and try to do something.
 *
 * Returns:
 *
 *	-1:	Error in processing
 *	 0:	Invalid password entered
 *	 1:	Association OK
 */

static int
doassociate()
{
    struct passwd *pwent;
    char
	promptbuf[100],
	buffer[200];
    struct storage_descriptor sd;

    if (api_exch_intype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	return -1;
    }
    if (sd.length >= sizeof buffer) {
	doreject("(internal error) Authentication key too long");
	return -1;
    }
    if (api_exch_intype(EXCH_TYPE_BYTES, sd.length, buffer) == -1) {
	return -1;
    }
    buffer[sd.length] = 0;

    if (strcmp(buffer, key) != 0) {
	if ((pwent = getpwuid((int)geteuid())) == 0) {
	    return -1;
	}
	sprintf(promptbuf, "Enter password for user %s:", pwent->pw_name);
	if (api_exch_outcommand(EXCH_CMD_SEND_AUTH) == -1) {
	    return -1;
	}
	sd.length = strlen(promptbuf);
	if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd)
									== -1) {
	    return -1;
	}
	if (api_exch_outtype(EXCH_TYPE_BYTES, strlen(promptbuf), promptbuf)
									== -1) {
	    return -1;
	}
	sd.length = strlen(pwent->pw_name);
	if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd)
									== -1) {
	    return -1;
	}
	if (api_exch_outtype(EXCH_TYPE_BYTES,
			    strlen(pwent->pw_name), pwent->pw_name) == -1) {
	    return -1;
	}
	if (api_exch_incommand(EXCH_CMD_AUTH) == -1) {
	    return -1;
	}
	if (api_exch_intype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd)
									== -1) {
	    return -1;
	}
	sd.length = sd.length;
	if (sd.length > sizeof buffer) {
	    doreject("Password entered was too long");
	    return -1;
	}
	if (api_exch_intype(EXCH_TYPE_BYTES, sd.length, buffer) == -1) {
	    return -1;
	}
	buffer[sd.length] = 0;

	/* Is this the correct password? */
	if (strlen(pwent->pw_name)) {
	    const char *ptr;
	    int i;

	    ptr = pwent->pw_name;
	    i = 0;
	    while (i < sd.length) {
		buffer[i++] ^= *ptr++;
		if (*ptr == 0) {
		    ptr = pwent->pw_name;
		}
	    }
	}
	if (strcmp(crypt(buffer, pwent->pw_passwd), pwent->pw_passwd) != 0) {
	    doreject("Invalid password");
	    sleep(10);		/* Don't let us do too many of these */
	    return 0;
	}
    }
    if (api_exch_outcommand(EXCH_CMD_ASSOCIATED) == -1) {
	return -1;
    } else {
	return 1;
    }
}


void
freestorage()
{
    struct storage_descriptor sd;

    if (storage_accessed) {
	fprintf(stderr, "Internal error - attempt to free accessed storage.\n");
	fprintf(stderr, "(Encountered in file %s at line %d.)\n",
			__FILE__, __LINE__);
	quit(0, NULL);
    }
    if (storage_must_send == 0) {
	return;
    }
    storage_must_send = 0;
    if (api_exch_outcommand(EXCH_CMD_HEREIS) == -1) {
	kill_connection();
	return;
    }
    sd.length = storage_length;
    sd.location = storage_location;
    if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	kill_connection();
	return;
    }
    if (api_exch_outtype(EXCH_TYPE_BYTES, storage_length, (char *)storage)
							    == -1) {
	kill_connection();
	return;
    }
}


static int
getstorage(address, length, copyin)
long
    address;
int
    length,
    copyin;
{
    struct storage_descriptor sd;

    freestorage();
    if (storage_accessed) {
	fprintf(stderr,
		"Internal error - attempt to get while storage accessed.\n");
	fprintf(stderr, "(Encountered in file %s at line %d.)\n",
			__FILE__, __LINE__);
	quit(0, NULL);
    }
    storage_must_send = 0;
    if (api_exch_outcommand(EXCH_CMD_GIMME) == -1) {
	kill_connection();
	return -1;
    }
    storage_location = address;
    storage_length = length;
    if (copyin) {
	sd.location = (long)storage_location;
	sd.length = storage_length;
	if (api_exch_outtype(EXCH_TYPE_STORE_DESC,
					sizeof sd, (char *)&sd) == -1) {
	    kill_connection();
	    return -1;
	}
	if (api_exch_incommand(EXCH_CMD_HEREIS) == -1) {
	    fprintf(stderr, "Bad data from other side.\n");
	    fprintf(stderr, "(Encountered at %s, %d.)\n", __FILE__, __LINE__);
	    return -1;
	}
	if (nextstore() == -1) {
	    kill_connection();
	    return -1;
	}
    }
    return 0;
}

/*ARGSUSED*/
void
movetous(local, es, di, length)
char
    *local;
unsigned int
    es,
    di;
int
    length;
{
    long where = SEG_OFF_BACK(es, di);

    if (length > sizeof storage) {
	fprintf(stderr, "Internal API error - movetous() length too long.\n");
	fprintf(stderr, "(detected in file %s, line %d)\n", __FILE__, __LINE__);
	quit(0, NULL);
    } else if (length == 0) {
	return;
    }
    getstorage(where, length, 1);
    memcpy(local, (char *)(storage+((where-storage_location))), length);
    if (apitrace) {
	Dump('(', local, length);
    }
}

/*ARGSUSED*/
void
movetothem(es, di, local, length)
unsigned int
    es,
    di;
char
    *local;
int
    length;
{
    long where = SEG_OFF_BACK(es, di);

    if (length > sizeof storage) {
	fprintf(stderr, "Internal API error - movetothem() length too long.\n");
	fprintf(stderr, "(detected in file %s, line %d)\n", __FILE__, __LINE__);
	quit(0, NULL);
    } else if (length == 0) {
	return;
    }
    freestorage();
    memcpy((char *)storage, local, length);
    if (apitrace) {
	Dump(')', local, length);
    }
    storage_length = length;
    storage_location = where;
    storage_must_send = 1;
}


char *
access_api(location, length, copyin)
char *
    location;
int
    length,
    copyin;			/* Do we need to copy in initially? */
{
    if (storage_accessed) {
	fprintf(stderr, "Internal error - storage accessed twice\n");
	fprintf(stderr, "(Encountered in file %s, line %d.)\n",
				__FILE__, __LINE__);
	quit(0, NULL);
    } else if (length != 0) {
	freestorage();
	getstorage((long)location, length, copyin);
	storage_accessed = 1;
    }
    return (char *) storage;
}

/*ARGSUSED*/
void
unaccess_api(location, local, length, copyout)
char 	*location;
char	*local;
int	length;
int	copyout;
{
    if (storage_accessed == 0) {
	fprintf(stderr, "Internal error - unnecessary unaccess_api call.\n");
	fprintf(stderr, "(Encountered in file %s, line %d.)\n",
			__FILE__, __LINE__);
	quit(0, NULL);
    }
    storage_accessed = 0;
    storage_must_send = copyout;	/* if needs to go back */
}

/*
 * Accept a connection from an API client, aborting if the child dies.
 */

static int
doconnect()
{
    struct pollfd set[1];
    int i;

    sock = -1;
    set[0].fd = serversock;
    set[0].events = POLLIN;
    while (shell_active && (sock == -1)) {
	if ((i = poll(set, 1, INFTIM)) < 0) {
	    if (errno == EINTR) {
		continue;
	    } else {
		perror("in poll waiting for API connection");
		return -1;
	    }
	} else {
	    i = accept(serversock, (struct sockaddr *)0, (socklen_t *)0);
	    if (i == -1) {
		perror("accepting API connection");
		return -1;
	    }
	    sock = i;
	}
    }
    /* If the process has already exited, we may need to close */
    if ((shell_active == 0) && (sock != -1)) {

	(void) close(sock);
	sock = -1;
	setcommandmode();	/* In case child_died sneaked in */
    }
    return 0;
}

/*
 * shell_continue() actually runs the command, and looks for API
 * requests coming back in.
 *
 * We are called from the main loop in telnet.c.
 */

int
shell_continue()
{
    int i;

    switch (state) {
    case DEAD:
	pause();			/* Nothing to do */
	break;
    case UNCONNECTED:
	if (doconnect() == -1) {
	    kill_connection();
	    return -1;
	}
	/* At this point, it is possible that we've gone away */
	if (shell_active == 0) {
	    kill_connection();
	    return -1;
	}
	if (api_exch_init(sock, "server") == -1) {
	    return -1;
	}
	while (state == UNCONNECTED) {
	    if (api_exch_incommand(EXCH_CMD_ASSOCIATE) == -1) {
		kill_connection();
		return -1;
	    } else {
		switch (doassociate()) {
		case -1:
		    kill_connection();
		    return -1;
		case 0:
		    break;
		case 1:
		    state = CONNECTED;
		}
	    }
	}
	break;
    case CONNECTED:
	switch (i = api_exch_nextcommand()) {
	case EXCH_CMD_REQUEST:
	    if (api_exch_intype(EXCH_TYPE_REGS, sizeof inputRegs,
				    (char *)&inputRegs) == -1) {
		kill_connection();
	    } else if (api_exch_intype(EXCH_TYPE_SREGS, sizeof inputSregs,
				    (char *)&inputSregs) == -1) {
		kill_connection();
	    } else if (nextstore() == -1) {
		kill_connection();
	    } else {
		handle_api(&inputRegs, &inputSregs);
		freestorage();			/* Send any storage back */
		if (api_exch_outcommand(EXCH_CMD_REPLY) == -1) {
		    kill_connection();
		} else if (api_exch_outtype(EXCH_TYPE_REGS, sizeof inputRegs,
				    (char *)&inputRegs) == -1) {
		    kill_connection();
		} else if (api_exch_outtype(EXCH_TYPE_SREGS, sizeof inputSregs,
				    (char *)&inputSregs) == -1) {
		    kill_connection();
		}
		/* Done, and it all worked! */
	    }
	    break;
	case EXCH_CMD_DISASSOCIATE:
	    kill_connection();
	    break;
	default:
	    if (i != -1) {
		fprintf(stderr,
			"Looking for a REQUEST or DISASSOCIATE command\n");
		fprintf(stderr, "\treceived 0x%02x.\n", i);
	    }
	    kill_connection();
	    break;
	}
    }
    return shell_active;
}


static void
child_died(code)
    int code;
{
    int pid, status;

    while ((pid = waitpid((pid_t) -1, &status, WNOHANG)) > 0) {
	if (pid == shell_pid) {
	    char inputbuffer[100];

	    shell_active = 0;
	    if (sock != -1) {
		(void) close(sock);
		sock = -1;
	    }
	    printf("[Hit return to continue]");
	    fflush(stdout);
	    (void) fgets(inputbuffer, sizeof(inputbuffer), stdin);
	    setconnmode(0);
	    ConnectScreen();	/* Turn screen on (if need be) */
	    (void) close(serversock);
	    (void) unlink(keyname);
	}
    }
    signal(SIGCHLD, child_died);
}


/*
 * Called from telnet.c to fork a lower command.com.  We
 * use the sprint... routines so that we can pick up
 * interrupts generated by application programs.
 */


int
shell(argc,argv)
int	argc;
char	*argv[];
{
    socklen_t length;
    struct sockaddr_in server;
    char sockNAME[128];
    static char **whereAPI = 0;
    int fd;
    struct timeval tv;
    long ikey;

    /* First, create verification file. */
#if	defined(BSD4_4)
    if (keyname != NULL)
	free(keyname);
    keyname = strdup("/tmp/apiXXXXXX");
    fd = mkstemp(keyname);
#else
    do {
	if (keyname != NULL)
	    free(keyname);
	keyname = mktemp(strdup("/tmp/apiXXXXXX")); /* NetBSD: NOT USED */
	fd = open(keyname, O_RDWR|O_CREAT|O_EXCL, IREAD|IWRITE);
    } while ((fd == -1) && (errno == EEXIST));
#endif	/* defined(BSD4_4) */

    if (fd == -1) {
	perror("open");
	return 0;
    }

    /* Now, get seed for random */

    if (gettimeofday(&tv, (struct timezone *)0) == -1) {
	perror("gettimeofday");
	return 0;
    }
    srandom(tv.tv_usec);		/* seed random number generator */
    do {
	ikey = random();
    } while (ikey == 0);
    sprintf(key, "%lu\n", (unsigned long) ikey);
    if (write(fd, key, strlen(key)) != strlen(key)) {
	perror("write");
	return 0;
    }
    key[strlen(key)-1] = 0;		/* Get rid of newline */

    if (close(fd) == -1) {
	perror("close");
	return 0;
    }

    /* Next, create the socket which will be connected to */
    serversock = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock < 0) {
	perror("opening API socket");
	return 0;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 0;
    if (bind(serversock, (struct sockaddr *)&server, sizeof server) < 0) {
	perror("binding API socket");
	return 0;
    }
    length = sizeof server;
    if (getsockname(serversock, (struct sockaddr *)&server, &length) < 0) {
	perror("getting API socket name");
	(void) close(serversock);
    }
    listen(serversock, 1);
    /* Get name to advertise in address list */
    strcpy(sockNAME, "API3270=");
    gethostname(sockNAME+strlen(sockNAME), sizeof sockNAME-strlen(sockNAME));
    sockNAME[sizeof(sockNAME) - 1] = '\0';
    if (strlen(sockNAME) > (sizeof sockNAME-(10+strlen(keyname)))) {
	fprintf(stderr, "Local hostname too large; using 'localhost'.\n");
	strcpy(sockNAME, "localhost");
    }
    sprintf(sockNAME+strlen(sockNAME), ":%u", ntohs(server.sin_port));
    sprintf(sockNAME+strlen(sockNAME), ":%s", keyname);

    if (whereAPI == 0) {
	char **ptr, **nextenv;
	extern char **environ;

	ptr = environ;
	nextenv = ourENVlist;
	while (*ptr) {
	    if (nextenv >= &ourENVlist[highestof(ourENVlist)-1]) {
		fprintf(stderr, "Too many environmental variables\n");
		break;
	    }
	    *nextenv++ = *ptr++;
	}
	whereAPI = nextenv++;
	*nextenv++ = 0;
	environ = ourENVlist;		/* New environment */
    }
    *whereAPI = sockNAME;

    child_died(0);			/* Start up signal handler */
    shell_active = 1;			/* We are running down below */
    if ((shell_pid = vfork()) != 0) {
	if (shell_pid == -1) {
	    perror("vfork");
	    (void) close(serversock);
	} else {
	    state = UNCONNECTED;
	}
    } else {				/* New process */
	int i;

	for (i = 3; i < 30; i++) {
	    (void) close(i);
	}
	if (argc == 1) {		/* Just get a shell */
	    char *cmdname;

	    cmdname = getenv("SHELL");
	    execlp(cmdname, cmdname, NULL);
	    perror("Exec'ing new shell");
	    _exit(1);
	} else {
	    execvp(argv[1], &argv[1]);
	    perror("Exec'ing command");
	    _exit(1);
	}
	/*NOTREACHED*/
    }
    return shell_active;		/* Go back to main loop */
}