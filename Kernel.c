/* $Id: Kernel.c,v 1.2 1999/02/08 19:44:04 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Kernel.h"

/* The structure of a kernel */
struct Kernel_t
{
    /* The Kernel's Grammar */
    Grammar grammar;

    /* The number of pairs in the Kernel */
    int count;

    /* The Kernel's encoded Production/offset pairs */
    int pairs[1];
};


/*
 *
 * Static functions
 *
 */

/* Basic allocation and initialization for a Kernel */
Kernel Alloc(Grammar grammar, int count)
{
    Kernel self;

    /* Allocate some space for the receiver */
    if ((self = (Kernel) malloc(sizeof(struct Kernel_t) + count * sizeof(int))) == NULL)
    {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }

    /* Set up some initial values */
    self -> grammar = grammar;
    self -> count = count;

    /* IT'S UP TO THE CALLER TO INITIALIZE THE PAIRS */

    return self;
}


/* Copy the kernel items from a List to an actual Kernel */
static void PopulateKernel(int number, Kernel self, int *index)
{
    self -> pairs[(*index)++] = number;
}



/*
 *
 * Exported functions
 *
 */

/* Answers a new Kernel containing the given Production */
Kernel Kernel_alloc(Grammar grammar, Production production)
{
    Kernel self;

    /* Allocate some space for the receiver */
    self = Alloc(grammar, 1);
    self -> pairs[0] = Grammar_encode(grammar, production, 0);

    return self;
}

/* Frees the resources consumed by the receiver */
void Kernel_free(Kernel self)
{
    free(self);
}

/* Prints debugging information about the receiver */
void Kernel_debug(Kernel self, FILE *out)
{
    Production production;
    int index;
    int offset;

    fprintf(out, "Kernel %p\n", self);
    fprintf(out, "  Grammar=%p\n", self -> grammar);
    for (index = 0; index < self -> count; index++)
    {
	offset = Grammar_decode(self -> grammar, self -> pairs[index], &production);
	fprintf(out, "  %d: ", index);
	Production_printWithOffset(production, out, offset);
	fputc('\n', out);
    }

    fputc('\n', out);
}

/* Answers the receiver's GotoTable */
Kernel *Kernel_getGotoTable(Kernel self)
{
    List *table;
    int index;
    int count = Grammar_nonterminalCount(self -> grammar) +
	Grammar_terminalCount(self -> grammar);
    Kernel *result;

    /* Make space in a table */
    table = (List *)calloc(count, sizeof(List));

    /* Pass the table off to the Grammar with each of our pairs */
    for (index = 0; index < self -> count; index++)
    {
	Grammar_computeGoto(self -> grammar, table, self -> pairs[index]);
    }

    /* Construct Kernels out of each of the Lists */
    result = (Kernel *)calloc(count, sizeof(Kernel));
    for (index = 0; index < count; index++)
    {
	if (table[index] != NULL)
	{
	    int i = 0;

	    result[index] = Alloc(self -> grammar, List_size(table[index]));
	    List_doWithWith(table[index], PopulateKernel, result[index], &i);
	    List_free(table[index]);
	}
    }

    free(table);
    return result;
}
