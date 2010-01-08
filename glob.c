/***********************************************************************
 * 
 * Implementation of (old standard) tool "glob" -- perform globbing
 * according to shell rules.  Uses GNU glob extensions.
 *
 * Copyright (C) 2000 Richard Mortier <mort@cantab.net>.  All Rights
 * Reserved.
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <glob.h>

//#define _TRACE_
#include "trc.h"

int
main(int argc, char **argv)
{
    unsigned int i;    
    glob_t globbuf;

    for(i=1; i < (unsigned)argc; i++)
    {
        TRC("%d : %s\n", i, argv[i]);
        glob(argv[i], 
             GLOB_NOSORT | GLOB_BRACE | GLOB_NOMAGIC | GLOB_TILDE | 
             ((i>1)?GLOB_APPEND:0), 
             NULL, &globbuf);
    }
    for(i=0; i < globbuf.gl_pathc; i++)
    {
        printf("%s ", globbuf.gl_pathv[i]);
    }
    printf("\n");

    globfree(&globbuf);
    return 0;
}
