/***********************************************************************
 * 
 * Ensure <n> processes are kept running, from list given on command
 * line.  Use to load up multi-CPU machines.
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

#include <sys/time.h>
#define uint32_t u_int32_t

//#define _TRACE_
#include "trc.h"

static uint32_t nprocs;
static pid_t    child;
static char    *prog_name;

void sig_usr1(int);
void sig_usr2(int);

/*********************************************************************
 * SIGUSR1/2 handlers : USR1 increments <nprocs>; USR2 decrements
 * <nprocs>
 **/

void
sig_usr1(int signum __attribute__((unused)))
{
    nprocs++;
}

void
sig_usr2(int signum __attribute__((unused)))
{
    if(--nprocs < 1) nprocs = 1;
}

/*********************************************************************
 */
int
main(int argc, char **argv)
{
    unsigned int     n=0;
    int              status;
    pid_t            ret;
    struct timeval   tv;
    struct sigaction sa;

    ENTER;

    prog_name = argv[0];
    if(argc < 3) 
    {
        fprintf(stdout, 
                "Usage: %s <n> <cmd 1> <cmd 2> <cmd 3> ...:\n"
                "Run <cmds> sequentially, keeping up to <n> active at once.\n"
                "No guarantees are made about termination order.\n",
                prog_name);
        exit(-1);
    }
    else
    {
        nprocs = atoi(argv[1]);
        argc -= 2; argv += 2; 
    }
    if(gettimeofday(&tv, NULL) < 0)
    {
        perror("gettimeofday");
        exit(-1);
    }

    /* install signal handlers for USR1/2 */
    sa.sa_handler = sig_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR1, &sa, NULL) != 0)
    {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sig_usr2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR2, &sa, NULL) != 0)
    {
        perror("sigaction");
        exit(-1);
    }

    /*****************************************************************
     * main loop : ensure that whilst we still have commands to run,
     * we keep <n> running.
     **/

reload:
    while((argc > 0) && (n < nprocs))
    {
        fflush(NULL);
        if((child = fork()) < 0) /* error  */
        {
            perror("fork");
            exit(-1);
        }
        else if(child == 0)      /* child  */
        {
            int  i=0, l;
            char stdout_path[256], stderr_path[256];
            char **c_argv, *c;

            /*********************************************************
             * child : construct argv[] for execvp()
             **/
            l = strlen(argv[0]);
            if((c_argv = (char**)malloc(l*sizeof(char*))) == NULL)
            {
                perror("malloc");
                exit(-1);
            }
            memset(c_argv, 0, l*sizeof(char*));

            c = argv[0];
            while(c <= argv[0]+l)
            {
                while(*c == ' ') c++;
                c_argv[i++] = c++;
                while(*c != ' ') c++;
                *c++ = '\0';
            }

            /*********************************************************
             * child : swizzle stdout & stderr
             **/
            sprintf(stdout_path, "%s.%lu.%lu.%d.stdout", 
                    prog_name, (unsigned long)tv.tv_sec, (unsigned long)tv.tv_usec, (int)getpid());
            sprintf(stderr_path, "%s.%lu.%lu.%d.stderr", 
                    prog_name, (unsigned long)tv.tv_sec, (unsigned long)tv.tv_usec, (int)getpid());

            TRC("stdout: '%s'\n", stdout_path);
            TRC("stderr: '%s'\n", stderr_path);

            if((freopen(stdout_path, "w", stdout) == NULL) ||
               (freopen(stderr_path, "w", stderr) == NULL))
            {
                fprintf(stderr, "error reopening stdout/stderr\n");
                exit(-1);
            }

            fprintf(stderr, "execvp: '%s'\n", c_argv[0]);
            while(i-- > 0)
            {
                fprintf(stderr, "[%d: '%s']", i, c_argv[i]);
            }
            fprintf(stderr, "\n");
            fflush(NULL); // XXX why doesn't fflush(stderr) DTRT here?

            /*********************************************************
             * child : do the exec
             **/
            execvp(c_argv[0], c_argv);
            perror(c_argv[0]);
	    
            exit(1);
        }
        else                     /* parent */
        {
            /*********************************************************
             * parent : keep track of number running and number still
             * to do
             **/
            n++; argc--; argv++;
            fprintf(stdout, 
                    "parent [%d] : forked [%d]; active=%d [max=%d]; left=%d\n",
                    (int)getpid(), (int)child, n, nprocs, argc);
        }
    }

    /*****************************************************************
     * main loop : wait for a child to complete
     **/

reap:
    fflush(NULL);
    if((ret = wait(&status)) < 0)
    {
        TRC("%s\n", strerror(errno));
        switch(errno)
        {
            case ECHILD:
                goto finish;
                break;
            case EINTR:
                fprintf(stdout, "SIG : max=%d [active=%d]\n", nprocs, n);
                goto reload;
                break;
            default:
                perror("wait");
                exit(-1);
        }
    }
    TRC("pid [%d] finished\n", (int)ret);

    n--;
    if(WIFEXITED(status))
    {
        fprintf(stdout,
                "child [%d] : terminated (status=%d); active=%d [max=%d] "
                "left=%d\n", 
                (int)ret, WEXITSTATUS(status), n, nprocs, 
                argc);
    }
    else if(WIFSIGNALED(status))
    {
        fprintf(stdout,  
                "process [%d] terminated abnormally (sig=%d); active=%d [max=%d] "
                "left=%d\n", 
                (int)ret, WTERMSIG(status), n, nprocs, 
                argc);
    }
    else if(WIFSTOPPED(status))
    {
        fprintf(stdout, 
                "process [%d] stopped (sig=%d); active=%d [max=%d] "
                "left=%d\n", 
                (int)ret, WSTOPSIG(status), n, nprocs, 
                argc);
    }

    /*****************************************************************
     * main loop : if still more to run, do so, otherwise reap
     * remaining children
     **/

    if(argc > 0)
    {
        goto reload;
    }
    else
    {
        goto reap;
    }

finish:
    RETURN 0;
}
