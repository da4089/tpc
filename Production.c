/* $Id: Production.c,v 1.3 1999/02/08 18:29:24 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Production.h"

struct Production_t
{
    /* The receiver's index */
    int index;

    /* The receiver's Nonterminal (the thing on the left-hand side) */
    Nonterminal nonterminal;

    /* The number of components in the receiver */
    int component_count;

    /* The receiver's components (the stuff on the right-hand side) */
    Component *components;
};


/*
 *
 * Static functions
 *
 */

/* Constructs an array of components from the Linked list */
void PopulateComponents(Component component, Production self, int *index)
{
    self -> components[(*index)++] = component;
}


/*
 *
 * Exported functions
 *
 */

/* Answers a new Production */
Production Production_alloc(Nonterminal nonterminal, List components, int index)
{
    Production self;
    int i = 0;

    /* Allocates memory for the Production */
    if ((self = (Production) malloc(sizeof(struct Production_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory\n");
	exit(1);
    }

    self -> index = index;
    self -> nonterminal = nonterminal;
    self -> component_count = List_size(components);
    self -> components = (Component *) calloc(self -> component_count, sizeof(Component));
    List_doWithWith(components, PopulateComponents, self, &i);

    return self;
}

/* Frees the resources consumed by a Production */
void Production_free(Production self)
{
    /* Assume that all nonterminals and terminals are freed elsewhere */
    free(self -> components);
    free(self);
}

/* Pretty-prints the receiver */
void Production_print(Production self, FILE *out)
{
    Production_printWithOffset(self, out, -1);
}

/* Pretty-prints the receiver with a * after the nth element */
void Production_printWithOffset(Production self, FILE *out, int offset)
{
    int index;

    Nonterminal_print(self -> nonterminal, out);
    fputs("::= ", out);

    for (index = 0; index < self -> component_count; index++)
    {
	if (index == offset)
	{
	    fputs("* ", out);
	}

	Component_print(self -> components[index], out);
    }

    if (index == offset)
    {
	fputs("* ", out);
    }
}



/* Answers the receiver's index */
int Production_getIndex(Production self)
{
    return self -> index;
}

/* Answers the index of the receiver's Nonterminal */
int Production_getNonterminalIndex(Production self)
{
    return Nonterminal_getIndex(self -> nonterminal);
}

/* Answers the receiver's first Component */
Component Production_getFirstComponent(Production self)
{
    return self -> components[0];
}
