/* $Id: Lexer.c,v 1.2 1999/02/08 08:43:27 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "Lexer.h"
#include "StringBuffer.h"

/* The Lexer data structure */
struct Lexer_t
{
    /* The receiver's current state */
    int state;

    /* The receiver's token buffer */
    StringBuffer buffer;
};


/*
 *
 * Static functions
 *
 */

/* Start reading any token */
static void StartToken(Lexer self, int ch)
{
    /* End of file? */
    if (ch == EOF)
    {
	printf("EOF\n");
	return;
    }

    switch (ch)
    {
	/* Start of ::= */
	case ':':
	{
	    self -> state = 1;
	    return;
	}

	/* Start of non-terminal */
	case '<':
	{
	    StringBuffer_clear(self -> buffer);
	    self -> state = 4;
	    return;
	}

	/* End-of-production character */
	case '.':
	{
	    self -> state = 8;
	    return;
	}
    }

    /* Whitespace? */
    if (isspace(ch))
    {
	self -> state = 0;
	return;
    }

    /* terminal? */
    if ((ch == '_') || (isalpha(ch)))
    {
	StringBuffer_clear(self -> buffer);
	StringBuffer_appendChar(self -> buffer, ch);
	self -> state = 7;
	return;
    }

    /* Invalid character */
    fprintf(stderr, "*** Ignoring bogus character \"%c\"\n", ch);
    self -> state = 0;
}

/* Read ':*:=' */
static void ReadDerives1(Lexer self, int ch)
{
    if (ch == ':')
    {
	self -> state = 2;
	return;
    }

    fprintf(stderr, "*** Invalid token \":%c\"\n", ch);
    self -> state = 0;
}

/* Read '::*=' */
static void ReadDerives2(Lexer self, int ch)
{
    if (ch == '=')
    {
	self -> state = 3;
	return;
    }

    fprintf(stderr, "*** Invalid token \"::%c\"\n", ch);
    self -> state = 0;
}

/* Read '::=*' */
static void AcceptDerives(Lexer self, int ch)
{
    printf("DERIVES [::=]\n");

    self -> state = 0;
    Lexer_acceptChar(self, ch);
}


static void ReadNonterminalName1(Lexer self, int ch)
{
    if ((ch == '_') || isalpha(ch))
    {
	StringBuffer_appendChar(self -> buffer, ch);
	self -> state = 5;
	return;
    }

    fprintf(stderr, "*** Bogus terminal name <%c\n", ch);
    self -> state = 0;
}

static void ReadNonterminalName2(Lexer self, int ch)
{
    if (ch == '>')
    {
	self -> state = 6;
	return;
    }

    if ((ch == '_') || (ch == '-') || isalpha(ch) || isdigit(ch))
    {
	StringBuffer_appendChar(self -> buffer, ch);
	self -> state = 5;
	return;
    }

    fprintf(stderr, "*** Bogus terminal name <%s%c\n", StringBuffer_getBuffer(self -> buffer), ch);
    self -> state = 0;
}

/* Accept a nonterminal token */
static void AcceptNonterminal(Lexer self, int ch)
{
    printf("NONTERMINAL [<%s>]\n", StringBuffer_getBuffer(self -> buffer));

    self -> state = 0;
    Lexer_acceptChar(self, ch);
}

/* Read the next character of a terminal */
static void ReadTerminal(Lexer self, int ch)
{
    if ((ch == '_') || (ch == '-') || isalpha(ch) || isdigit(ch))
    {
	StringBuffer_appendChar(self -> buffer, ch);
	self -> state = 7;
	return;
    }

    printf("TERMINAL [%s]\n", StringBuffer_getBuffer(self -> buffer));

    self -> state = 0;
    Lexer_acceptChar(self, ch);
}

/* Accept an end-of-production character */
static void AcceptStop(Lexer self, int ch)
{
    printf("STOP\n");

    self -> state = 0;
    Lexer_acceptChar(self, ch);
}


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
    self -> buffer = StringBuffer_alloc();
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
    switch (self -> state)
    {
	/* Just starting to read a token */
	case 0:
	{
	    StartToken(self, ch);
	    return;
	}

	/* Reading a derives [::=] token */
	case 1:
	{
	    ReadDerives1(self, ch);
	    return;
	}

	/* Reading a derives [::=] token */
	case 2:
	{
	    ReadDerives2(self, ch);
	    return;
	}

	/* Reading a derives [::=] token */
	case 3:
	{
	    AcceptDerives(self, ch);
	    return;
	}

	/* Reading a non-terminal [<foo>] token */
	case 4:
	{
	    ReadNonterminalName1(self, ch);
	    return;
	}

	/* Reading a non-terminal [<foo>] token */
	case 5:
	{
	    ReadNonterminalName2(self, ch);
	    return;
	}

	/* Accept a non-terminal token */
	case 6:
	{
	    AcceptNonterminal(self, ch);
	    return;
	}

	/* Read a terminal [foo] token */
	case 7:
	{
	    ReadTerminal(self, ch);
	    return;
	}

	/* Read an end-of-production character */
	case 8:
	{
	    AcceptStop(self, ch);
	    return;
	}
    }
}
