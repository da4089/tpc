/* $Id: Production.c,v 1.2 1999/02/08 17:17:33 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Production.h"

struct Production_t
{
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
Production Production_alloc(Nonterminal nonterminal, List components)
{
    Production self;
    int index = 0;

    /* Allocates memory for the Production */
    if ((self = (Production) malloc(sizeof(struct Production_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory\n");
	exit(1);
    }

    self -> nonterminal = nonterminal;
    self -> component_count = List_size(components);
    self -> components = (Component *) calloc(self -> component_count, sizeof(Component));
    List_doWithWith(components, PopulateComponents, self, &index);

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
    int index;

    Nonterminal_print(self -> nonterminal, out);
    fprintf(out, "::= ");

    for (index = 0; index < self -> component_count; index++)
    {
	Component_print(self -> components[index], out);
    }
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
