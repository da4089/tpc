/* $Id: Kernel.c,v 1.1 1999/02/08 18:28:06 phelps Exp $ */

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
    fprintf(out, "  count=%d\n", self -> count);
    for (index = 0; index < self -> count; index++)
    {
	offset = Grammar_decode(self -> grammar, pairs[index], &production);
	fputs("    %d: ", out);
	Production_printWithOffset(production, offset);
	fputc('\n', out);
    }
}
