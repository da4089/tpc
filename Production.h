/* $Id: Production.h,v 1.2 1999/02/08 17:17:34 phelps Exp $ */

#ifndef PRODUCTION_H
#define PRODUCTION_H

typedef struct Production_t *Production;

#include "Component.h"
#include "Nonterminal.h"
#include "List.h"

/* Answers a new Production */
Production Production_alloc(Nonterminal nonterminal, List components);

/* Frees the resources consumed by a Production */
void Production_free(Production self);

/* Pretty-prints the receiver */
void Production_print(Production self, FILE *out);

/* Answers the index of the receiver's Nonterminal */
int Production_getNonterminalIndex(Production self);

/* Answers the receiver's first Component */
Component Production_getFirstComponent(Production self);

#endif /* PRODUCTION_H */
