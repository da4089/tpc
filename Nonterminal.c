/* $Id: Nonterminal.c,v 1.3 1999/02/11 01:46:12 phelps Exp $ */

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
static int True(Nonterminal self)
{
    return 1;
}

/* Marks the terminals which can appear as the first element of the receiver */
static void MarkFirst(Nonterminal self, Grammar grammar, char *table)
{
    printf("*** MarkFirst(Nonterminal, Grammar, char *) not implemented!\n");
    exit(1);
}

/* The method table */
static ComponentFunctions functions =
{
    (PrintMethod) Nonterminal_print,
    (IsNonterminalMethod) True,
    (MarkFirstMethod) MarkFirst
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
