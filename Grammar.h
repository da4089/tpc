/* $Id: Grammar.h,v 1.5 1999/02/11 01:48:39 phelps Exp $
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

/* Allocates a new Grammar with the given Productions */
Grammar Grammar_alloc(List productions, int nonterminal_count, int terminal_count);

/* Frees the resources consumed by the receiver */
void Grammar_free(Grammar self);

/* Prints debugging information about the receiver */
void Grammar_debug(Grammar self, FILE *out);

/* Answers the number of nonterminals in the receiver */
int Grammar_nonterminalCount(Grammar self);

/* Answers the number of terminals in the receiver */
int Grammar_terminalCount(Grammar self);

/* Answers the number of productions in the receiver */
int Grammar_productionCount(Grammar self);

/* Computes the contribute of the encoded production/offset (and
 * derived productions) to the goto table */
void Grammar_computeGoto(Grammar self, List *table, int number);

/* Encodes a Production and offset in a single integer */
int Grammar_encode(Grammar self, Production production, int offset);

/* Answers the Production and offset encoded in the integer */
int Grammar_decode(Grammar self, int number, Production *production_return);

/* Construct the set of LR(0) states */
void Grammar_getLR0States(Grammar self);

/* Updates the follows table to indicate that component may
 * be followed by the follows Component.  If follows is NULL, then the 
 * follows information is copied from the source table */
void Grammar_computeClosure(
    Grammar self,
    Component component,
    Component follows,
    char *source,
    char *table);

#endif /* GRAMMAR_H */
