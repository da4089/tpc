/***************************************************************

  Copyright (C) DSTC Pty Ltd (ACN 052 372 577) 1995.
  Unpublished work.  All Rights Reserved.

  The software contained on this media is the property of the
  DSTC Pty Ltd.  Use of this software is strictly in accordance
  with the license agreement in the accompanying LICENSE.DOC
  file.  If your distribution of this software does not contain
  a LICENSE.DOC file then you have no rights to use this
  software in any manner and should contact DSTC at the address
  below to determine an appropriate licensing arrangement.

     DSTC Pty Ltd
     Level 7, Gehrmann Labs
     University of Queensland
     St Lucia, 4072
     Australia
     Tel: +61 7 3365 4310
     Fax: +61 7 3365 4311
     Email: enquiries@dstc.edu.au

  This software is being provided "AS IS" without warranty of
  any kind.  In no event shall DSTC Pty Ltd be liable for
  damage of any kind arising out of or in connection with
  the use or performance of this software.

****************************************************************/

#ifndef lint
static const char cvsid[] = "$Id: parser.c,v 1.3 1999/12/11 16:58:19 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"
#include "component.h"
#include "production.h"
#include "grammar.h"

typedef void *(*reduce_function_t )(parser_t self);

/* Prototypes for the reduction functions */
static void *accept_grammar(parser_t self);
static void *extend_production_list(parser_t self);
static void *make_production_list(parser_t self);
static void *make_production(parser_t self);
static void *add_component(parser_t self);
static void *make_exp_list(parser_t self);
static void *make_nonterminal(parser_t self);
static void *make_terminal(parser_t self);
static void *make_function(parser_t self);

#include "pcg.h"

#define INITIAL_BUFFER_SIZE 512
#define INITIAL_STACK_SIZE 16

/* The type of a lexer state */
typedef int (*lexer_state_t)(parser_t self, int ch);


/* The parser data structure */
struct parser
{
    /* The parser's callback */
    parser_callback_t callback;

    /* The callback's user-supplied argument */
    void *rock;

    /* The parser's state stack */
    int *state_stack;

    /* The end of the state stack */
    int *state_end;

    /* The top of the state stack */
    int *state_top;

    /* The parser's value stack */
    void **value_stack;

    /* The top of the value stack */
    void **value_top;

    /* The parser's current lexical state */
    lexer_state_t lex_state;

    /* The token buffer */
    char *token;

    /* The end of the token buffer */
    char *token_end;

    /* The next character in the token buffer */
    char *point;
};


/* Function prototypes for the lexer states */
static int lex_start(parser_t self, int ch);
static int lex_comment(parser_t self, int ch);
static int lex_colon(parser_t self, int ch);
static int lex_colon_colon(parser_t self, int ch);
static int lex_id(parser_t self, int ch);
static int lex_string(parser_t self, int ch);
static int lex_error(parser_t self, int ch);


/* Expands the stack */
static int grow_stack(parser_t self)
{
    size_t new_count = (self -> state_end - self -> state_top) * 2;
    int *new_state_stack;
    void **new_value_stack;

    /* Try to allocate more memory for the state stack */
    if ((new_state_stack = (int *)realloc(
	self -> state_stack, new_count * sizeof(int))) == NULL)
    {
	return -1;
    }

    /* Try to allocate memory for the new value stack */
    if ((new_value_stack = (void **)realloc(
	self -> value_stack, new_count * sizeof(void *))) == NULL)
    {
	return -1;
    }

    /* Update the pointers */
    self -> state_end = new_state_stack + new_count;
    self -> state_top = self -> state_top - self -> state_stack + new_state_stack;
    self -> state_stack = new_state_stack;

    self -> value_top = self -> value_top - self -> value_stack + new_value_stack;
    self -> value_stack = new_value_stack;
    return 0;
}

/* Push a state and value onto the stack */
static int push(parser_t self, int state, void *value)
{
    if (! (self -> state_top < self -> state_end))
    {
	if (grow_stack(self) < 0)
	{
	    return -1;
	}
    }

    /* The state stack is pre-increment */
    *(++self -> state_top) = state;

    /* The value stack is post-increment */
    *(self -> value_top++) = value;

    return 0;
}

/* Move the top of the stack back `count' positions */
static void pop(parser_t self, int count)
{
    if (self -> state_stack > self -> state_top - count)
    {
	fprintf(stderr, "pop underflow\n");
	abort();
    }

    self -> state_top -= count;
    self -> value_top -= count;
}

/* Returns the top of the state stack */
static int top(parser_t self)
{
    return *(self -> state_top);
}



/* Perform all possible reductions and then shift in the terminal */
static int shift_reduce(parser_t self, terminal_t type, void *value)
{
    int action;

    /* Reduce as many times as possible */
    while (IS_REDUCE(action = sr_table[top(self)][type]))
    {
	struct production *production;
	int reduction;
	void *result;

	/* Locate the production we're going to use to do the reduction */
	reduction = REDUCTION(action);
	production = productions + reduction;

	/* Point the stack to the beginning of the components */
	pop(self, production -> count);

	/* Reduce by calling the production's reduction function */
	if ((result = production -> function(self)) == NULL)
	{
	    fprintf(stderr, "reduce error\n");
	    abort();
	}

	/* Push the result of the reduction back onto the stack */
	if (push(self, REDUCE_GOTO(top(self), production), result) < 0)
	{
	    fprintf(stderr, "push error\n");
	    abort();
	}
    }

    /* Can we shift? */
    if (IS_SHIFT(action))
    {
	return push(self, SHIFT_GOTO(action), value);
    }

    /* Can we accept? */
    if (IS_ACCEPT(action))
    {
	/* Set up the stack */
	pop(self, 1);

	accept_grammar(self);
	return 0;
/*	return self -> result = accept_grammar(self) != NULL;*/
    }

    /* Everything else is an error */
    fprintf(stderr, "*** Parse error [state=%d, type=%d]\n", top(self), type);
/*    clean_stack(self);*/
    return 0;
}

static int accept_eof(parser_t self)
{
    return shift_reduce(self, TT_EOF, NULL);
}

static int accept_error(parser_t self, char *token)
{
    printf("ERROR: %s\n", token);
    abort();
}

static int accept_lbracket(parser_t self)
{
    return shift_reduce(self, TT_LBRACKET, NULL);
}

static int accept_rbracket(parser_t self)
{
    return shift_reduce(self, TT_RBRACKET, NULL);
}

static int accept_derives(parser_t self)
{
    return shift_reduce(self, TT_DERIVES, NULL);
}

static int accept_id(parser_t self, char *id)
{
    char *value;

    /* Make a copy of the id */
    if ((value = strdup(id)) == NULL)
    {
	return -1;
    }

    /* Push it onto the stack and reduce */
    return shift_reduce(self, TT_ID, value);
}

static int accept_string(parser_t self, char *string)
{
    char *value;

    /* Make a copy of the string */
    if ((value = strdup(string)) == NULL)
    {
	return -1;
    }

    /* Push it onto the stack and reduce */
    return shift_reduce(self, TT_STRING, value);
}


/* Expands the token buffer */
static int grow_buffer(parser_t self)
{
    size_t new_length = (self -> token_end - self -> token) * 2;
    char *new_token;

    /* Allocate a bigger buffer */
    if ((new_token = (char *)realloc(self -> token, new_length)) == NULL)
    {
	return -1;
    }

    /* Update the other pointers */
    self -> point = self -> point - self -> token + new_token;
    self -> token = new_token;
    self -> token_end = new_token + new_length;
    return 0;
}

/* Appends a character to the end of the token buffer */
static int append_char(parser_t self, int ch)
{
    /* Make sure there's enough room */
    if (! (self -> point < self -> token_end))
    {
	if (grow_buffer(self) < 0)
	{
	    return -1;
	}
    }

    *(self -> point++) = ch;
    return 0;
}


/* Awaiting the first character of a token */
static int lex_start(parser_t self, int ch)
{
    switch (ch)
    {
	/* Watch for the end-of-file marker */
	case EOF:
	{
	    self -> lex_state = lex_start;
	    return accept_eof(self);
	}

	/* Watch for a string */
	case '"':
	{
	    self -> point = self -> token;
	    self -> lex_state = lex_string;
	    return 0;
	}

	/* Watch for a comment */
	case '#':
	{
	    self -> lex_state = lex_comment;
	    return 0;
	}

	/* Watch for a colon */
	case ':':
	{
	    self -> point = self -> token;
	    if (append_char(self, ch) < 0)
	    {
		return -1;
	    }

	    self -> lex_state = lex_colon;
	    return 0;
	}

	/* Watch for a left square bracket */
	case '[':
	{
	    self -> lex_state = lex_start;
	    return accept_lbracket(self);
	}

	/* Watch for a right square bracket */
	case ']':
	{
	    self -> lex_state = lex_start;
	    return accept_rbracket(self);
	}
    }

    /* Skip whitespace */
    if (isspace(ch))
    {
	self -> lex_state = lex_start;
	return 0;
    }

    /* Alpha or underscore is the beginning of an ID */
    if (ch == '_' || isalpha(ch))
    {
	self -> point = self -> token;
	return lex_id(self, ch);
    }

    /* Anything else is bogus */
    self -> point = self -> token;
    return lex_error(self, ch);
}

static int lex_comment(parser_t self, int ch)
{
    /* Watch for EOF */
    if (ch == EOF)
    {
	return lex_start(self, ch);
    }

    /* Watch for newline */
    if (ch == '\n')
    {
	self -> lex_state = lex_start;
	return 0;
    }

    /* Anything else is part of the comment (and ignored) */
    return 0;
}

/* We've seen a single colon.  Look for a second */
static int lex_colon(parser_t self, int ch)
{
    /* Look for another colon */
    if (ch == ':')
    {
	if (append_char(self, ch) < 0)
	{
	    return -1;
	}

	self -> lex_state = lex_colon_colon;
	return 0;
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int lex_colon_colon(parser_t self, int ch)
{
    /* Look for an equals sign */
    if (ch == '=')
    {
	self -> lex_state = lex_start;
	return accept_derives(self);
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int lex_id(parser_t self, int ch)
{
    /* Watch for additional id characters */
    if (ch == '_' || ch == '-' || isalnum(ch))
    {
	if (append_char(self, ch) < 0)
	{
	    return -1;
	}

	self -> lex_state = lex_id;
	return 0;
    }

    /* Null-terminate the ID token */
    if (append_char(self, 0) < 0)
    {
	return -1;
    }

    /* Accept the token */
    if (accept_id(self, self -> token) < 0)
    {
	return -1;
    }

    /* Scan the character again from the start state */
    return lex_start(self, ch);
}

static int lex_string(parser_t self, int ch)
{
    /* Watch for illegal EOF in the string */
    if (ch == EOF)
    {
	return lex_error(self, ch);
    }

    /* Watch for the closing quote */
    if (ch == '"')
    {
	/* Null-terminate the token */
	if (append_char(self, 0) < 0)
	{
	    return -1;
	}

	/* Accept it */
	self -> lex_state = lex_start;
	return accept_string(self, self -> token);
    }

    /* Anything else is part of the string */
    if (append_char(self, ch) < 0)
    {
	return -1;
    }

    self -> lex_state = lex_string;
    return 0;
}

/* We've encountered a bogus token.  Continue reading it until we have 
 * a complete token to play with before complaining */
static int lex_error(parser_t self, int ch)
{
    /* Watch for the end of the token */
    if (ch == EOF || isspace(ch))
    {
	/* Null-terminate the token */
	if (append_char(self, 0) < 0)
	{
	    return -1;
	}

	/* Acknowledge a broken token */
	if (accept_error(self, self -> token) < 0)
	{
	    return -1;
	}

	/* Rescan this character */
	return lex_start(self, ch);
    }

    /* This must still be part of the token */
    if (append_char(self, ch) < 0)
    {
	return -1;
    }

    self -> lex_state = lex_error;
    return 0;
}


/* Returns the nonterminal_t with the given name, creating it if necessary */
static component_t intern_nonterminal(parser_t self, char *name)
{
    /* FIX THIS: need a hash table to ensure uniqueness */
    return nonterminal_alloc(name, 19);
}

/* Returns the nonterminal_t with the given name, creating it if necessary */
static component_t intern_terminal(parser_t self, char *name)
{
    /* FIX THIS: need a hash table to ensure uniqueness */
    return terminal_alloc(name, 19);
}

/* grammar ::= production-list */
static void *accept_grammar(parser_t self)
{
    printf("accept_grammar()\n");
    exit(1);
}

/* production-list ::= production-list production */
static void *extend_production_list(parser_t self)
{
    grammar_t grammar = (grammar_t)self -> value_top[0];
    production_t production = (production_t)self -> value_top[1];

    grammar_add_production(grammar, production);
    return grammar;
}

/* production-list ::= production */
static void *make_production_list(parser_t self)
{
    grammar_t grammar;
    production_t production;

    /* Allocate a grammar_t to hold the productions */
    if ((grammar = grammar_alloc()) == NULL)
    {
	return NULL;
    }

    /* Add the production to it */
    production = (production_t)self -> value_top[0];
    grammar_add_production(grammar, production);
    return grammar;
}

/* production ::= nonterminal "DERIVES" exp-list function */
static void *make_production(parser_t self)
{
    component_t component = (component_t)self -> value_top[0];
    production_t production = (production_t)self -> value_top[2];
    char *function = (char *)self -> value_top[3];

    production_set_nonterminal(production, component);
    production_set_function(production, function);
    production_print(production, stdout);
    printf("\n");
    return production;
}

/* exp-list ::= exp-list nonterminal */
/* exp-list ::= exp-list terminal */
static void *add_component(parser_t self)
{
    production_t production = (production_t)self -> value_top[0];
    component_t component = (component_t)self -> value_top[1];

    production_add_component(production, component);
    return production;
}

/* exp-list ::= nonterminal */
/* exp-list ::= terminal */
static void *make_exp_list(parser_t self)
{
    production_t production;
    component_t component;

    if ((production = production_alloc(37)) == NULL)
    {
	return NULL;
    }

    component = self -> value_top[0];
    production_add_component(production, component);
    return production;
}

/* nonterminal ::= "ID" */
static void *make_nonterminal(parser_t self)
{
    char *name;
    component_t nonterminal;

    name = self -> value_top[0];
    nonterminal = intern_nonterminal(self, name);
    free(name);
    return nonterminal;
}

/* terminal ::= "STRING" */
static void *make_terminal(parser_t self)
{
    char *name;
    component_t terminal;

    name = self -> value_top[0];
    terminal = intern_terminal(self, name);
    free(name);
    return terminal;
}

/* function ::= "LBRACKET" "ID" "RBRACKET" */
static void *make_function(parser_t self)
{
    return self -> value_top[1];
}


/* Allocates and initializes a new parser_t */
parser_t parser_alloc(parser_callback_t callback, void *rock)
{
    parser_t self;

    /* Allocate some memory for the new parser_t */
    if ((self = (parser_t)malloc(sizeof(struct parser))) == NULL)
    {
	return NULL;
    }

    /* Initialize all the fields to sane values */
    self -> callback = callback;
    self -> rock = rock;
    self -> state_stack = NULL;
    self -> state_end = NULL;
    self -> state_top = NULL;
    self -> value_stack = NULL;
    self -> value_top = NULL;
    self -> lex_state = lex_start;
    self -> token = NULL;
    self -> token_end = NULL;
    self -> point = NULL;

    /* Allocate some space for the state stack */
    if ((self -> state_stack = (int *)calloc(INITIAL_STACK_SIZE, sizeof(int))) == NULL)
    {
	parser_free(self);
	return NULL;
    }

    /* Allocate space for the value stack */
    if ((self -> value_stack = (void **)calloc(INITIAL_STACK_SIZE, sizeof(void *))) == NULL)
    {
	parser_free(self);
	return NULL;
    }

    /* Set up the stack pointers */
    self -> state_end = self -> state_stack + INITIAL_STACK_SIZE;
    self -> state_top = self -> state_stack;
    *(self -> state_top) = 0;
    self -> value_top = self -> value_stack;

    /* Allocate some room for the token buffer */
    if ((self -> token = (unsigned char *)malloc(INITIAL_BUFFER_SIZE)) == NULL)
    {
	parser_free(self);
	return NULL;
    }

    /* Set up the token pointers */
    self -> token_end = self -> token + INITIAL_BUFFER_SIZE;
    return self;
}

/* Releases the resources consumed by the receiver */
void parser_free(parser_t self)
{
    if (self -> state_stack != NULL)
    {
	free(self -> state_stack);
    }

    if (self -> value_stack != NULL)
    {
	free(self -> value_stack);
    }

    if (self -> token != NULL)
    {
	free(self -> token);
    }

    free(self);
}

/* Parses the characters in the buffer */
int parser_parse(parser_t self, unsigned char *buffer, size_t length)
{
    unsigned char *pointer;

    /* An empty buffer indicates end of file */
    if (length == 0)
    {
	return self -> lex_state(self, EOF);
    }

    /* Otherwise send each character through the lexer */
    for (pointer = buffer; pointer < buffer + length; pointer++)
    {
	if (self -> lex_state(self, *pointer) < 0)
	{
	    self -> lex_state = lex_start;
	    return -1;
	}
    }

    return 0;
}

#include <unistd.h>
int main(int argc, char *argv[])
{
    parser_t parser;

    if ((parser = parser_alloc(NULL, NULL)) == NULL)
    {
	perror("parser_alloc(): failed");
	abort();
    }

    while (1)
    {
	char buffer[2048];
	ssize_t length;

	if ((length = read(STDIN_FILENO, buffer, 2048)) < 0)
	{
	    perror("read(): failed");
	    abort();
	}

	parser_parse(parser, buffer, length);

	if (length == 0)
	{
	    return 0;
	}
    }
}
