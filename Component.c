/* $Id: Component.c,v 1.1 1999/02/08 16:30:37 phelps Exp $ */

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
