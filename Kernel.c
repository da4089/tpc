/* $Id: Kernel.c,v 1.19 1999/02/22 00:44:11 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char *propagates;

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
    self -> follows = NULL;
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

    /* If there is no such component, then bail */
    if (component == NULL)
    {
	return NULL;
    }

    /* Otherwise look it up */
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
    char *propagates,
    char *table);

/* Propagate follows info for the given production, offset and follows info */
static void PropagatePrepare(
    Kernel self,
    Production production,
    int offset,
    Terminal terminal,
    char *propagates,
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
	/* Kernel items are implicit and shouldn't be recorded */
	if (offset == 0)
	{
	    /* See if we've already done this production */
	    char *pointer = propagates + Production_getIndex(production);
	    if (*pointer != 0)
	    {
		return;
	    }

	    /* Nope -- mark it so that we don't do it again though */
	    *pointer = 1;
	}
    }
    /* Otherwise we've got a spontaneously generated terminal */
    else
    {
	/* Make sure we haven't already done this one */
	char *pointer = table + Terminal_getIndex(terminal) +
	    (Production_getIndex(production) * Grammar_terminalCount(self -> grammar));

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
	int count = Grammar_terminalCount(self -> grammar);
	char *firsts = (char *) alloca(count * sizeof(char));
	int index;

	/* Get the first-set of the nonterminal */
	memset(firsts, 0, count * sizeof(char));
	Grammar_markFirst(self -> grammar, (Nonterminal)after, firsts);

	/* Do each item in the set */
	for (index = 0; index < count; index++)
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
    char *propagates,
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

/* Propagate the follows information from the pair to its derived kernels */
void PropagateFollows(Kernel self, int index, Terminal terminal, int *isDone)
{
    int pair = self -> pairs[index];
    Production production;
    int offset;
    int destination;
    int count;
    char *pointer;
    int i;

    /* Look up our production and offset */
    offset = Grammar_decode(self -> grammar, pair, &production);

    /* Propagate to the kernel item's production's next position if one exists */
    if (offset < Production_getCount(production))
    {
	destination = Grammar_encode(self -> grammar, production, offset + 1);
	if (Kernel_addFollowsTerminal(
	    GetGotoKernel(self, production, offset),
	    destination,
	    terminal))
	{
	    *isDone = 0;
	}
    }

    /* Go through the propagates table and propagate to those items too */
    count = Grammar_productionCount(self -> grammar);
    pointer = self -> propagates + (index * count);
    for (i = 0; i < count; i++)
    {
	if (pointer[i] != 0)
	{
	    Production production = Grammar_getProduction(self -> grammar, i);
	    destination = Grammar_encode(self -> grammar, production, 1);
	    if (Kernel_addFollowsTerminal(GetGotoKernel(self, production, 0), destination, terminal))
	    {
		*isDone = 0;
	    }
	}
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


/* Answers the production in the receiver with the lowest index */
static int GetFirstProductionIndex(Kernel self)
{
    int index;
    int result = Grammar_productionCount(self -> grammar);

    /* Go through the kernel productions */
    for (index = 0; index < self -> count; index++)
    {
	Production production;

	Grammar_decode(self -> grammar, self -> pairs[index], &production);
	if (Production_getIndex(production) < result)
	{
	    result = Production_getIndex(production);
	}
    }

    return result;
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

    fprintf(out, "Kernel %d (%p)\n", self -> index, self);
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

    /* Construct the receiver's propagates table */
    length = self -> count * Grammar_productionCount(self -> grammar);
    self -> propagates = (char *)calloc(length, sizeof(char));

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
	char *propagates = self -> propagates + 
	    (index * Grammar_productionCount(self -> grammar));
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
    int count;
    char *pointer;
    int i;
    int j;

    /* Nothing to propagate yet! */
    if (self -> follows == NULL)
    {
	return;
    }

    count = Grammar_terminalCount(self -> grammar);
    pointer = self -> follows;

    /* Go through each kernel item */
    for (i = 0; i < self -> count; i++)
    {
	/* Go through each terminal symbol for that item */
	for (j = 0; j < count; j++)
	{
	    /* See if that terminal is in the follows set */
	    if (*(pointer++) != 0)
	    {
		Terminal terminal = Grammar_getTerminal(self -> grammar, j);
		PropagateFollows(self, i, terminal, isDone);
	    }
	}
    }
}


/* Adds a terminal to the receiver's follows set for the given kernel item */
int Kernel_addFollowsTerminal(Kernel self, int pair, Terminal terminal)
{
    int index;

    /* Make sure we have a follows set */
    if (self -> follows == NULL)
    {
	self -> follows = (char *)calloc(
	    self -> count * Grammar_terminalCount(self -> grammar), sizeof(char));
    }

    /* Get the index of the pair */
    for (index = 0; index < self -> count; index++)
    {
	if (self -> pairs[index] == pair)
	{
	    char *pointer;

	    pointer = self -> follows + 
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

/* Print the receiver's portion of the parse table */
void Kernel_printSRTableEntry(Kernel self, FILE *out)
{
    int nonterminal_count = Grammar_nonterminalCount(self -> grammar);
    int terminal_count = Grammar_terminalCount(self -> grammar);
    int *reductions;
    int index;

    /* Create a table in which to record the reductions */
    reductions = (int *)alloca(terminal_count * sizeof(int));
    memset(reductions, -1, terminal_count * sizeof(int));

    /* Populate the reductions table */
    for (index = 0; index < self -> count; index++)
    {
	Production production;
	int offset = Grammar_decode(self -> grammar, self -> pairs[index], &production);

	/* We reduce on the follow set if we're the end of the production */
	if (offset == Production_getCount(production))
	{
	    char *in = self -> follows + (index * terminal_count);
	    char *end = in + terminal_count;
	    int *out = reductions;
	    int production_index = Production_getIndex(production);

	    /* Indicate that we reduce on the production for inputs in its follows set */
	    while (in < end)
	    {
		/* Only worry about terminals in the follows set */
		if (*in != 0)
		{
		    /* Watch for reduce/reduce conflicts and default
		     * to the first one encountered */
		    if (*out != -1)
		    {
			int tid = in - (self -> follows + (index * terminal_count));
			fprintf(stderr, "*** Warning: reduce/reduce conflict on ");
			Terminal_print(Grammar_getTerminal(self -> grammar, tid), stderr);
			fprintf(stderr, "in kernel %d\n", self -> index);
			fprintf(stderr, "    [using first listed reduction]\n");
			Kernel_debug(self, stderr);
		    }
		    else
		    {
			*out = production_index;
		    }
		}

		in++;
		out++;
	    }
	}
    }

    fputs("    { ", out);

    /* Print the terminal transitions */
    for (index = 0; index < terminal_count; index++)
    {
	Kernel kernel = self -> goto_table[nonterminal_count + index];
	int shift = (kernel == NULL) ? 0 : Kernel_getIndex(kernel);
	int reduction = reductions[index];

	/* Print a comma separator */
	if (index != 0)
	{
	    fputs(", ", out);
	}


	/* See if there's a shift action for this terminal */
	if (shift != 0)
	{
	    /* Watch for shift/reduce conflicts and default to shift */
	    if (reduction != -1)
	    {
		int sindex;

		fprintf(stderr, "*** Warning: shift/reduce conflict on ");
		Terminal_print(Grammar_getTerminal(self -> grammar, index), stderr);
		fprintf(stderr, "in kernel %d\n", self -> index);

		/* Resolve the conflict according to the order of
		 * productions in the grammar */
		sindex = GetFirstProductionIndex(Grammar_getKernel(self -> grammar, shift));

		if (reduction < sindex)
		{
		    /* We can never have a shift/reduce conflict for
		     * the accept state since we can never shift
		     * EOF, so don't worry about reduction == 0 */
		    fprintf(out, "R(%d)", reduction);
		    fprintf(stderr, "    [choosing to reduce]\n");
		}
		else
		{
		    fprintf(out, "S(%d)", shift);
		    fprintf(stderr, "    [choosing to shift]\n");
		}

		Kernel_debug(self, stderr);
	    }
	    else
	    {
		/* Print out a shift */
		fprintf(out, "S(%d)", shift);
	    }
	}

	/* Check for an accept */
	else if (reduction == 0)
	{
	    /* Print out an accept */
	    fprintf(out, "ACC");
	}

	/* Otherwise see if there's a reduce action for this terminal */
	else if (reduction != -1)
	{
	    /* Print out a reduce */
	    fprintf(out, "R(%d)", reduction);
	}

	/* Otherwise it's an error to get that transition */
	else
	{
	    fputs("ERR", out);
	}
    }

    fputs(" },\n", out);
}

/* Print the receiver's portion of the parse table */
void Kernel_printGotoTableEntry(Kernel self, FILE *out)
{
    int index;

    fputs("    { ", out);

    /* Go through each nonterminal and look up its goto information */
    for (index = 0; index < Grammar_nonterminalCount(self -> grammar); index++)
    {
	Kernel kernel = self -> goto_table[index];
	int target = (kernel == NULL) ? 0 : Kernel_getIndex(kernel);

	if (index == 0)
	{
	    fprintf(out, "%d", target);
	}
	else
	{
	    fprintf(out, ", %d", target);
	}
    }

    fputs(" },\n", out);
}

