/* $Id: Production.h,v 1.4 1999/02/08 19:44:30 phelps Exp $ */

#ifndef PRODUCTION_H
#define PRODUCTION_H

typedef struct Production_t *Production;

#include "Component.h"
#include "Nonterminal.h"
#include "List.h"

/* Answers a new Production */
Production Production_alloc(Nonterminal nonterminal, List components, int index);

/* Frees the resources consumed by a Production */
void Production_free(Production self);

/* Pretty-prints the receiver */
void Production_print(Production self, FILE *out);

/* Pretty-prints the receiver with a * after the nth element */
void Production_printWithOffset(Production self, FILE *out, int offset);

/* Answers the receiver's index */
int Production_getIndex(Production self);

/* Answers the index of the receiver's Nonterminal */
int Production_getNonterminalIndex(Production self);

/* Answers the number of Component in the receiver */
int Production_getCount(Production self);

/* Answers one of the receiver's Components */
Component Production_getComponent(Production self, int index);

/* Answers the receiver's first Component */
Component Production_getFirstComponent(Production self);

#endif /* PRODUCTION_H */
