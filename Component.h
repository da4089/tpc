/* $Id: Component.h,v 1.3 1999/02/11 01:46:12 phelps Exp $
 *
 * Common code for Terminals and Nonterminals
 */

#ifndef COMPONENT_H
#define COMPONENT_H

typedef struct Component_t *Component;

#include "Grammar.h"

/* The format of the Print method */
typedef void (*PrintMethod)(Component self, FILE *out);

/* The format of the IsNonterminal method */
typedef int (*IsNonterminalMethod)(Component self);

/* The format of the MarkFirst method */
typedef void (*MarkFirstMethod)(Component self, Grammar grammar, char *table);


/* The structure of the functions table */
typedef struct ComponentFunctions_t
{
    /* Pretty-prints the Component */
    PrintMethod print;

    /* Answers non-zero if the Component is a Nonterminal */
    IsNonterminalMethod isNonterminal;

    /* Marks the terminals which can appear as the first element of the receiver */
    MarkFirstMethod markFirst;
} ComponentFunctions;

/* Pretty-prints the receiver */
void Component_print(Component self, FILE *out);

/* Answers non-zero if the receiver is a Nonterminal */
int Component_isNonterminal(Component self);

/* Marks the terminals which can appear as the first element of the receiver */
void Component_markFirst(Component self, Grammar grammar, char *table);

#endif /* COMPONENT_H */
