/* $Id: Parser.c,v 1.1 1999/02/08 09:23:54 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Parser.h"

/* The Parser data structure */
struct Parser_t
{
    /* The receiver's stack */
    void *stack;

    /* The receiver's accept callback */
    AcceptCallback callback;

    /* The receiver's accept callback context */
    void *context;
};

/* Answers a new Parser */
Parser Parser_alloc(AcceptCallback callback, void *context)
{
    Parser self;

    /* Allocate memory for the Parser */
    if ((self = (Parser) malloc(sizeof(struct Parser_t))) == NULL)
    {
	fprintf(stderr, "*** Out of memory\n");
	exit(1);
    }

    self -> stack = NULL;
    self -> callback = callback;
    self -> context = context;
    return self;
}

/* Releases the resources consumed by the receiver */
void Parser_free(Parser self)
{
    free(self);
}


/* Updates the receiver's state based on the next ::= token */
void Parser_acceptDerives(Parser self)
{
    printf("::= "); fflush(stdout);
}

/* Updates the receiver's state based on the next non-terminal token */
void Parser_acceptNonterminal(Parser self, char *value)
{
    printf("<%s> ", value); fflush(stdout);
}

/* Updates the receiver's state based on the next terminal token */
void Parser_acceptTerminal(Parser self, char *value)
{
    printf("%s ", value); fflush(stdout);
}

/* Updates the receiver's state based on the next stop token */
void Parser_acceptStop(Parser self)
{
    printf(".\n");
}

/* Updates the receiver's state based on the end of input */
void Parser_acceptEOF(Parser self)
{
    printf("[EOF]\n");
}

