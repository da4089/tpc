/* $Id: Terminal.c,v 1.2 1999/02/08 16:31:24 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Terminal.h"
#include "Component.h"

struct Terminal_t
{
    /* The receiver's Component functions */
    ComponentFunctions functions;

    /* The terminal's name */
    char *name;

    /* The terminal's index */
    int index;
};


/*
 *
 * Static function definitions
 *
 */

/* Always returns false */
int False(Terminal self)
{
    return 0;
}

/* Pretty-prints the receiver */
void Print(Terminal self, FILE *out)
{
    fprintf(out, "%s ", self -> name);
}


/* The method table */
static ComponentFunctions functions =
{
    (PrintMethod) Print,
    (IsNonterminalMethod) False
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

    self -> functions = functions;
    self -> name = strdup(name);
    self -> index = index;
    return self;
}

/* Frees the resources consumed by the receiver */
void Terminal_free(Terminal self)
{
    free(self);
}
