/* $Id: Nonterminal.c,v 1.1 1999/02/08 13:04:53 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Nonterminal.h"

struct Nonterminal_t
{
    /* The nonterminal's name */
    char *name;

    /* The nonterminal's index */
    int index;
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
    fprintf(out, "<%s> (%d)", self -> name, self -> index);
}
