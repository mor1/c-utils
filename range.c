/***********************************************************************
 * 
 * Implementation of python range() builtin, handy for shell scripts.
 *
 * Copyright (C) 2000 Richard Mortier <mort@cantab.net>.  All Rights
 * Reserved.
 *
 **/

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
    if((argc != 4) ||
       ((atoi(argv[1]) >= atoi(argv[2])) && (atoi(argv[3]) > 0)) ||
       ((atoi(argv[1]) <= atoi(argv[2])) && (atoi(argv[3]) < 0)))
    {
        fprintf(stderr, 
                "Syntax: %s <min> <max> <step> to generate [<min>, <max>)\n",
                argv[0]);
        exit(1);
    }
    else
    {
        int i;
        if(atoi(argv[3]) > 0)
        {
            for( i=atoi(argv[1]); i < atoi(argv[2]); i+=atoi(argv[3]) )
            {
                printf("%d ", i);
            }
        }
        else
        {
            for( i=atoi(argv[1]); i > atoi(argv[2]); i+=atoi(argv[3]) )
            {
                printf("%d ", i);
            }
        }
    }
	
    exit(0);
}
