/* $Id: Kernel.c,v 1.9 1999/02/12 06:06:22 phelps Exp $ */

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

    /* The propagates tables of the kernel items */
    int *propagates;

    /* The follows table */
    char *follows;

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
    self -> index = 0;
    self -> goto_table = NULL;
    self -> propagates = NULL;
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


/* Answers the Kernel that we would goto for the given production and offset */
static Kernel GetGotoKernel(Kernel self, Production production, int offset)
{
    int index;
    Component component = Production_getComponent(production, offset);
    if (Component_isNonterminal(component))
    {
	index = Nonterminal_getIndex((Nonterminal)component);
    }
    else
    {
	index = Terminal_getIndex((Terminal)component) + 
	    Grammar_nonterminalCount(self -> grammar);
    }

    return self -> goto_table[index];
}

/* Header */
static void PropagateDerived(
    Kernel self,
    Nonterminal nonterminal,
    Terminal terminal,
    List propagates,
    char *table);

/* Propagate follows info for the given production, offset and follows info */
static void PropagatePrepare(
    Kernel self,
    Production production,
    int offset,
    Terminal terminal,
    List propagates,
    char *table)
{
    int destination;
    Component component;
    Component after;

    /* If this is the end of the production, then nothing is propagated or generated */
    if (! (offset < Production_getCount(production)))
    {
	return;
    }

    /* Compute our shifted pair */
    destination = Grammar_encode(self -> grammar, production, offset + 1);

    /* Are we doing propagation stuff? */
    if (terminal == NULL)
    {
	/* See if we've already done this production */
	if (List_includes(propagates, (void *)destination))
	{
	    return;
	}

	List_addLast(propagates, (void *)destination);
    }
    /* Otherwise we've got a spontaneously generated terminal */
    else
    {
	/* Make sure we haven't already done this one */
	char *pointer = table + Terminal_getIndex(terminal) +
	    (Production_getIndex(production) * Grammar_productionCount(self -> grammar));

	if (*pointer != 0)
	{
	    return;
	}

	*pointer = 1;

	/* Append the spontaneously generated terminal to the follows set for the kernel */
	Kernel_addFollowsTerminal(GetGotoKernel(self, production, offset), destination, terminal);
    }

    /* If the next component is a Terminal, then we're done */
    component = Production_getComponent(production, offset);
    if (! Component_isNonterminal(component))
    {
	return;
    }

    /* Get the component after the next component */
    after = Production_getComponent(production, offset + 1);

    /* If there is no such component, then we propagate our terminal */
    if (after == NULL)
    {
	PropagateDerived(self, (Nonterminal) component, terminal, propagates, table);
	return;
    }

    /* If it is a Terminal, then it becomes our new terminal symbol */
    if (! Component_isNonterminal(after))
    {
	PropagateDerived(self, (Nonterminal) component, (Terminal)after, propagates, table);
	return;
    }

    /* Otherwise, repeate for each terminal in the first set of the nonterminal */
    else
    {
	int length = Grammar_terminalCount(self -> grammar) * sizeof(char);
	int index;
	char *firsts = (char *) alloca(length);

	/* Get the first-set of the nonterminal */
	memset(firsts, 0, length);
	Grammar_markFirst(self -> grammar, (Nonterminal)after, firsts);

	/* Do each item in the set */
	for (index = 0; index < length; index++)
	{
	    if (firsts[index] != 0)
	    {
		PropagateDerived(
		    self,
		    (Nonterminal) component,
		    Grammar_getTerminal(self -> grammar, index),
		    propagates,
		    table);
	    }
	}
    }
}

/* Propagates the follows information to the derived-set of the nonterminal */
static void PropagateDerived(
    Kernel self,
    Nonterminal nonterminal,
    Terminal terminal,
    List propagates,
    char *table)
{
    List list = Grammar_getDerivedProductions(self -> grammar, nonterminal);
    int count = List_size(list);
    int index;

    for (index = 0; index < count; index++)
    {
	PropagatePrepare(self, (Production)List_get(list, index), 0, terminal, propagates, table);
    }
}

/* Prints out the follows set for the indexed pair */
static void PrintFollowsSet(Kernel self, int index, FILE *out)
{
    int count = Grammar_terminalCount(self -> grammar);
    char *pointer = self -> follows + (index * count);
    int isFirst = 1;
    int i;

    for (i = 0; i < count; i++)
    {
	if (pointer[i] != 0)
	{
	    if (isFirst)
	    {
		fputs(", ", out);
		isFirst = 0;
	    }
	    else
	    {
		fputs("/ ", out);
	    }

	    Terminal_print(Grammar_getTerminal(self -> grammar, i), out);
	}
    }
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
    /* Free our propagates table if we have one */
    if (self -> propagates != NULL)
    {
	free(self -> propagates);
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

	/* Print follows information */
	if (self -> follows != NULL)
	{
	    PrintFollowsSet(self, index, out);
	}

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

/* Prepare to propagate the follows info */
void Kernel_propagatePrepare(Kernel self)
{
    int index;
    int length;
    char *table;

    /* Use the canonical version of the kernels in the goto table */
    Grammar_resolveKernels(self -> grammar, self -> goto_table);

    /* Construct a table */
    length = Grammar_productionCount(self -> grammar) * 
	Grammar_terminalCount(self -> grammar) *
	sizeof(char);
    table = (char *)alloca(length);

    /* For each Kernel item, determine what kernel/pair it propagates
     * its follow set to, and what follows items are spontaneously
     * generated (and put them in the initial follows set for those
     * kernel/pairs */
    for (index = 0; index < self -> count; index++)
    {
	List propagates = List_alloc();
	Production production;
	int offset;

	memset(table, 0, length);
	offset = Grammar_decode(self -> grammar, self -> pairs[index], &production);
	PropagatePrepare(self, production, offset, NULL, propagates, table);
    }
}

/* Propagate follows information from this kernel to the ones it derives */
void Kernel_propagateFollows(Kernel self, int *isDone)
{
}


/* Adds a terminal to the receiver's follows set for the given kernel item */
int Kernel_addFollowsTerminal(Kernel self, int pair, Terminal terminal)
{
    int index;

    /* Make sure we have a follows set */
    if (self -> follows == NULL)
    {
	self -> follows = calloc(
	    self -> count * Grammar_nonterminalCount(self -> grammar), sizeof(char));
    }

    /* Get the index of the pair */
    for (index = 0; index < self -> count; index++)
    {
	if (self -> pairs[index] == pair)
	{
	    char *pointer = self -> follows + 
		(index * Grammar_terminalCount(self -> grammar)) +
		Terminal_getIndex(terminal);

	    /* If the item wasn't previously set, then set it and return that we've changed */
	    if (*pointer == 0)
	    {
		*pointer = 1;
		return 1;
	    }

	    /* Otherwise don't change a thing */
	    return 0;
	}
    }

    fprintf(stderr, "*** BADNESS\n");
    exit(1);
}
