/* $Id: Lexer.c,v 1.1 1999/02/08 06:56:32 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Lexer.h"

/* The Lexer data structure */
struct Lexer_t
{
    /* The receiver's current state */
    int state;
};


/*
 *
 * Static functions
 *
 */

/*
 *
 * Exported functions
 *
 */

/* Answers a new Lexer */
Lexer Lexer_alloc(void *callback, void *context)
{
    Lexer self;

    /* Allocate memory for the new Lexer */
    if ((self = (Lexer) malloc(sizeof(struct Lexer_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory\n");
	exit(1);
    }

    /* Set up the initial state */
    self -> state = 0;
    return self;
}


/* Releases the resources consumed by the receiver */
void Lexer_free(Lexer self)
{
    free(self);
}


/* Prints out debugging information about the receiver */
void Lexer_debug(Lexer self)
{
    printf("Lexer (%p)\n", self);
    printf("  state = %d\n", self -> state);
}

/* Updates the recevier's state based on the given character */
void Lexer_acceptChar(Lexer self, int ch)
{
    printf("ch=%d\n", ch);
}
