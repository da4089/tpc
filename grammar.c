/* -*- mode: c; c-file-style: "elvin" -*- */
/***********************************************************************

  Copyright (C) 1999-2006 by Mantara Software (ABN 17 105 665 594).
  All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.

   * Neither the name of the Mantara Software nor the names
     of its contributors may be used to endorse or promote
     products derived from this software without specific prior
     written permission. 

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/

#ifndef lint
static const char cvsid[] = "$Id: grammar.c,v 1.30 2006/10/17 14:49:10 phelps Exp $";
#endif /* lint */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "component.h"
#include "production.h"
#include "grammar.h"

/* The kernel data structure */
typedef struct kernel *kernel_t;
struct kernel
{
    /* The number of (production, offset) pairs in the kernel */
    int count;

    /* The encoded (production, offset) pairs */
    int *pairs;

    /* The kernel's go-to table */
    int *goto_table;

    /* The kernel's propagates table */
    char **propagates_table;

    /* The kernel's follows tables */
    char **follows_table;
};

/* Allocates and initializes a new kernel_t */
static kernel_t
kernel_alloc(int count, int *pairs, int terminal_count)
{
    kernel_t self;
    int index;

    /* Allocate some space for the new kernel_t */
    if ((self = (kernel_t)malloc(sizeof(struct kernel))) == NULL) {
        return NULL;
    }

    /* Initialize its contents to sane values */
    self->count = count;
    self->pairs = pairs;
    self->goto_table = NULL;
    self->follows_table = NULL;

    /* Allocate some room for the propagates table */
    self->propagates_table = (char **)calloc(count, sizeof(char *));
    if (self->propagates_table == NULL) {
        free(self->pairs);
        free(self);
        return NULL;
    }

    /* Allocate some room for the follows tables */
    self->follows_table = (char **)calloc(count, sizeof(char *));
    if (self->follows_table == NULL) {
        free(self->propagates_table);
        free(self->pairs);
        free(self);
        return NULL;
    }

    /* Fill in the follows table entries */
    for (index = 0; index < count; index++) {
        self->follows_table[index] =
            (char *)calloc(terminal_count, sizeof(char));
        if (self->follows_table[index] == NULL) {
            abort();
        }
    }

    return self;
}

#if 0
/* Releases the resources consumed by the receiver */
static void
kernel_free(kernel_t self)
{
    if (self->pairs != NULL) {
        free(self->pairs);
    }

    if (self->goto_table != NULL) {
        free(self->goto_table);
    }

    free(self);
}
#endif /* 0 */

/* Returns nonzero if the kernel matches the set of encoded
 * (production, offset) pairs */
static int
kernel_matches(kernel_t self, int count, int *pairs)
{
    /* See if the right number of pairs are there */
    if (self->count != count) {
        return 0;
    }

    /* Compare the arrays */
    return memcmp(self->pairs, pairs, count * sizeof(int)) == 0;
}

/* Adds the terminal to the follows set for the given kernel item */
static int
kernel_set_follows(kernel_t self, int code, int terminal_index)
{
    int i;

    /* Figure out which kernel item we're dealing with */
    for (i = 0; i < self->count; i++) {
        /* Got a match? */
        if (self->pairs[i] == code) {
            int old_value = self->follows_table[i][terminal_index];
            self->follows_table[i][terminal_index] = 1;
            return old_value;
        }
    }

    /* Couldn't find the entry */
    abort();
}


/* The organization of the grammar */
struct grammar
{
    /* The number of productions */
    int production_count;

    /* The productions */
    production_t *productions;

    /* The number of terminal symbols */
    int terminal_count;

    /* The terminals */
    component_t *terminals;

    /* The number of nonterminal symbols */
    int nonterminal_count;

    /* The nonterminals */
    component_t *nonterminals;

    /* A table of productions listed by their left-hand-side */
    production_t **productions_by_nonterminal;

    /* The generates table.  Once initialized, generates[generator][generated]
     * will be nonzero if the nonterminal[generator] can generate
     * nonterminal[generated]. */
    char **generates;

    /* The number of kernels in the receiver */
    int kernel_count;

    /* The kernels */
    kernel_t *kernels;
};



/* Constructs a table mapping nonterminal indices to a null-terminated
 * array of productions.  Returns NULL if something goes wrong. */
static production_t **
compute_productions_by_nonterminal(int nonterminal_count,
                                   int production_count,
                                   production_t *productions)
{
    production_t **table;
    production_t *productions_end = productions + production_count;
    production_t *production;

    /* Allocate memory for a new table */
    table = (production_t **)calloc(nonterminal_count, sizeof(production_t *));
    if (table == NULL) {
        return NULL;
    }

    /* Popluate it */
    for (production = productions;
         production < productions_end;
         production++) {
        int index = production_get_nonterminal_index(*production);
        production_t *array;
        int count = 0;

        /* If there is no entry for the nonterminal then create one */
        if ((array = table[index]) == NULL) {
            array = (production_t *)malloc(2 * sizeof(production_t));
            if (array == NULL) {
                /* FIX THIS: memory leak */
                free(table);
                return NULL;
            }

            count = 0;
        } else {
            /* Count the number of entries so far */
            for (count = 0; array[count] != NULL; count++);

            /* Enlarge the array */
            array = (production_t *)realloc(array, (count + 2) *
                                            sizeof(production_t));
            if (array == NULL) {
                /* FIX THIS: memory leak */
                free(table);
                return NULL;
            }
        }

        array[count] = *production;
        array[count + 1] = NULL;
        table[index] = array;
    }

    return table;
}

/* Verifies that each nonterminal has at least on production rule */
static int
verify_productions_by_nonterminal(grammar_t self)
{
    int index, line;
    int result = 0;

    /* Do a sanity check -- there should be no empty entries */
    for (index = 0; index < self->nonterminal_count; index++) {
        if (self->productions_by_nonterminal[index] == NULL) {
            char *filename;
            line = component_get_origin(self->nonterminals[index], &filename);

            fprintf(stderr, "%s:%d: no rule to generate ",
                    filename ? filename : "[stdin]", line);
            component_print(self->nonterminals[index], stderr);
            fprintf(stderr, "\n");
            result = -1;
        }
    }

    return result;
}

/* Indicate that the nonterminal `generator' spontaneously generates
 * the nonterminal `generated' in the grammar */
static void
mark_generates(grammar_t self, int generator, int generated)
{
    int index;

    /* Make sure the table entry exists */
    if (self->generates[generator] == NULL) {
        self->generates[generator] =
            (char *)calloc(self->nonterminal_count, sizeof(char));
    } else if (self->generates[generator][generated] != 0) {
        return;
    }

    /* Set the flag */
    self->generates[generator][generated] = 1;

    /* Propagate the flag to any nonterminal which generates us */
    for (index = 0; index < self->nonterminal_count; index++) {
        if (self->generates[index] != NULL &&
            self->generates[index][generator] != 0) {
            mark_generates(self, index, generated);
        }
    }

    /* Propagate the flags to the nonterminal we've just generated */
    if (self->generates[generated] != NULL) {
        for (index = 0; index < self->nonterminal_count; index++) {
            if (self->generates[generated][index] != 0) {
                mark_generates(self, generator, index);
            }
        }
    }
}

/* Constructs the `generates' table */
static void
compute_generates(grammar_t self)
{
    int index;

    /* Create the `generates' table */
    self->generates = (char **)calloc(self->nonterminal_count, sizeof(char *));

    /* Go through each of the productions and figure out which things
     * generate which */
    for (index = 0; index < self->production_count; index++) {
        production_t production = self->productions[index];
        int nonterminal = production_get_nonterminal_index(production);
        component_t component = production_get_component(production, 0);

        if (component_is_nonterminal(component)) {
            mark_generates(self, nonterminal, component_get_index(component));
        }
    }
}

/* Encode a production number and offset in a single integer. */
static int
encode(grammar_t self, int index, int offset)
{
    return self->production_count * (offset + 1) - index - 1;
}

/* Decodes an integer into an production number and offset */
static int
decode(grammar_t self, int code, int *index_out)
{
    *index_out = self->production_count - (code % self->production_count) - 1;
    return code / self->production_count;
}


/* Locates or creates a kernel for the given goto pairs and returns
 * its index */
int
intern_kernel(grammar_t self, int count, int *pairs)
{
    int index;
    kernel_t kernel;

    /* If there are no pairs then don't do anything special */
    if (count == 0) {
        return -1;
    }

    /* See if we've already got a matching kernel */
    for (index = 0; index < self->kernel_count; index++) {
        if (kernel_matches(self->kernels[index], count, pairs)) {
            free(pairs);
            return index;
        }
    }

    /* Not there, so create a new kernel */
    kernel = kernel_alloc(count, pairs, self->terminal_count);

    /* And put it in the table */
    self->kernels = (kernel_t *)realloc(self->kernels,
                                        (self->kernel_count + 1) *
                                        sizeof(kernel_t));
    self->kernels[self->kernel_count] = kernel;

    /* Return its index */
    return self->kernel_count++;
}

/* Adds an entry to the pairs table */
static void
add_pairs_entry(int *counts, int **table, int index, int pair)
{
    int i;

    /* Figure out where to add the entry (common case is at the end so
     * we count backwards) */
    for (i = counts[index]; i > 0; i--) {
        /* If it's already there then bail out now */
        if (pair == table[index][i - 1]) {
            return;
        }

        /* Is this where we should insert? */
        if (pair < table[index][i - 1]) {
            break;
        }
    }

    /* Expand the table */
    table[index] = (int *)realloc(table[index],
                                  (counts[index] + 1) * sizeof(int));

    /* See if we need to make a hole */
    if (i != counts[index]) {
        memmove(table[index] + i + 1, table[index] + i,
                sizeof(int) * (counts[index] - i));
    }

    /* Insert the pair */
    table[index][i] = pair;
    counts[index]++;
}

/* Returns the index to use for the given component */
static int
component_index(grammar_t self, component_t component)
{
    if (component_is_nonterminal(component)) {
        return component_get_index(component);
    }

    return self->nonterminal_count + component_get_index(component);
}


/* Computes the contribution of an item in the kernel to the pairs table */
static void
compute_pairs_for_kernel_item(grammar_t self,
                              production_t production,
                              int offset,
                              int *counts,
                              int **table)
{
    component_t component;
    int index;
    char *generates;
    int i;

    /* Look up the component and see if it's a nonterminal */
    if ((component = production_get_component(production, offset)) == NULL ||
	! component_is_nonterminal(component)) {
        return;
    }

    index = component_get_index(component);
    generates = self->generates[index];

    /* Go through the generates entry for this component */
    for (i = 0; i < self->nonterminal_count; i++) {
        if (index == i || (generates != NULL && generates[i])) {
            production_t *probe;

            for (probe = self->productions_by_nonterminal[i];
                 *probe != NULL;
                 probe++) {
                add_pairs_entry(
                    counts, table,
                    component_index(self, production_get_component(*probe, 0)),
                    encode(self, production_get_index(*probe), 1));
            }
        }
    }
}



/* Fills in the pairs table for the given kernel.  The table encodes
 * which kernel to go to when a given component is encountered while
 * parsing in this kernel. */
static void
compute_pairs(grammar_t self, kernel_t kernel, int *counts, int **table)
{
    int count = kernel->count;
    int *pairs = kernel->pairs;
    int index;

    /* Add an entry for each kernel item */
    for (index = 0; index < count; index++) {
        int production_index;
        int offset = decode(self, pairs[index], &production_index);
        production_t production = self->productions[production_index];
        component_t component = production_get_component(production, offset);

        /* Insert an entry for the component into the table */
        if (component != NULL) {
            add_pairs_entry(
                counts, table,
                component_index(self, component),
                encode(self, production_index, offset + 1));
        }
    }

    /* Compute the closure of each kernel item using the generates table */
    for (index = 0; index < count; index++) {
        int production_index;
        int offset = decode(self, pairs[index], &production_index);

        compute_pairs_for_kernel_item(
            self,
            self->productions[production_index], offset,
            counts, table);
    }
}

/* Computes the LR(0) kernels for the grammar */
int
compute_LR0_kernels(grammar_t self)
{
    int *pairs;
    int count = grammar_get_component_count(self);
    int *pairs_counts;
    int **pairs_table;
    int *goto_table;
    int i, j;

    /* Construct the first kernel to seed the table */
    pairs = (int *)malloc(sizeof(int));
    pairs[0] = encode(self, 0, 0);
    intern_kernel(self, 1, pairs);

    /* Allocate some room for the goto table */
    pairs_counts = (int *)calloc(count, sizeof(int *));
    pairs_table = (int **)calloc(count, sizeof(int *));

    /* Do the goto table thing for each kernel */
    for (i = 0; i < self->kernel_count; i++) {
        /* Compute the pairs from the kernel */
        compute_pairs(self, self->kernels[i], pairs_counts, pairs_table);

        /* Allocate room for the resulting goto table */
        goto_table = (int *)calloc(count, sizeof(int));

        /* Translate the pairs into kernel indices */
        for (j = 0; j < count; j++) {
            goto_table[j] = intern_kernel(self, pairs_counts[j],
                                          pairs_table[j]);
            pairs_counts[j] = 0;
            pairs_table[j] = NULL;
        }

        /* Set the kernel's goto table */
        self->kernels[i]->goto_table = goto_table;
    }

    /* Clean up */
    free(pairs_counts);
    free(pairs_table);

    return 0;
}


/* Mark the nonterminals which can appear first in the given nonterminal */
static void
mark_firsts_with_table(grammar_t self,
                       component_t nonterminal,
                       char *table,
                       char *tried)
{
    int index = component_get_index(nonterminal);
    production_t *probe;

    /* Go through the productions and mark accordingly */
    for (probe = self->productions_by_nonterminal[index];
         *probe != NULL;
         probe++) {
        int pi = production_get_index(*probe);

        /* Try this production if we haven't already done so */
        if (! tried[pi]) {
            component_t component;

            /* Indicate that we've now tried this production rule */
            tried[pi] = 1;

            /* Look up the first component of the production */
            component = production_get_component(*probe, 0);
            if (component_is_nonterminal(component)) {
                /* Recursively add the first elements of the nonterminal */
                mark_firsts_with_table(self, component, table, tried);
            } else {
                /* Just add the terminal symbol */
                table[component_get_index(component)] = 1;
            }
        }
    }
}

/* Marks the nonterminals which can appear first in the given nonterminal */
static void
mark_firsts(grammar_t self, component_t nonterminal, char *table)
{
    char *tried;

    /* Make a table in which we can keep track of the productions
     * we've tried */
    tried = (char *)calloc(self->production_count, sizeof(char));
    mark_firsts_with_table(self, nonterminal, table, tried);
    free(tried);
}

/* Forward declaration */
static int
compute_propagates_for_production_and_offset(grammar_t self,
                                             kernel_t kernel,
                                             production_t production,
                                             int offset,
                                             component_t terminal,
                                             char *propagates,
                                             char *table);


/* Propagate a terminal to the derived productions */
static int
propagate_derived(grammar_t self,
                  kernel_t kernel,
                  component_t nonterminal,
                  component_t terminal,
                  char *propagates,
                  char *table)
{
    production_t *probe;
    int ni;

    ni = component_get_index(nonterminal);
    for (probe = self->productions_by_nonterminal[ni];
         *probe != NULL;
         probe++) {
        if (compute_propagates_for_production_and_offset(self, kernel,
                                                         *probe, 0,
                                                         terminal, propagates,
                                                         table) < 0) {
            return -1;
        }
    }

    return 0;    
}

/* Compute the propagates table contribution of a given kernel production */
static int
compute_propagates_for_production_and_offset(grammar_t self,
                                             kernel_t kernel,
                                             production_t production,
                                             int offset,
                                             component_t terminal,
                                             char *propagates,
                                             char *table)
{
    component_t component;
    component_t next;
    int pi = production_get_index(production);

    /* If there is no next component then we're done */
    if ((component = production_get_component(production, offset)) == NULL) {
        return 0;
    }

    /* A NULL terminal is the special `propagates' token */
    if (terminal == NULL) {
        /* Kernel items are implicit and shouldn't be recorded */
        if (offset == 0) {
            /* If this production already propagates then bail */
            if (propagates[pi]) {
                return 0;
            }

            /* Mark it as propagating */
            propagates[pi] = 1;
        }
    } else {
        kernel_t target;
        int ti = component_get_index(terminal);
        int code;

        /* Figure out which kernel this belongs in */
        target = self->kernels[kernel->goto_table[component_index(self,
                                                                  component)]];
        code = encode(self, pi, offset + 1);

        /* See if we've already done this one */
        if (table[ti + pi * self->terminal_count]) {
            return 0;
        }
	
        /* Mark this production */
        table[ti + pi * self->terminal_count] = 1;

        /* Put the terminal in the follows set of the destination */
        kernel_set_follows(target, code, ti);
    }

    /* If the component is a terminal then there's nothing else to do */
    if (! component_is_nonterminal(component)) {
        return 0;
    }

    /* If there's no following component then life is easy */
    if ((next = production_get_component(production, offset + 1)) == NULL) {
        propagate_derived(self, kernel, component, terminal,
                          propagates, table);
        return 0;
    }

    /* If it's a nonterminal then things are complicated */
    if (next != NULL && component_is_nonterminal(next)) {
        char *firsts;
        int index;

        /* Allocate some room for the `firsts' table */
        firsts = (char *)calloc(self->terminal_count, sizeof(char));
        if (firsts == NULL) {
            return -1;
        }

        /* Determine what terminals may occupy the first position in
         * the nonterminal */
        mark_firsts(self, next, firsts);

        /* Go through the firsts set and add it to the follows set of
         * our target */
        for (index = 0; index < self->terminal_count; index++) {
            if (firsts[index]) {
                propagate_derived(
                    self, kernel,
                    component, self->terminals[index],
                    propagates,
                    table);
            }
        }

        /* Clean up */
        free(firsts);
        return 0;
    }

    /* Otherwise we just propagate the terminal to the target's follows set */
    propagate_derived(self, kernel, component, next, propagates, table);
    return 0;
}

/* Compute the kernel's propagates table */
static void
compute_propagates_for_kernel(grammar_t self, kernel_t kernel)
{
    int index;

    /* Go through each of the (production, offset) pairs in the kernel */
    for (index = 0; index < kernel->count; index++) {
        int pi;
        char *table;
        char *done_table;
        int offset = decode(self, kernel->pairs[index], &pi);

        /* Create a table to mark the (production, terminal)s that
         * we've done */
        done_table = (char *)calloc(self->terminal_count *
                                    self->production_count, sizeof(char));

        /* Make the propagates table entry if it doesn't already exist */
        if ((table = kernel->propagates_table[index]) == NULL) {
            table = (char *)calloc(self->production_count, sizeof(char));
            kernel->propagates_table[index] = table;
        }

        /* Fill it in (and compute the spontaneously generated follows
         * set info */
        compute_propagates_for_production_and_offset(self, kernel,
                                                     self->productions[pi],
                                                     offset, NULL, table,
                                                     done_table);

        free(done_table);
    }
}


/* Propagate the follows set of the kernel item for the given production */
static void
propagate_kernel_item_follows(grammar_t self,
                              kernel_t kernel,
                              int index,
                              production_t production,
                              int offset,
                              int *changed)
{
    component_t component;
    kernel_t target;
    int code;
    int i;

    /* Look up the next component in the production */
    if ((component = production_get_component(production, offset)) == NULL) {
        return;
    }

    /* Figure out where we go */
    target =
        self->kernels[kernel->goto_table[component_index(self, component)]];
    if (target == NULL) {
        abort();
    }

    code = encode(self, production_get_index(production), offset + 1);

    /* Go through the terminal symbols and propagate those that are
     * present in the follows table. */
    for (i = 0; i < self->terminal_count; i++) {
        if (kernel->follows_table[index][i]) {
            /* Mark the target kernel's follows set */
            if (kernel_set_follows(target, code, i) == 0 && changed != NULL) {
                *changed = 1;
            }
        }
    }
}

/* Propagate the follows table information around */
static void
propagate_follows(grammar_t self, int *changed)
{
    int i, j, k;

    /* Go through the kernels and propagate stuff */
    for (i = 0; i < self->kernel_count; i++) {
        kernel_t kernel = self->kernels[i];

        /* Go through each kernel item */
        for (j = 0; j < kernel->count; j++) {
            /* Go through the productions */
            for (k = -1; k < self->production_count; k++) {
                /* See if we propagate */
                if (k < 0 || kernel->propagates_table[j][k]) {
                    int pi;
                    int offset;

                    /* Work out the production and offset */
                    if (k < 0) {
                        offset = decode(self, kernel->pairs[j], &pi);
                    } else {
                        pi = k;
                        offset = 0;
                    }

                    /* Propagate everything in the follows table */
                    propagate_kernel_item_follows(
                        self, kernel, j,
                        self->productions[pi], offset,
                        changed);
                }
            }
        }
    }
}


/* Compute a table which encodes which terminals can follow each
 * production rule in a given kernel */
static int
compute_propagates(grammar_t self)
{
    int index;
    int changed;

    /* Prepare the kernels for propagation table construction */
    for (index = 0; index < self->kernel_count; index++) {
        compute_propagates_for_kernel(self, self->kernels[index]);
    }

    /* Inject the <EOF> terminal into the start kernel's production */
    self->kernels[0]->follows_table[0][0] = 1;

    /* Move stuff around until things stop changing */
    changed = 1;
    while (changed) {
        changed = 0;
        propagate_follows(self, &changed);
    }

    return 0;
}




/* Allocates and initializes a new nonterminal grammar_t */
grammar_t
grammar_alloc(int production_count, production_t *productions,
              int terminal_count, component_t *terminals,
              int nonterminal_count, component_t *nonterminals)
{
    grammar_t self;

    /* Allocate space for a new grammar_t */
    if ((self = (grammar_t)malloc(sizeof(struct grammar))) == NULL) {
        return NULL;
    }

    /* Initialize its contents to sane values */
    self->production_count = production_count;
    self->productions = productions;
    self->terminal_count = terminal_count;
    self->terminals = terminals;
    self->nonterminal_count = nonterminal_count;
    self->nonterminals = nonterminals;
    self->productions_by_nonterminal = NULL;
    self->generates = NULL;
    self->kernel_count = 0;
    self->kernels = NULL;

    /* Compute the productions_by_nonterminal */
    if ((self->productions_by_nonterminal = 
         compute_productions_by_nonterminal(
             nonterminal_count,
             production_count,
             productions)) == NULL) {
        grammar_free(self);
        return NULL;
    }

    /* Make sure that every nonterminal has at least on production that generates it. */
    if (verify_productions_by_nonterminal(self) < 0) {
        grammar_free(self);
        return NULL;
    }

    /* Compute the `generates' table */
    compute_generates(self);

    /* Compute the LR(0) kernels */
    if (compute_LR0_kernels(self) < 0) {
        grammar_free(self);
        return NULL;
    }

    /* Compute the propagation table */
    compute_propagates(self);

    return self;
}

/* Releases the resources consumed by the receiver */
void
grammar_free(grammar_t self)
{
    int index;

    if (self->productions != NULL) {
        for (index = 0; index < self->production_count; index++) {
            production_free(self->productions[index]);
        }

        free(self->productions);
    }

    if (self->terminals != NULL) {
        for (index = 0; index < self->terminal_count; index++) {
            component_free(self->terminals[index]);
        }
    }

    if (self->nonterminals != NULL) {
        for (index = 0; index < self->nonterminal_count; index++) {
            component_free(self->nonterminals[index]);
        }
    }

    free(self);
}

/* Returns the number of components in the grammar */
int
grammar_get_component_count(grammar_t self)
{
    return self->terminal_count + self->nonterminal_count;
}


/* Prints a single kernel */
void
print_kernel(grammar_t self, int index, FILE *out)
{
    kernel_t kernel = self->kernels[index];
    int i;

    fprintf(out, "Kernel %d\n", index);
    for (i = 0; i < kernel->count; i++) {
        int first = 1;
        int pi;
        int j;

        int offset = decode(self, kernel->pairs[i], &pi);
        fprintf(out, " %d: ", i);
        production_print_with_offset(self->productions[pi], out, offset);

        for (j = 0; j < self->terminal_count; j++) {
            if (kernel->follows_table[i][j]) {
                if (first) {
                    fprintf(out, ", ");
                    first = 0;
                } else {
                    fprintf(out, "/ ");
                }

                component_print(self->terminals[j], out);
            }
        }

        fprintf(out, "\n");
    }

    /* Print out the goto table's nonterminals */
    for (i = 0; i < self->nonterminal_count; i++) {
        if (! (kernel->goto_table[i] < 0)) {
            fprintf(out, "    ");
            component_print(self->nonterminals[i], out);
            fprintf(out, ": %d\n", kernel->goto_table[i]);
        }
    }

    /* Print out the goto table's terminals */
    for (i = 0; i < self->terminal_count; i++) {
        if (! (kernel->goto_table[i + self->nonterminal_count] < 0)) {
            fprintf(out, "    ");
            component_print(self->terminals[i], out);
            fprintf(out, ": %d\n",
                    kernel->goto_table[i + self->nonterminal_count]);
        }
    }

    fprintf(out, "\n");
}

/* Print out the kernels */
void
grammar_print_kernels(grammar_t self, FILE *out)
{
    int index;

    for (index = 0; index < self->kernel_count; index++) {
        print_kernel(self, index, out);
    }
}


/* Print out some macros that simplify table access */
static void
print_c_header(grammar_t self, FILE *out)
{
    fprintf(out,
            "/* Generated by %s version %s */\n\n"
            "#define IS_ERROR(action) ((action) == 0)\n"
            "#define IS_ACCEPT(action) ((action) == %d)\n"
            "#define IS_REDUCE(action) (0 < (action) && (action) < %d)\n"
            "#define IS_SHIFT(action) (%d <= (action) && (action) < %d)\n"
            "#define REDUCTION(action) (action)\n"
            "#define REDUCE_GOTO(state, production) \\\n"
            "    (goto_table[state][production->nonterm_type])\n"
            "#define SHIFT_GOTO(action) ((action) - %d)\n\n",
            PACKAGE, VERSION,
            self->production_count + self->kernel_count,
            self->production_count,
            self->production_count,
            self->production_count + self->kernel_count,
            self->production_count);
}

/* Prints an enumeration which lists the various terminals */
static void
print_c_terminal_enum(grammar_t self, FILE *out)
{
    int index;

    /* Print the enum header */
    fprintf(out, "typedef enum\n{\n");

    /* Print each terminal */
    for (index = 0; index < self->terminal_count; index++) {
        component_print_c_enum(self->terminals[index], out);
    }

    fprintf(out, "\n} terminal_t;\n\n");
}

/* Prints out the reduction table */
static void
print_c_reduction_table(grammar_t self, FILE *out)
{
    int index;

    /* Define the structure type */
    fprintf(out,
            "struct production\n{\n"
            "    reduction_t reduction;\n"
            "    int nonterm_type;\n"
            "    int count;\n};\n\n");

    /* Print the table header */
    fprintf(out, "static struct production productions[%d] =\n{\n",
	    self->production_count);

    /* Print the production functions */
    for (index = 0; index < self->production_count; index++) {
        production_t production = self->productions[index];

        /* Put some space between the entries */
        if (index != 0) {
            fprintf(out, ",\n\n");
        }

        /* Print out a comment containing the production */
        fprintf(out, "    /* %d: ", index);
        production_print(production, out);
        fprintf(out, "*/\n");

        /* Print the production's struct */
        production_print_c_struct(production, out);
    }

    /* Print the table footer */
    fprintf(out, "\n};\n\n");
}

/* Returns the index of the first production in the kernel to be
 * listed in the input file */
static int
first_production_index(grammar_t self, kernel_t kernel)
{
    int index;
    int result = self->production_count;

    /* Go through the pairs and find the first listed production */
    for (index = 0; index < kernel->count; index++) {
        int test;

        decode(self, kernel->pairs[index], &test);
        if (test < result) {
            result = test;
        }
    }

    return result;
}

/* Prints out the contribution of a kernel to the SR table */
static void
print_kernel_SR_entry(grammar_t self,
                      int kernel_index,
                      char *lparen,
                      char *rparen,
                      char *separator,
                      FILE *out)
{
    kernel_t kernel = self->kernels[kernel_index];
    int *reductions;
    int index;

    /* Create a table in which to record the reductions */
    reductions = (int *)malloc(self->terminal_count * sizeof(int));
    memset(reductions, -1, self->terminal_count * sizeof(int));

    /* Populate the reductions table */
    for (index = 0; index < kernel->count; index++) {
        int pi;
        int offset = decode(self, kernel->pairs[index], &pi);
        production_t production = self->productions[pi];

        /* We reduce on the follow set if we're the end of the production */
        if (production_get_component(production, offset) == NULL) {
            int i;

            /* Traverse the follows set */
            for (i = 0; i < self->terminal_count; i++) {
                if (kernel->follows_table[index][i]) {
                    /* Report reduce/reduce conflicts */
                    if (reductions[i] != -1) {
                        fprintf(stderr,
                                "*** Warning: reduce/reduce conflict on ");
                        component_print(self->terminals[i], stderr);
                        fprintf(stderr, "in kernel %d\n", kernel_index);
                        fprintf(stderr, "  [using first listed reduction]\n");
                        print_kernel(self, kernel_index, stderr);
                    } else {
                        reductions[i] = pi;
                    }
                }
            }
        }
    }

    /* Print out the table entry */
    fprintf(out, "    %s", lparen);

    /* Print the terminal transitions */
    for (index = 0; index < self->terminal_count; index++) {
        int ki = kernel->goto_table[self->nonterminal_count + index];
        int shift = (ki < 0) ? 0 : ki;
        int reduction = reductions[index];

        /* Print a comma separator */
        if (index != 0) {
            fprintf(out, separator);
        }

        /* See if there's a shift action for this terminal */
        if (shift != 0) {
            /* Report shift/reduce conflicts */
            if (reduction != -1) {
                int si;

                fprintf(stderr, "*** Warning: shift/reduce conflict on ");
                component_print(self->terminals[index], stderr);
                fprintf(stderr, "in kernel %d\n", kernel_index);

                /* Resolve the conflict according to the order of the
                 * productions in the grammar.  Figure out which
                 * production generated the shift operation */
                si = first_production_index(self, self->kernels[shift]);
                if (reduction < si) {
                    fprintf(out, "R(%d)", reduction);
                    fprintf(stderr, "    [choosing to reduce]\n");
                } else {
                    fprintf(out, "S(%d)", shift);
                    fprintf(stderr, "    [choosing to shift]\n");
                }

                /* Print the kernel for reference */
                print_kernel(self, kernel_index, stderr);
            } else {
                /* Print out a shift */
                fprintf(out, "S(%d)", shift);
            }
        } else if (reduction == 0) {
            /* Accept */
            fprintf(out, "ACC");
        } else if (reduction > 0) {
            /* Normal reduction */
            fprintf(out, "R(%d)", reduction);
        } else {
            /* Error */
            fprintf(out, "ERR");
        }
    }

    /* Close this table entry */
    fprintf(out, rparen);

    /* Clean up */
    free(reductions);
}

/* Prints out the shift/reduce table */
static void
print_c_shift_reduce_table(grammar_t self, FILE *out)
{
    int max = self->production_count + self->kernel_count;
    int index;

    /* Print out some helpful macros */
    fprintf(out,
            "#define ERR 0\n"
            "#define ACC %d\n"
            "#define R(x) (x)\n"
            "#define S(x) (x + %d)\n\n",
            max, self->production_count);

    /* Print the SR table header */
    fprintf(out, "static unsigned int sr_table[%d][%d] =\n{\n",
	    self->kernel_count,
	    self->terminal_count);

    /* Go through each kernel and print out its part of the SR table */
    for (index = 0; index < self->kernel_count; index++) {
	if (index != 0) {
	    fprintf(out, ",\n");
	}

	print_kernel_SR_entry(self, index, "{ ", " }", ", ", out);
    }

    /* Close off the SR table and undefine our macros */
    fprintf(out,
            "\n};\n\n"
            "#undef ERR\n"
            "#undef R\n"
            "#undef S\n\n");
}

/* Prints out the goto table in C format */
static void
print_c_goto_table(grammar_t self, FILE *out)
{
    int index;

    /* Print the goto table header */
    fprintf(out, "static unsigned int goto_table[%d][%d] =\n{\n",
	    self->kernel_count,
	    self->nonterminal_count);

    /* Go through each kernel and print its portion of the goto table */
    for (index = 0; index < self->kernel_count; index++) {
        kernel_t kernel = self->kernels[index];
        int i;

        if (index != 0) {
            fprintf(out, ",\n");
        }

        fprintf(out, "    { ");

        /* Go through each nonterminal and look up its goto information */
        for (i = 0; i < self->nonterminal_count; i++) {
            int ki = kernel->goto_table[i];
            if (i == 0) {
                fprintf(out, "%d", (ki < 0) ? 0 : ki);
            } else {
                fprintf(out, ", %d", (ki < 0) ? 0 : ki);
            }
        }

        fprintf(out, " }");
    }

    /* Close off the goto table */
    fprintf(out, "\n};\n\n");
}

/* Print out the parse tables in C format */
void
grammar_print_c_tables(grammar_t self, FILE *out)
{
    print_c_header(self, out);
    print_c_terminal_enum(self, out);
    print_c_reduction_table(self, out);
    print_c_shift_reduce_table(self, out);
    print_c_goto_table(self, out);
}



/* Prints the python header */
static void
print_python_header(grammar_t self, char *module, FILE *out)
{
    /* Print a comment */
    fprintf(out, "# Generated by %s version %s\n\n", PACKAGE, VERSION);

    /* Print the import line if a module was specified */
    if (module != NULL) {
        fprintf(out, "import %s\n\n", module);
    }
}


/* Prints the terminals in python format */
static void
print_python_terminals(grammar_t self, FILE *out)
{
    int index;

    /* Print a line for each terminal */
    for (index = 0; index < self->terminal_count; index++) {
        component_print_python_assign(self->terminals[index], out);
    }

    fprintf(out, "\n");
}

/* Print out the reductions table in python format */
static void
print_python_reductions(grammar_t self, char *module, FILE *out)
{
    int index;

    /* Print the table header */
    fprintf(out, "productions = (\n");

    /* Print out the production rules */
    for (index = 0; index < self->production_count; index++) {
        production_t production = self->productions[index];

        /* Put some space between the entries */
        if (index != 0) {
            fprintf(out, ",\n\n");
        }

        /* Print out a comment containing the production */
        fprintf(out, "    # %d: ", index);
        production_print(production, out);
        fprintf(out, "\n");

        /* Print the production's python tuple */
        production_print_python_tuple(production, module, out);
    }

    fprintf(out, ")\n\n");
}

/* Prints out the shift-reduce table in python format */
static void
print_python_shift_reduce_table(grammar_t self, FILE *out)
{
    int index;

    /* Print out some functions which help generate tables */
    fprintf(out,
            "ERR = -1\n"
            "ACC = 0\n\n"
            "def R(x):\n"
            "    return x\n\n"
            "def S(x):\n"
            "    return x + %d\n\n",
            self->production_count);

    fprintf(out, "sr_table = (\n");

    /* Go through each kernel and print out its part of the SR table */
    for (index = 0; index < self->kernel_count; index++) {
        if (index != 0) {
            fprintf(out, ",\n");
        }

        print_kernel_SR_entry(self, index, "(", ")", ", ", out);
    }

    fprintf(out, ")\n\n");
}

/* Prints out the goto table in python format */
static void
print_python_goto_table(grammar_t self, FILE *out)
{
    int index;

    /* Print the goto table */
    fprintf(out, "goto_table = (\n");

    /* Go through each kernel and print its portion of the goto table */
    for (index = 0; index < self->kernel_count; index++) {
        kernel_t kernel = self->kernels[index];
        int i;

        if (index != 0) {
            fprintf(out, ",\n");
        }

        fprintf(out, "    (");

        /* Go through each nonterminal and look up its goto information */
        for (i = 0; i < self->nonterminal_count; i++) {
            int ki = kernel->goto_table[i];
            if (i == 0) {
                fprintf(out, "%d", (ki < 0) ? 0 : ki);
            } else {
                fprintf(out, ", %d", (ki < 0) ? 0 : ki);
            }
        }

        fprintf(out, ")");
    }

    /* Close off the goto table */
    fprintf(out, ")\n\n");
}

/* Print out the parse tables in python format */
void
grammar_print_python_tables(grammar_t self, char *module, FILE *out)
{
    print_python_header(self, module, out);
    print_python_terminals(self, out);
    print_python_reductions(self, module, out);
    print_python_shift_reduce_table(self, out);
    print_python_goto_table(self, out);
}
