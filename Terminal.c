/* $Id: Terminal.c,v 1.1 1999/02/08 13:05:08 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Terminal.h"

struct Terminal_t
{
    /* The terminal's name */
    char *name;

    /* The terminal's index */
    int index;
};


/* Answers a new Terminal */
Terminal Terminal_alloc(char *name, int index)
{
    Terminal self;

    /* Allocates memory for the Terminal */
    if ((self = (Terminal) malloc(sizeof(struct Terminal_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory\n");
	exit(1);
    }

    self -> name = strdup(name);
    self -> index = index;
    return self;
}

/* Frees the resources consumed by the receiver */
void Terminal_free(Terminal self)
{
    free(self);
}

/* Pretty-prints the receiver */
void Terminal_print(Terminal self, FILE *out)
{
    fprintf(out, "%s (%d)", self -> name, self -> index);
}
