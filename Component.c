/* $Id: Component.c,v 1.2 1999/02/11 01:46:11 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Component.h"

struct Component_t
{
    ComponentFunctions functions;
};


/* Prints the receiver */
void Component_print(Component self, FILE *out)
{
    (self -> functions.print)(self, out);
}

/* Answers non-zero if the receiver is a Nonterminal */
int Component_isNonterminal(Component self)
{
    return (self -> functions.isNonterminal)(self);
}

/* Marks the terminals which can appear as the first element of the receiver */
void Component_markFirst(Component self, Grammar grammar, char *table)
{
    (self -> functions.markFirst)(self, grammar, table);
}
