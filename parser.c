/* -*- mode: c; c-file-style: "elvin" -*- */
/***********************************************************************

  Copyright (C) 1999-2006 by Mantara Software (ABN 17 105 665 594).
  All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.

   * Neither the name of the Mantara Software nor the names
     of its contributors may be used to endorse or promote
     products derived from this software without specific prior
     written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "component.h"
#include "production.h"
#include "grammar.h"
#include "parser.h"

typedef void *(*reduction_t)(parser_t self);

/* Prototypes for the reduction functions */
static void *accept_grammar(parser_t self);
static void *extend_production_list(parser_t self);
static void *make_production_list(parser_t self);
static void *make_production(parser_t self);
static void *extend_exp_list(parser_t self);
static void *make_exp_list(parser_t self);
static void *make_nonterminal(parser_t self);
static void *make_terminal(parser_t self);
static void *make_reduction(parser_t self);

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

    /* The filename from which we're reading */
    char *filename;

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

    /* The line currently being read */
    int line;

    /* The parser's current lexical state */
    lexer_state_t lex_state;

    /* The line number of the last id token */
    int id_token_line;

    /* The token buffer */
    char *token;

    /* The end of the token buffer */
    char *token_end;

    /* The next character in the token buffer */
    char *point;

    /* The number of different terminal symbols */
    int terminal_count;

    /* The terminal symbols */
    component_t *terminals;

    /* The number of different nonterminal symbols */
    int nonterminal_count;

    /* The nonterminal symbols */
    component_t *nonterminals;

    /* The number of components in the current production */
    int component_count;

    /* The components of the current production */
    component_t *components;

    /* The number of productions */
    int production_count;

    /* The productions */
    production_t *productions;
};


/* Function prototypes for the lexer states */
static int lex_start(parser_t self, int ch);
static int lex_comment(parser_t self, int ch);
static int lex_colon(parser_t self, int ch);
static int lex_colon_colon(parser_t self, int ch);
static int lex_id(parser_t self, int ch);
static int lex_error(parser_t self, int ch);


/* Expands the stack */
static int
grow_stack(parser_t self)
{
    size_t new_count = (self->state_end - self->state_top) * 2;
    int *new_state_stack;
    void **new_value_stack;

    /* Try to allocate more memory for the state stack */
    new_state_stack = (int *)realloc(self->state_stack,
                                     new_count * sizeof(int));
    if (new_state_stack == NULL) {
	return -1;
    }

    /* Try to allocate memory for the new value stack */
    new_value_stack = (void **)realloc(self->value_stack,
                                       new_count * sizeof(void *));
    if (new_value_stack == NULL) {
	return -1;
    }

    /* Update the pointers */
    self->state_end = new_state_stack + new_count;
    self->state_top = self->state_top - self->state_stack + new_state_stack;
    self->state_stack = new_state_stack;

    self->value_top = self->value_top - self->value_stack + new_value_stack;
    self->value_stack = new_value_stack;
    return 0;
}

/* Push a state and value onto the stack */
static int
push(parser_t self, int state, void *value)
{
    if (! (self->state_top < self->state_end)) {
	if (grow_stack(self) < 0) {
	    return -1;
	}
    }

    /* The state stack is pre-increment */
    *(++self->state_top) = state;

    /* The value stack is post-increment */
    *(self->value_top++) = value;

    return 0;
}

/* Move the top of the stack back `count' positions */
static void
pop(parser_t self, int count)
{
    if (self->state_stack > self->state_top - count) {
	fprintf(stderr, "pop underflow\n");
	abort();
    }

    self->state_top -= count;
    self->value_top -= count;
}

/* Returns the top of the state stack */
static int
top(parser_t self)
{
    return *(self->state_top);
}


/* Prints an error message */
static void
print_parse_error(parser_t self, terminal_t type)
{
    char *token;
    char *file = self->filename == NULL ? "<stdin>" : self->filename;

    /* Convert the token back into a string */
    switch (type) {
    case TT_EOF:
        fprintf(stderr, "%s:%d: unexpected end of file\n", file, self->line);
        return;

    case TT_DERIVES:
        token = "::=";
        break;

    case TT_LT:
        token = "<";
        break;

    case TT_ID:
        token = self->token;
        break;

    case TT_GT:
        token = ">";
        break;

    case TT_LBRACKET:
        token = "[";
        break;

    case TT_RBRACKET:
        token = "]";
        break;

    default:
        abort();
    }

    fprintf(stderr, "%s:%d: parse error before `%s'\n",
            file, self->line, token);
}

/* Perform all possible reductions and then shift in the terminal */
static int
shift_reduce(parser_t self, terminal_t type, void *value)
{
    int action;
    void *result;

    /* Reduce as many times as possible */
    while (IS_REDUCE(action = sr_table[top(self)][type])) {
	struct production *production;
	int reduction;

	/* Locate the production we're going to use to do the reduction */
	reduction = REDUCTION(action);
	production = productions + reduction;

	/* Point the stack to the beginning of the components */
	pop(self, production->count);

	/* Reduce by calling the production's reduction */
	if ((result = production->reduction(self)) == NULL) {
	    fprintf(stderr, "reduce error\n");
	    abort();
	}

	/* Push the result of the reduction back onto the stack */
	if (push(self, REDUCE_GOTO(top(self), production), result) < 0) {
	    fprintf(stderr, "push error\n");
	    abort();
	}
    }

    /* Can we shift? */
    if (IS_SHIFT(action)) {
	return push(self, SHIFT_GOTO(action), value);
    }

    /* Can we accept? */
    if (IS_ACCEPT(action)) {
	/* Set up the stack */
	pop(self, 1);

	/* Finish parsing */
	if ((result = accept_grammar(self)) == NULL) {
	    return -1;
	}

	/* Deliver the grammar to the callback */
	if (self->callback != NULL) {
	    self->callback(self->rock, result);
	}

	return 0;
    }

    /* Watch for errors */
    if (IS_ERROR(action)) {
	print_parse_error(self, type);
	return -1;
    }

    return 0;
}

static int
accept_eof(parser_t self)
{
    int result = shift_reduce(self, TT_EOF, NULL);
    self->line = 1;
    return result;
}

static int
accept_error(parser_t self, char *token)
{
    char *file = self->filename == NULL ? "<stdin>" : self->filename;

    fprintf(stderr, "%s:%d: invalid token `%s'\n",
            file, self->line, self->token);
    return -1;
}

static int
accept_lt(parser_t self)
{
    return shift_reduce(self, TT_LT, NULL);
}

static int
accept_gt(parser_t self)
{
    return shift_reduce(self, TT_GT, NULL);
}

static int
accept_lbracket(parser_t self)
{
    return shift_reduce(self, TT_LBRACKET, NULL);
}

static int
accept_rbracket(parser_t self)
{
    return shift_reduce(self, TT_RBRACKET, NULL);
}

static int
accept_derives(parser_t self)
{
    return shift_reduce(self, TT_DERIVES, NULL);
}

static int
accept_id(parser_t self, char *id)
{
    char *value;

    /* Make a copy of the id */
    if ((value = strdup(id)) == NULL) {
	return -1;
    }

    /* Record the line number */
    self->id_token_line = self->line;

    /* Push it onto the stack and reduce */
    return shift_reduce(self, TT_ID, value);
}

/* Expands the token buffer */
static int
grow_buffer(parser_t self)
{
    size_t new_length = (self->token_end - self->token) * 2;
    char *new_token;

    /* Allocate a bigger buffer */
    if ((new_token = (char *)realloc(self->token, new_length)) == NULL) {
	return -1;
    }

    /* Update the other pointers */
    self->point = self->point - self->token + new_token;
    self->token = new_token;
    self->token_end = new_token + new_length;
    return 0;
}

/* Appends a character to the end of the token buffer */
static int
append_char(parser_t self, int ch)
{
    /* Make sure there's enough room */
    if (! (self->point < self->token_end)) {
	if (grow_buffer(self) < 0) {
	    return -1;
	}
    }

    *(self->point++) = ch;
    return 0;
}


/* Awaiting the first character of a token */
static int
lex_start(parser_t self, int ch)
{
    switch (ch) {
    case EOF:
	/* Watch for the end-of-file marker */
        self->lex_state = lex_start;
        return accept_eof(self);

    case '#':
        /* Watch for a comment */
        self->lex_state = lex_comment;
        return 0;

    case ':':
	/* Watch for a colon */
        self->point = self->token;
        if (append_char(self, ch) < 0) {
            return -1;
        }

        self->lex_state = lex_colon;
        return 0;

    case '<':
	/* Watch for a less-than symbol */
        self->lex_state = lex_start;
        return accept_lt(self);

    case '>':
	/* Watch for a greater-than symbol */
        self->lex_state = lex_start;
        return accept_gt(self);

    case '[':
	/* Watch for a left square bracket */
        self->lex_state = lex_start;
        return accept_lbracket(self);

    case ']':
	/* Watch for a right square bracket */
        self->lex_state = lex_start;
        return accept_rbracket(self);
    }

    /* Skip whitespace */
    if (isspace(ch)) {
	self->lex_state = lex_start;
	return 0;
    }

    /* Alpha or underscore is the beginning of an ID */
    if (ch == '_' || isalpha(ch)) {
	self->point = self->token;
	return lex_id(self, ch);
    }

    /* Anything else is bogus */
    self->point = self->token;
    return lex_error(self, ch);
}

static int
lex_comment(parser_t self, int ch)
{
    /* Watch for EOF */
    if (ch == EOF) {
	return lex_start(self, ch);
    }

    /* Watch for newline */
    if (ch == '\n') {
	return lex_start(self, ch);
    }

    /* Anything else is part of the comment (and ignored) */
    return 0;
}

/* We've seen a single colon.  Look for a second */
static int
lex_colon(parser_t self, int ch)
{
    /* Look for another colon */
    if (ch == ':') {
	if (append_char(self, ch) < 0) {
	    return -1;
	}

	self->lex_state = lex_colon_colon;
	return 0;
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int
lex_colon_colon(parser_t self, int ch)
{
    /* Look for an equals sign */
    if (ch == '=') {
	self->lex_state = lex_start;
	return accept_derives(self);
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int
lex_id(parser_t self, int ch)
{
    /* Watch for additional id characters */
    if (ch == '_' || ch == '-' || isalnum(ch)) {
	if (append_char(self, ch) < 0) {
	    return -1;
	}

	self->lex_state = lex_id;
	return 0;
    }

    /* Null-terminate the ID token */
    if (append_char(self, 0) < 0) {
	return -1;
    }

    /* Accept the token */
    if (accept_id(self, self->token) < 0) {
	return -1;
    }

    /* Scan the character again from the start state */
    return lex_start(self, ch);
}

/* We've encountered a bogus token.  Continue reading it until we have
 * a complete token to play with before complaining */
static int
lex_error(parser_t self, int ch)
{
    /* Watch for the end of the token */
    if (ch == EOF || isspace(ch)) {
	/* Null-terminate the token */
	if (append_char(self, 0) < 0) {
	    return -1;
	}

	/* Acknowledge a broken token */
	if (accept_error(self, self->token) < 0) {
	    return -1;
	}

	/* Rescan this character */
	return lex_start(self, ch);
    }

    /* This must still be part of the token */
    if (append_char(self, ch) < 0) {
	return -1;
    }

    self->lex_state = lex_error;
    return 0;
}


/* Returns the nonterminal_t with the given name, creating it if necessary */
static component_t
intern_terminal(parser_t self, char *name)
{
    component_t component;
    int index;

    /* See if we've already encountered this terminal symbol */
    for (index = 0; index < self->terminal_count; index++) {
	component = self->terminals[index];
	if (strcmp(name, component_get_name(component)) == 0) {
	    return component;
	}
    }

    /* Not there so create one */
    component = terminal_alloc(self->filename, self->id_token_line,
                               name, self->terminal_count);

    /* Make space in the table for it */
    self->terminals = (component_t *)realloc(self->terminals,
                                             (self->terminal_count + 1) *
                                             sizeof(component_t));
    self->terminals[self->terminal_count++] = component;

    return component;
}

/* Returns the nonterminal_t with the given name, creating it if necessary */
static component_t
intern_nonterminal(parser_t self, char *name)
{
    component_t component;
    int index;

    /* See if we've already encountered this terminal symbol */
    for (index = 0; index < self->nonterminal_count; index++) {
	component = self->nonterminals[index];
	if (strcmp(name, component_get_name(component)) == 0) {
	    return component;
	}
    }

    /* Not there so create one */
    component = nonterminal_alloc(self->filename, self->id_token_line,
                                  name, self->nonterminal_count);

    /* Make space for it in the table */
    self->nonterminals = (component_t *)realloc(self->nonterminals,
                                                (self->nonterminal_count + 1) *
                                                sizeof(component_t));
    self->nonterminals[self->nonterminal_count++] = component;

    return component;
}

/* Adds a component to the production */
static void
add_component(parser_t self, component_t component)
{
    self->components = (component_t *)realloc(self->components,
                                              (self->component_count + 1) *
                                              sizeof(component_t));
    self->components[self->component_count++] = component;
}

/* Adds a production to the list */
static void
add_production(parser_t self, production_t production)
{
    self->productions = (production_t *)realloc(self->productions,
                                                (self->production_count + 1) *
                                                sizeof(production_t));
    self->productions[self->production_count++] = production;
}


/* <grammar> ::= <production-list> */
static void *
accept_grammar(parser_t self)
{
    grammar_t grammar;

    grammar = grammar_alloc(self->production_count, self->productions,
                            self->terminal_count, self->terminals,
                            self->nonterminal_count, self->nonterminals);
    if (grammar == NULL) {
	return NULL;
    }

    /* Clear our fields so that we can safely be freed */
    self->terminals = NULL;
    self->nonterminals = NULL;
    self->productions = NULL;
    return grammar;
}

/* <production-list> ::= <production-list> <production> */
static void *
extend_production_list(parser_t self)
{
    add_production(self, (production_t)self->value_top[1]);
    return self->productions;
}

/* <production-list> ::= <production> */
static void *
make_production_list(parser_t self)
{
    production_t production = (production_t)self->value_top[0];
    add_production(self, production);
/*    production_set_index(production, index);*/
    return self->productions;
}

/* <production> ::= <nonterminal> DERIVES <exp-list> <reduction> */
static void *
make_production(parser_t self)
{
    production_t production;

    production = production_alloc(self->production_count,
                                  (component_t)self->value_top[0],
                                  self->component_count,
                                  self->components,
                                  (char *)self->value_top[4]);

    self->component_count = 0;
    self->components = NULL;
    return production;
}

/* <exp-list> ::= <exp-list> <nonterminal> */
/* <exp-list> ::= <exp-list> <terminal> */
static void *
extend_exp_list(parser_t self)
{
    add_component(self, (component_t)self->value_top[1]);
    return self->components;
}

/* <exp-list> ::= <nonterminal> */
/* <exp-list> ::= <terminal> */
static void *
make_exp_list(parser_t self)
{
    add_component(self, (component_t)self->value_top[0]);
    return self->components;
}

/* <nonterminal> ::= LT ID GT */
static void *
make_nonterminal(parser_t self)
{
    char *name;
    component_t nonterminal;

    name = self->value_top[1];
    nonterminal = intern_nonterminal(self, name);
    free(name);
    return nonterminal;
}

/* <terminal> ::= ID */
static void *
make_terminal(parser_t self)
{
    char *name;
    component_t terminal;

    name = self->value_top[0];
    terminal = intern_terminal(self, name);
    free(name);
    return terminal;
}

/* <reduction> ::= LBRACKET ID RBRACKET */
static void *
make_reduction(parser_t self)
{
    return self->value_top[1];
}


/* Allocates and initializes a new parser_t */
parser_t
parser_alloc(parser_callback_t callback, void *rock)
{
    parser_t self;

    /* Allocate some memory for the new parser_t */
    if ((self = (parser_t)malloc(sizeof(struct parser))) == NULL) {
	return NULL;
    }

    /* Set everything to a sane value: 0 */
    memset(self, 0, sizeof(struct parser));

    /* Initialize all the fields to sane values */
    self->callback = callback;
    self->rock = rock;
    self->line = 1;
    self->lex_state = lex_start;

    /* Allocate some space for the state stack */
    if ((self->state_stack = (int *)calloc(INITIAL_STACK_SIZE,
                                           sizeof(int))) == NULL) {
	parser_free(self);
	return NULL;
    }

    /* Allocate space for the value stack */
    if ((self->value_stack = (void **)calloc(INITIAL_STACK_SIZE,
                                             sizeof(void *))) == NULL) {
	parser_free(self);
	return NULL;
    }

    /* Set up the stack pointers */
    self->state_end = self->state_stack + INITIAL_STACK_SIZE;
    self->state_top = self->state_stack;
    *(self->state_top) = 0;
    self->value_top = self->value_stack;

    /* Allocate some room for the token buffer */
    if ((self->token = (char *)malloc(INITIAL_BUFFER_SIZE)) == NULL) {
	parser_free(self);
	return NULL;
    }

    /* Set up the token pointers */
    self->token_end = self->token + INITIAL_BUFFER_SIZE;

    /* Create a dummy terminal for EOF */
    if (intern_terminal(self, "<EOF>") == NULL) {
	parser_free(self);
	return NULL;
    }

    return self;
}

/* Releases the resources consumed by the receiver */
void
parser_free(parser_t self)
{
    int index;

    if (self->state_stack != NULL) {
	free(self->state_stack);
    }

    if (self->value_stack != NULL) {
	free(self->value_stack);
    }

    if (self->token != NULL) {
	free(self->token);
    }

    if (self->terminals != NULL) {
	for (index = 0; index < self->terminal_count; index++) {
	    component_free(self->terminals[index]);
	}

	free(self->terminals);
    }

    if (self->nonterminals != NULL) {
	for (index = 0; index < self->nonterminal_count; index++) {
	    component_free(self->nonterminals[index]);
	}

	free(self->nonterminals);
    }

    if (self->components) {
	free(self->components);
    }

    if (self->productions != NULL) {
	for (index = 0; index < self->production_count; index++) {
	    production_free(self->productions[index]);
	}

	free(self->productions);
    }

    free(self);
}

/* Parses the characters in the buffer */
int
parser_parse(parser_t self, char *filename,
             unsigned char *buffer, size_t length)
{
    unsigned char *pointer;

    /* Record the filename */
    self->filename = filename;

    /* An empty buffer indicates end of file */
    if (length == 0) {
	return self->lex_state(self, EOF);
    }

    /* Otherwise send each character through the lexer */
    for (pointer = buffer; pointer < buffer + length; pointer++) {
	int ch;

	if (self->lex_state(self, ch = *pointer) < 0) {
	    self->lex_state = lex_start;
	    self->filename = NULL;
	    return -1;
	}

	/* Keep track of linefeeds */
	if (ch == '\n') {
	    self->line++;
	}
    }

    /* Discard the filename */
    self->filename = NULL;
    return 0;
}
