/* $Id: Nonterminal.c,v 1.2 1999/02/08 16:31:23 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Nonterminal.h"
#include "Component.h"

struct Nonterminal_t
{
    /* The receiver's Component functions */
    ComponentFunctions functions;

    /* The nonterminal's name */
    char *name;

    /* The nonterminal's index */
    int index;
};


/*
 *
 * Static function definitions
 *
 */

/* Always returns true */
int True(Nonterminal self)
{
    return 1;
}

/* The method table */
static ComponentFunctions functions =
{
    (PrintMethod) Nonterminal_print,
    (IsNonterminalMethod) True
};



/* Answers a new Nonterminal */
Nonterminal Nonterminal_alloc(char *name, int index)
{
    Nonterminal self;

    /* Allocate memory for the Nonterminal */
    if ((self = (Nonterminal) malloc(sizeof(struct Nonterminal_t))) == NULL)
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
void Nonterminal_free(Nonterminal self)
{
    free(self);
}


/* Pretty-prints the receiver */
void Nonterminal_print(Nonterminal self, FILE *out)
{
    fprintf(out, "<%s> ", self -> name);
}

/* Answers the receiver's index */
int Nonterminal_getIndex(Nonterminal self)
{
    return self -> index;
}
