/* $Id: Grammar.c,v 1.17 1999/02/12 08:58:22 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Grammar.h"
#include "Kernel.h"
#include "Production.h"
#include "Terminal.h"

struct Grammar_t
{
    /* The number of productions */
    int production_count;

    /* The number of non-terminals in the grammar */
    int nonterminal_count;

    /* The grammar's non-terminals */
    Nonterminal *nonterminals;

    /* The number of terminals in the grammar */
    int terminal_count;

    /* The grammar's terminals */
    Terminal *terminals;

    /* The Grammar's productions */
    Production *productions;

    /* An array which maps Nonterminal indices to a List of Productions */
    List *productionsByNonterminal;

    /* This table records which productions are spontaneously
     * generated by a given nonterminal.  If generates[3][4] is
     * non-zero, then there is a production of the form
     * <nonterminal-3> ::= <nonterminal-4> ... in the grammar */
    char **generates;

    /* The number of Kernels in the receiver */
    int kernel_count;

    /* An array containing the Kernels of each of the states in the grammar */
    Kernel *kernels;
};


/*
 *
 * Static functions
 *
 */


/* Mark the box that indicates that non-terminal x generates non-terminal x */
static void MarkGenerates(Grammar self, int x, int y)
{
    int i;

    /* If the box is already marked then we've no further work to do */
    if (self -> generates[x][y])
    {
	return;
    }

    /* Mark the box */
    self -> generates[x][y] = 1;

    /* Propagate the mark to any non-terminal which generates us */
    for (i = 0; i < self -> nonterminal_count; i++)
    {
	if ((self -> generates[i] != NULL) && (self -> generates[i][x] != 0))
	{
	    MarkGenerates(self, i, y);
	}
    }

    /* Propagate the marks of the non-terminal we just generated */
    if (self -> generates[y] != NULL)
    {
	for (i = 0; i < self -> nonterminal_count; i++)
	{
	    if (self -> generates[y][i] != 0)
	    {
		MarkGenerates(self, x, i);
	    }
	}
    }
}

/* Marks the first terminals of a given non-terminal in the table */
static void MarkFirst(Grammar self, Nonterminal nonterminal, char *table, char *tried)
{
    List list;
    int index;

    list = self -> productionsByNonterminal[Nonterminal_getIndex(nonterminal)];
    for (index = 0; index < List_size(list); index++)
    {
	Production production = List_get(list, index);

	if (! tried[Production_getIndex(production)])
	{
	    Component component;

	    tried[Production_getIndex(production)] = 1;
	    component = Production_getFirstComponent(production);

	    if (Component_isNonterminal(component))
	    {
		MarkFirst(self, (Nonterminal)component, table, tried);
	    }
	    else
	    {
		table[Terminal_getIndex((Terminal)component)] = 1;
	    }
	}
    }
}

/* Copies the contents of the productions list into the receiver's productions */
static void PopulateProductions(Production production, Grammar self, int *index)
{
    Component component = Production_getFirstComponent(production);
    int i = Production_getNonterminalIndex(production);
    self -> productions[(*index)++] = production;

    /* If this is the first time we've encountered this non-terminal,
     * then update our tables */
    if (self -> productionsByNonterminal[i] == NULL)
    {
	self -> productionsByNonterminal[i] = List_alloc();
	self -> generates[i] = (char *)calloc(self -> nonterminal_count, sizeof(char));
	self -> generates[i][i] = 1;
    }

    /* Append the production to the end of the list for the non-terminal */
    List_addLast(self -> productionsByNonterminal[i], production);

    /* Record a mark in the "generates" table for this non-terminal
     * and first Component of the production if that component is a
     * Nonterminal */
    if (Component_isNonterminal(component))
    {
	int j = Nonterminal_getIndex((Nonterminal)component);
	MarkGenerates(self, i, j);
    }
}


/* Match two integers */
static int Equals(int x, int y)
{
    return x == y;
}

/* Updates the goto table for the given production and offset */
static void UpdateGoto(Grammar self, List *table, Production production, int offset)
{
    Component component = Production_getComponent(production, offset);
    int pair;
    int index;

    /* Figure out what the index should be */
    if (Component_isNonterminal(component))
    {
	index = Nonterminal_getIndex((Nonterminal) component);
    }
    else
    {
	index = self -> nonterminal_count + Terminal_getIndex((Terminal) component);
    }

    /* Compute the encoded pair */
    pair = Grammar_encode(self, production, offset + 1);

    /* Make sure the table has a List at that index */
    if (table[index] == NULL)
    {
	table[index] = List_alloc();
    }
    /* Make sure the item isn't already in the List */
    else
    {
 	if (List_findFirstWith(table[index], (ListFindWithFunc)Equals, (void *)pair) != NULL)
	{
	    return;
	}
    }

    /* Append the encoded item to the list */
    List_addLast(table[index], (void *) pair);
}

/* Populates the Goto table with the Productions at offset 0 */
static void PopulateGotoTable(Production production, Grammar self, List *table)
{
    UpdateGoto(self, table, production, 0);
}

/* Construct the set of LR(0) kernels */
void ComputeLR0Kernels(Grammar self)
{
    List list = List_alloc();
    List queue = List_alloc();
    Kernel kernel = Kernel_alloc(self, self -> productions[0]);
    int count = self -> nonterminal_count + self -> terminal_count;
    int index;

    List_addLast(list, kernel);
    List_addLast(queue, kernel);

    /* Loop until we don't have anything left on the queue */
    while ((kernel = List_dequeue(queue)) != NULL)
    {
	Kernel *table;

	/* Get the goto table for the kernel */
	table = Kernel_getGotoTable(kernel);

	/* Walk through it to see if we've already got those Kernel items */
	for (index = 0; index < count; index++)
	{
	    Kernel k = table[index];
	    if (k != NULL)
	    {
		if (List_findFirstWith(list, (ListFindWithFunc)Kernel_equals, k) == NULL)
		{
		    List_addLast(list, k);
		    List_addLast(queue, k);
		}
	    }
	}
    }

    List_free(queue);

    /* Create an array in which to store the Kernels */
    self -> kernel_count = List_size(list);
    self -> kernels = (Kernel *) calloc(self -> kernel_count, sizeof(Kernel));
    index = 0;
    while (! List_isEmpty(list))
    {
	Kernel kernel = (Kernel) List_dequeue(list);
	self -> kernels[index] = kernel;
	Kernel_setIndex(kernel, index);

	index++;
    }

    printf("%d kernels\n", self -> kernel_count);
}


/* Propagate to follows information */
static void PropagateFollows(Grammar self, int *isDone)
{
    int index;
    *isDone = 1;

    for (index = 0; index < self -> kernel_count; index++)
    {
	Kernel_propagateFollows(self -> kernels[index], isDone);
    }
}


/*
 *
 * Exported functions
 *
 */

/* Allocates a new Grammar with the given Productions */
Grammar Grammar_alloc(
    List productions,
    int nonterminal_count,
    Nonterminal *nonterminals,
    int terminal_count,
    Terminal *terminals)
{
    Grammar self;
    int index = 0;

    /* Allocate space for a new Grammar */
    if ((self = (Grammar) malloc(sizeof(struct Grammar_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory!\n");
	exit(1);
    }

    /* Set some initial values */
    self -> production_count = List_size(productions);
    self -> nonterminal_count = nonterminal_count;
    self -> nonterminals = nonterminals;
    self -> terminal_count = terminal_count;
    self -> terminals = terminals;

    /* Copy the productions into the receiver */
    self -> productions = (Production *)calloc(self -> production_count, sizeof(Production));
    self -> productionsByNonterminal = (List *)calloc(nonterminal_count, sizeof(List));
    self -> generates = (char **)calloc(nonterminal_count, sizeof(char *));
    List_doWithWith(productions, PopulateProductions, self, &index);

    return self;
}

/* Frees the resources consumed by the receiver */
void Grammar_free(Grammar self)
{
    int index;

    for (index = 0; index < self -> production_count; index++)
    {
/*	Production_free(self -> productions[index]);*/
    }

/*    free(self);*/
}

/* Pretty-prints the receiver */
void Grammar_debug(Grammar self, FILE *out)
{
    int index;

    fprintf(out, "Grammar %p\n", self);
    for (index = 0; index < self -> nonterminal_count; index++)
    {
	fprintf(out, "  %d ", index);
	List_doWith(self -> productionsByNonterminal[index], Production_print, out);
	fprintf(out, "\n");
    }
}

/* Answers the number of nonterminals in the receiver */
int Grammar_nonterminalCount(Grammar self)
{
    return self -> nonterminal_count;
}

/* Answers the indexed non-terminal */
Nonterminal Grammar_getNonterminal(Grammar self, int index)
{
    return self -> nonterminals[index];
}

/* Answers the number of terminals in the receiver */
int Grammar_terminalCount(Grammar self)
{
    return self -> terminal_count;
}

/* Answers the indexed terminal */
Terminal Grammar_getTerminal(Grammar self, int index)
{
    return self -> terminals[index];
}

/* Answers the number of productions in the receiver */
int Grammar_productionCount(Grammar self)
{
    return self -> production_count;
}

/* Answers the indexed production */
Production Grammar_getProduction(Grammar self, int index)
{
    return self -> productions[index];
}



/* Encodes a Production and offset in a single integer that sorts nicely */
int Grammar_encode(Grammar self, Production production, int offset)
{
    int count = self -> production_count;
    return (Production_getIndex(production) + 1) - (count * (offset + 1));
}

/* Answers the Production and offset encoded in the integer */
int Grammar_decode(Grammar self, int number, Production *production_return)
{
    int count = self -> production_count;
    *production_return = self -> productions[count + number % count - 1];
    return - (number / count);
}


/* Computes the contribute of the encoded production/offset (and
 * derived productions) to the goto table */
void Grammar_computeGoto(Grammar self, List *table, int number)
{
    Production production;
    int offset = Grammar_decode(self, number, &production);

    /* If the kernel production has more components, then add them to the list */
    if (offset < Production_getCount(production))
    {
	Component component = Production_getComponent(production, offset);
	UpdateGoto(self, table, production, offset);

	/* If the Component is a nonterminal, then compute the Goto for
	 * all of the productions of all of the non-terminals in the
	 * generates table for this non-terminal */
	if (Component_isNonterminal(component))
	{
	    int index = Nonterminal_getIndex((Nonterminal) component);
	    int j;

	    for (j = 0; j < self -> nonterminal_count; j++)
	    {
		if (self -> generates[index][j] != 0)
		{
		    List list = self -> productionsByNonterminal[j];
		    List_doWithWith(list, PopulateGotoTable, self, table);
		}
	    }
	}
    }
}

/* Transforms a Kernel into an actual kernel */
Kernel Grammar_resolveKernel(Grammar self, Kernel kernel)
{
    int index;

    if (kernel == NULL)
    {
	return NULL;
    }

    /* If the kernel is the official one, then simply return it */
    index = Kernel_getIndex(kernel);
    if (index != 0)
    {
	return kernel;
    }

    /* Try to find it in the kernel table */
    for (index = 0; index < self -> kernel_count; index++)
    {
	if (Kernel_equals(kernel, self -> kernels[index]))
	{
	    Kernel_free(kernel);
	    return self -> kernels[index];
	}
    }

    /* If we get here, then something has gone wrong... */
    printf("uh oh...\n");
    Kernel_debug(kernel, stdout);
    exit(1);
}

/* Transforms a table of Kernels into the "actual" kernels of the receiver */
void Grammar_resolveKernels(Grammar self, Kernel *kernels)
{
    int count = self -> nonterminal_count + self -> terminal_count;
    int index;

    for (index = 0; index < count; index++)
    {
	kernels[index] = Grammar_resolveKernel(self, kernels[index]);
    }
}

/* Answers the index of the Kernel in the receiver */
int Grammar_kernelIndex(Grammar self, Kernel kernel)
{
    int index;

    /* Return failure if we're not initialized */
    if (self -> kernels == NULL)
    {
	return -1;
    }

    /* Linear search through the kernels to find a match */
    for (index = 0; index < self -> kernel_count; index++)
    {
	if (Kernel_equals(self -> kernels[index], kernel))
	{
	    return index;
	}
    }

    /* Not found is an error */
    return -1;
}

/* Answers the Kernel corresponding to the given index */
Kernel Grammar_getKernel(Grammar self, int index)
{
    return self -> kernels[index];
}

/* Answers the Productions which are derived from a pair */
List Grammar_getDerivedProductions(Grammar self, Nonterminal nonterminal)
{
    return self -> productionsByNonterminal[Nonterminal_getIndex(nonterminal)];
}

/* Marks the first terminals of a given non-terminal in the table */
void Grammar_markFirst(Grammar self, Nonterminal nonterminal, char *table)
{
    char *productions = (char *)alloca(self -> production_count * sizeof(char));
    MarkFirst(self, nonterminal, table, productions);
}



/* Computes the set of LALR(0) states */
void Grammar_getLALRStates(Grammar self)
{
    int index;
    int isDone = 0;

    /* Compute the LR0 kernels */
    ComputeLR0Kernels(self);

    /* Prepare the kernels for computation of follows information */
    for (index = 0; index < self -> kernel_count; index++)
    {
	Kernel_propagatePrepare(self -> kernels[index]);
    }

    /* Inject the EOF terminal into the root production */
    Kernel_addFollowsTerminal(
	self -> kernels[0],
	Grammar_encode(self, self -> productions[0], 0),
	self -> terminals[0]);

    /* Keep propagating the follows information until it stops changing */
    while (! isDone)
    {
	PropagateFollows(self, &isDone);
    }

    /* Print out the result */
    for (index = 0; index < self -> kernel_count; index++)
    {
	Kernel_debug(self -> kernels[index], stdout);
    }
}
