/* $Id: Kernel.c,v 1.7 1999/02/11 07:56:48 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Kernel.h"

/* The structure of a kernel */
struct Kernel_t
{
    /* The Kernel's Grammar */
    Grammar grammar;

    /* The Kernel's index */
    int index;

    /* The Kernel's goto table */
    Kernel *goto_table;

    /* The number of pairs in the Kernel */
    int count;

    /* The follows tables of the kernel items */
    int *follows;

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
    self -> index = 0;
    self -> count = count;

    /* IT'S UP TO THE CALLER TO INITIALIZE THE PAIRS */

    return self;
}

/* Make sure the pairs are sorted */
static int Compare(const void *i, const void *j)
{
    int *x = (int *)i;
    int *y = (int *)j;

    if (*x < *y)
    {
	return -1;
    }
    else if (*x > *y)
    {
	return 1;
    }
    else
    {
	return 0;
    }
}

/* Sorts the Kernel's pairs (easy sorting order!) */
static void Sort(Kernel self)
{
    /* Use qsort to sort the elements */
    qsort(self -> pairs, self -> count, sizeof(int), Compare);
}


/* Copy the kernel items from a List to an actual Kernel */
static void PopulateKernel(int number, Kernel self, int *index)
{
    self -> pairs[(*index)++] = number;
}


#if 0
/* Computes the closure of the receiver */
void ComputeClosure(Kernel self)
{
    /* Add a fake production for the kernel sets and a fake terminal 
     * for determining propagation */
    int production_count = Grammar_productionCount(self -> grammar);
    int terminal_count = Grammar_terminalCount(self -> grammar);
    char *table = (char *)calloc((production_count + 1) * (terminal_count + 1), sizeof(char));
    int index;

    /* Flag the propagation terminal on the fake production */
    table[(production_count + 1) * (terminal_count + 1) - 1] = 1;

    /* Go through the kernel items and use them to compute the closure */
    for (index = 0; index < self -> count; index++)
    {
	Production production;
	int offset;

	offset = Grammar_decode(self -> grammar, self -> pairs[index], &production);
	Grammar_computeClosure(
	    self -> grammar,
	    Production_getComponent(production, offset),
	    Production_getComponent(production, offset + 1),
	    table + production_count * (terminal_count + 1),
	    table);
    }

    /* Store the table away */
    self -> follows = table;

    /* Print out the table */
    for (index = 0; index <= production_count; index++)
    {
	int j;

	printf("\n%d: ", index);
	for (j = 0; j <= terminal_count; j++)
	{
	    printf("%d ", table[index * (terminal_count + 1) + j]);
	}
    }
    printf("\n");
}
#endif /* 0 */

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
    /* Free our follows table if we have one */
    if (self -> follows != NULL)
    {
	free(self -> follows);
    }

    free(self);
}

/* Prints debugging information about the receiver */
void Kernel_debug(Kernel self, FILE *out)
{
    Production production;
    int index;
    int offset;

    fprintf(out, "Kernel (%d) %p\n", self -> index, self);
    fprintf(out, "  Grammar=%p\n", self -> grammar);
    for (index = 0; index < self -> count; index++)
    {
	offset = Grammar_decode(self -> grammar, self -> pairs[index], &production);
	fprintf(out, "  %d: ", index);
	Production_printWithOffset(production, out, offset);
	fputc('\n', out);
    }

    if (self -> goto_table != NULL)
    {
	int non_count = Grammar_nonterminalCount(self -> grammar);
	int term_count = Grammar_terminalCount(self -> grammar);

	for (index = 0; index < non_count; index++)
	{
	    if (self -> goto_table[index] != 0)
	    {
		fputs("    ", out);
		Nonterminal_print(Grammar_getNonterminal(self -> grammar, index), out);
		fprintf(out, ": %d\n", Kernel_getIndex(self -> goto_table[index]));
	    }
	}

	for (index = 0; index < term_count; index++)
	{
	    if (self -> goto_table[index + non_count] != NULL)
	    {
		fputs("    ", out);
		Terminal_print(Grammar_getTerminal(self -> grammar, index), out);
		fprintf(out, ": %d\n", Kernel_getIndex(self -> goto_table[non_count + index]));
	    }
	}
    }

    fputc('\n', out);
}

/* Sets the receiver's index */
void Kernel_setIndex(Kernel self, int index)
{
    self -> index = index;
}

/* Answers the receiver's index */
int Kernel_getIndex(Kernel self)
{
    return self -> index;
}

/* Answers non-zero if the receiver equals the kernel */
int Kernel_equals(Kernel self, Kernel kernel)
{
    int index;

    /* Check the Grammar (!) */
    if (self -> grammar != kernel -> grammar)
    {
	return 0;
    }

    /* Check the number of pairs */
    if (self -> count != kernel -> count)
    {
	return 0;
    }

    /* Since the pairs are sorted we just compare them one at a time */
    for (index = 0; index < self -> count; index++)
    {
	if (self -> pairs[index] != kernel -> pairs[index])
	{
	    return 0;
	}
    }

    /* Nothing else to compare, so they must be equal */
    return 1;
}

/* Set the follow information for the 0th production to be EOF */
void Kernel_markEOF(Kernel self)
{
    self -> follows[0] = 1;
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
	    Kernel k = Alloc(self -> grammar, List_size(table[index]));
	    int i = 0;

	    result[index] = k;
	    List_doWithWith(table[index], PopulateKernel, k, &i);
	    Sort(k);
	    List_free(table[index]);
	}
    }

    free(table);

    /* Get the grammar to resolve those kernels into the actual kernels */
    self -> goto_table = result;
    return result;
}

/* Propagate follows information from this kernel to the ones it derives */
void Kernel_propagateFollows(Kernel self, int *isDone)
{
    int count = Grammar_nonterminalCount(self -> grammar) +
	Grammar_terminalCount(self -> grammar);
    Grammar_resolveKernels(self -> grammar, count, self -> goto_table);

    if (self -> follows == NULL)
    {
/*	ComputeClosure(self);*/
    }
}



