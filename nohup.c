/***********************************************************************
 * 
 * Replacement for poxy GNU nohup script.
 *
 * Copyright (C) 2000 Richard Mortier <mort@cantab.net>.  All Rights
 * Reserved.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

//#define _TRACE_
#include "trc.h"

static char    *prog_name;
static u_int32_t niceness = 0;
static u_int32_t verbose  = 0;
static u_int32_t sigquit  = 0;
static u_int32_t errout   = 0;

void      usage(void);
u_int32_t getint(char*, unsigned int);

void
usage(void)
{
    printf("Usage: %s [-h] [-v] [-n <nice>] -- <cmd...>\n"
           "       -h        : print this help message\n"
           "       -v        : verbose\n"
           "       -e        : redirect stderr as well as stdout to nohup.out\n"
           "       -q        : ignore SIGQUIT as well (a la Solaris2.8 nohup)\n"
           "       -n <nice> : run <cmd> niced to <nice>\n"
           "\n"
           "Run <cmd> with stdout/stderr appended to nohup.out/nohup.err\n"
           "and ignoring SIGHUP\n",
           prog_name);
    exit(-1);
}

u_int32_t
getint(char *str, unsigned int base)
{
    u_int32_t l;
    char     *str2;

    ENTER;
    TRC("%s:%d\n", str, base);
    l = strtoul(str, &str2, base);
    TRC("%d\n", l);
    if(str == str2)
    {
        fprintf(stderr, "%s: not a number", str);
        exit(1);
    }
    RETURN (u_int32_t)l;
}

int
main(int argc, char **argv)
{
    struct sigaction sa;
    int              opt=0, i=0;

    ENTER;

    prog_name = argv[0];
    if(argc < 2) usage();

    while((opt = getopt(argc, argv, "+vhqen:")) != -1)
        switch(opt)
        {
            case 'v':
                verbose = 1;
                break;

            case 'n':
                niceness = getint(optarg, 10);
                break;

            case 'q':
                sigquit = 1;
                break;

            case 'e':
                errout = 1;
                break;

            case 'h':
            default:
                usage();
        }

    argv += optind; argc -= optind;
    if(verbose)
    {
        for(i=0; i<argc; i++)
        {
            fprintf(stderr, "[%d: '%s']", i, argv[i]);
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "niceness: %d\n", niceness);
    }

    /*****************************************************************
     * install signal handlers for SIGHUP and nice ourselves if req'd.
     **/

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;

    if(sigaction(SIGHUP, &sa, NULL) != 0)
    {
        perror("sigaction");
        exit(-1);
    }
    if(sigquit && (sigaction(SIGQUIT, &sa, NULL) != 0))
    {
        perror("sigaction");
        exit(-1);
    }
    if(nice(niceness) < 0)
    {
        perror("nice");
        exit(-1);
    }

    /*****************************************************************
     * swizzle stdout & stderr
     **/

    if(freopen("nohup.out", "a+", stdout) == NULL)
    {
        fprintf(stderr, "error reopening stdout\n");
        exit(-1);
    }
    if(freopen(errout?"nohup.out":"nohup.err", "a+", stderr) == NULL)
    {
        fprintf(stderr, "error reopening stderr\n");
        exit(-1);
    }

    if(verbose)
    {
        fprintf(stderr, "execvp: '%s'\n", argv[0]);
        while(argc-- > 0)
            fprintf(stderr, "[%d: '%s']", argc, argv[argc]);
        fprintf(stderr, "\n");
        fflush(NULL); 
    }

    /*****************************************************************
     * do the exec
     **/

    execvp(argv[0], argv);

    perror(argv[0]);
    RETURN 0;
}
