/* $Id: Component.h,v 1.1 1999/02/08 16:30:38 phelps Exp $
 *
 * Common code for Terminals and Nonterminals
 */

#ifndef COMPONENT_H
#define COMPONENT_H

typedef struct Component_t *Component;

/* The format of the Print method */
typedef void (*PrintMethod)(Component self, FILE *out);

/* The format of the IsNonterminalMethod */
typedef int (*IsNonterminalMethod)(Component self);


/* The structure of the functions table */
typedef struct ComponentFunctions_t
{
    /* Pretty-prints the Component */
    PrintMethod print;

    /* Answers non-zero if the Component is a Nonterminal */
    IsNonterminalMethod isNonterminal;
} ComponentFunctions;

/* Pretty-prints the receiver */
void Component_print(Component self, FILE *out);

#endif /* COMPONENT_H */