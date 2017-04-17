/*
 * Copyright (c) 1988, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	8.3 (Berkeley) 5/30/95";
#endif /* not lint */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <libtelnet/auth.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "ring.h"
#include "externs.h"
#include "defines.h"

#ifdef TLS
void tls_optarg(char *optarg);
#endif

#ifdef FWD_X
extern int fwdx_enable_flag;
#endif

/*
 * Initialize variables.
 */
    void
tninit()
{
    init_terminal();

    init_network();

    init_telnet();

    init_sys();

#if defined(TN3270)
    init_3270();
#endif
}

	void
usage()
{
	fprintf(stderr, "Usage: %s %s%s%s%s%s%s\n",
	    prompt,
#ifdef	AUTHENTICATION
	    "[-8] [-E] [-K] [-L] [-S tos] [-X atype] [-a] [-c] [-d] [-e char]",
	    "\n\t[-k realm] [-l user] [-f/-F] [-n tracefile] ",
#else
	    "[-8] [-E] [-L] [-S tos] [-a] [-c] [-d] [-e char] [-l user]",
	    "\n\t[-n tracefile]",
#endif
#if defined(TN3270) && defined(unix)
# ifdef AUTHENTICATION
	    "[-noasynch] [-noasynctty]\n\t[-noasyncnet] [-r] [-t transcom] ",
# else
	    "[-noasynch] [-noasynctty] [-noasyncnet] [-r]\n\t[-t transcom]",
# endif
#else
	    "[-r] ",
#endif
#ifdef KRB5
	    "[-A] ",
#endif /* KRB5 */
#ifdef FWD_X
	    "[-D]\n\t",
#else /* FWD_X */
	    "\n\t",
#endif /* FWD_X */
#ifdef TLS
	    "[-T cert=FILE] [-T key=FILE] [-T crlfile=FILE] [-T crldir=DIR]\n\t[-T cipher=STRING] [-T CApath=DIR] [-T CAfile=FILE]\n\t",
#else /* TLS */
	    "",
#endif /* TLS */
#ifdef	ENCRYPTION
	    "[-x] [host-name [port]]"
#else	/* ENCRYPTION */
	    "[host-name [port]]"
#endif	/* ENCRYPTION */
	);
	/* TODO: add help for FWD_X, TLS */
	exit(1);
}

/*
 * main.  Parse arguments, invoke the protocol or command parser.
 */

/* see forward.c -- indicate that we're in telnet, not telnetd. */
char *line = 0;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	int ch;
	char *user;
#ifndef strrchr
	char *strrchr();
#endif
#ifdef	FORWARD
	extern int forward_flags;
#endif	/* FORWARD */
#ifdef ENCRYPTION
        extern int auth_enable_encrypt;
#endif /* ENCRYPTION */
#ifdef KRB5
	extern int krb5_use_addresses;
#endif /* KRB5 */

	tninit();		/* Clear out things */
#if	defined(CRAY) && !defined(__STDC__)
	_setlist_init();	/* Work around compiler bug */
#endif

	TerminalSaveState();

	if (prompt = strrchr(argv[0], '/'))
		++prompt;
	else
		prompt = argv[0];

	user = NULL;

	rlogin = (strncmp(prompt, "rlog", 4) == 0) ? '~' : _POSIX_VDISABLE;
	autologin = -1;

	while ((ch = getopt(argc, argv, "8EKLS:X:acde:fFk:l:n:rt:xT:DA")) != EOF) {
		switch(ch) {
		case '8':
			eight = 3;	/* binary output and input */
			break;
		case 'E':
			rlogin = escape = _POSIX_VDISABLE;
			break;
		case 'K':
#ifdef	AUTHENTICATION
			autologin = 0;
#endif
			break;
		case 'L':
			eight |= 2;	/* binary output only */
			break;
		case 'S':
		    {
#ifdef	HAS_GETTOS
			extern int tos;

			if ((tos = parsetos(optarg, "tcp")) < 0)
				fprintf(stderr, "%s%s%s%s\n",
					prompt, ": Bad TOS argument '",
					optarg,
					"; will try to use default TOS");
#else
			fprintf(stderr,
			   "%s: Warning: -S ignored, no parsetos() support.\n",
								prompt);
#endif
		    }
			break;
		case 'X':
#ifdef	AUTHENTICATION
			auth_disable_name(optarg);
#endif
			break;
		case 'a':
			autologin = 1;
			break;
		case 'c':
			skiprc = 1;
			break;
		case 'd':
			debug = 1;
			break;
		case 'e':
			set_escape_char(optarg);
			break;
		case 'f':
#if defined(AUTHENTICATION) && defined(KRB5) && defined(FORWARD)
			if (forward_flags & OPTS_FORWARD_CREDS) {
			    fprintf(stderr,
				    "%s: Only one of -f and -F allowed.\n",
				    prompt);
			    usage();
			}
			forward_flags |= OPTS_FORWARD_CREDS;
#else
			fprintf(stderr,
			 "%s: Warning: -f ignored, no Kerberos V5 support.\n",
				prompt);
#endif
			break;
		case 'F':
#if defined(AUTHENTICATION) && defined(KRB5) && defined(FORWARD)
			if (forward_flags & OPTS_FORWARD_CREDS) {
			    fprintf(stderr,
				    "%s: Only one of -f and -F allowed.\n",
				    prompt);
			    usage();
			}
			forward_flags |= OPTS_FORWARD_CREDS;
			forward_flags |= OPTS_FORWARDABLE_CREDS;
#else
			fprintf(stderr,
			 "%s: Warning: -F ignored, no Kerberos V5 support.\n",
				prompt);
#endif
			break;
		case 'k':
#if defined(AUTHENTICATION) && defined(KRB4)
		    {
			extern char *dest_realm, dst_realm_buf[];
			extern int dst_realm_sz;
			dest_realm = dst_realm_buf;
			(void)strncpy(dest_realm, optarg, dst_realm_sz);
		    }
#endif
#if defined(AUTHENTICATION) && defined(KRB5)
		    {
			extern char *telnet_krb5_realm;

			telnet_krb5_realm = optarg;
			break;
		    }
#endif
#if !defined(AUTHENTICATION) || (!defined(KRB4) && !defined(KRB5))
			fprintf(stderr,
			   "%s: Warning: -k ignored, no Kerberos V4 support.\n",
								prompt);
#endif
			break;
		case 'l':
			autologin = 1;
			user = optarg;
			break;
		case 'n':
#if defined(TN3270) && defined(unix)
			/* distinguish between "-n oasynch" and "-noasynch" */
			if (argv[optind - 1][0] == '-' && argv[optind - 1][1]
			    == 'n' && argv[optind - 1][2] == 'o') {
				if (!strcmp(optarg, "oasynch")) {
					noasynchtty = 1;
					noasynchnet = 1;
				} else if (!strcmp(optarg, "oasynchtty"))
					noasynchtty = 1;
				else if (!strcmp(optarg, "oasynchnet"))
					noasynchnet = 1;
			} else
#endif	/* defined(TN3270) && defined(unix) */
				SetNetTrace(optarg);
			break;
		case 'r':
			rlogin = '~';
			break;
		case 't':
#if defined(TN3270) && defined(unix)
			transcom = tline;
			(void)strncpy(transcom, optarg, sizeof(tline));
			transcom[sizeof(tline) - 1] = 0;
#else
			fprintf(stderr,
			   "%s: Warning: -t ignored, no TN3270 support.\n",
								prompt);
#endif
			break;
		case 'x':
#ifdef	ENCRYPTION
			encrypt_auto(1);
			decrypt_auto(1);
                        wantencryption = 1;
                        autologin = 1;
                        auth_enable_encrypt = 1;
#else	/* ENCRYPTION */
			fprintf(stderr,
			    "%s: Warning: -x ignored, no ENCRYPT support.\n",
								prompt);
#endif	/* ENCRYPTION */
#ifdef FWD_X
			fwdx_enable_flag = 0;
#endif  /* FWD_X */
			break;

#ifdef TLS
		case 'T':
			tls_optarg(optarg);
			break;
#endif  /* TLS */
#ifdef FWD_X
	        case 'D':
			fwdx_enable_flag = 0;
			break;
#endif  /* FWD_X */
#ifdef KRB5
	        case 'A':
			krb5_use_addresses = 0;
			break;
#endif	/* KRB5 */

		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
	if (autologin == -1)
#ifdef TJW
		autologin = 1;
#else
		autologin = (rlogin == _POSIX_VDISABLE) ? 0 : 1;
#endif

	argc -= optind;
	argv += optind;

	if (argc) {
		char *args[7], **argp = args;

		if (argc > 2)
			usage();
		*argp++ = prompt;
		if (user) {
			*argp++ = "-l";
			*argp++ = user;
		}
		*argp++ = argv[0];		/* host */
		if (argc > 1)
			*argp++ = argv[1];	/* port */
		*argp = 0;

		if (setjmp(toplevel) != 0)
			Exit(0);
		if (tn(argp - args, args) == 1)
			return (0);
		else
			return (1);
	}
	(void)setjmp(toplevel);
	for (;;) {
#ifdef TN3270
		if (shell_active)
			shell_continue();
		else
#endif
			command(1, 0, 0);
	}
}