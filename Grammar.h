/* $Id: Grammar.h,v 1.10 1999/02/12 08:58:41 phelps Exp $
 *
 * A Grammar is a collection of Productions which, together with a
 * starting non-terminal, construe a language.  The Grammar can be
 * used to construct a set of parse tables with which a Parser may be
 * constructed
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

typedef struct Grammar_t *Grammar;

#include "List.h"
#include "Production.h"
#include "Nonterminal.h"
#include "Terminal.h"
#include "Kernel.h"

/* Allocates a new Grammar with the given Productions */
Grammar Grammar_alloc(
    List productions,
    int nonterminal_count,
    Nonterminal *nonterminals,
    int terminal_count,
    Terminal *terminals);

/* Frees the resources consumed by the receiver */
void Grammar_free(Grammar self);

/* Prints debugging information about the receiver */
void Grammar_debug(Grammar self, FILE *out);

/* Answers the number of nonterminals in the receiver */
int Grammar_nonterminalCount(Grammar self);

/* Answers the indexed nonterminal */
Nonterminal Grammar_getNonterminal(Grammar self, int index);

/* Answers the number of terminals in the receiver */
int Grammar_terminalCount(Grammar self);

/* Answers the indexed terminal */
Terminal Grammar_getTerminal(Grammar self, int index);

/* Answers the number of productions in the receiver */
int Grammar_productionCount(Grammar self);

/* Answers the indexed production */
Production Grammar_getProduction(Grammar self, int index);

/* Encodes a Production and offset in a single integer */
int Grammar_encode(Grammar self, Production production, int offset);

/* Answers the Production and offset encoded in the integer */
int Grammar_decode(Grammar self, int number, Production *production_return);

/* Computes the contribute of the encoded production/offset (and
 * derived productions) to the goto table */
void Grammar_computeGoto(Grammar self, List *table, int number);

/* Transforms a table of Kernels into the "actual" kernels of the receiver */
void Grammar_resolveKernels(Grammar self, Kernel *kernels);

/* Answers the index of the Kernel in the receiver */
int Grammar_kernelIndex(Grammar self, Kernel kernel);

/* Answers the Kernel corresponding to the given index */
Kernel Grammar_getKernel(Grammar self, int index);

/* Answers the Productions which are derived from a Nonterminal */
List Grammar_getDerivedProductions(Grammar self, Nonterminal nonterminal);

/* Updates the follows table to indicate that component may
 * be followed by the follows Component.  If follows is NULL, then the 
 * follows information is copied from the source table */
void Grammar_computeClosure(
    Grammar self,
    Component component,
    Component follows,
    char *source,
    char *table);

/* Marks the first terminals of a given non-terminal in the table */
void Grammar_markFirst(Grammar self, Nonterminal nonterminal, char *table);

/* Construct the set of LR(0) states */
void Grammar_getLALRStates(Grammar self);

#endif /* GRAMMAR_H */
