/* $Id: Parser.c,v 1.12 1999/02/16 12:35:46 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Parser.h"
#include "Grammar.h"
#include "Production.h"
#include "Component.h"
#include "Nonterminal.h"
#include "Terminal.h"
#include "Hash.h"

/*
 * We know the maximum stack for our simple grammar in advance
 * (which is actually about 5, but we'll make it 16 to be safe)
 */
#define STACK_SIZE 16


/*
 *
 * Static function headers
 *
 */

/* Production reduction functions */
static void *Accept(Parser self);
static void *ProductionListExtend(Parser self);
static void *ProductionListCreate(Parser self);
static void *ProductionCreate(Parser self);
static void *ExpListAddNonterminal(Parser self);
static void *ExpListAddTerminal(Parser self);
static void *ExpListCreateNonterminal(Parser self);
static void *ExpListCreateTerminal(Parser self);

/* Can't include this before the reduction function headers */
#include "grammar.h"

static void Push(Parser self, int state, void *value);
static void Pop(Parser self, int count);
static int Top(Parser self);
static void DecodeNonterminal(char *key, Nonterminal nonterminal, Nonterminal *array);
static void DecodeTerminal(char *key, Terminal terminal, Terminal *array);
static void ShiftReduce(Parser self, terminal_t type, void *value);


/* Helper functions */
static Nonterminal FindOrCreateNonterminal(Parser self, char *name);
static Terminal FindOrCreateTerminal(Parser self, char *name);


/* The Parser data structure */
struct Parser_t
{
    /* The receiver's accept callback */
    AcceptCallback callback;

    /* The receiver's accept callback context */
    void *context;

    /* The receiver's stack */
    int stack[STACK_SIZE];

    /* A pointer to the top of the receiver's stack */
    int *top;

    /* The receiver's value stack */
    void *value_stack[STACK_SIZE];

    /* The top of the receiver's value stack */
    void **value_top;

    /* The number of productions we've created so far */
    int production_count;

    /* The number of nonterminals we've created so far */
    int nonterminal_count;

    /* The receiver's table of non-terminals */
    Hashtable nonterminals;

    /* The number of terminals we've created so far */
    int terminal_count;

    /* The receiver's table of terminals */
    Hashtable terminals;
};



/*
 *
 * Static functions
 *
 */

/* Push a state onto the stack */
static void Push(Parser self, int state, void *value)
{
    /* State stack is pre-increment */
    *(++self -> top) = state;

    /* Value stack is post-increment */
    *(self -> value_top++) = value;
}

/* Pop states off the stack */
static void Pop(Parser self, int count)
{
    self -> top -= count;
    self -> value_top -= count;
}

/* Returns the top of the stack */
static int Top(Parser self)
{
    return *(self -> top);
}



/* Copies a Nonterminal into an Array */
static void DecodeNonterminal(char *key, Nonterminal nonterminal, Nonterminal *array)
{
    array[Nonterminal_getIndex(nonterminal)] = nonterminal;
}

/* Copies a Terminal into an Array */
static void DecodeTerminal(char *key, Terminal terminal, Terminal *array)
{
    array[Terminal_getIndex(terminal)] = terminal;
}



/* Perform all possible reductions and finally shift in the terminal */
static void ShiftReduce(Parser self, terminal_t type, void *value)
{
    while (1)
    {
	int state = Top(self);
	int action = sr_table[state][type];
	struct production *production;
	int reduction;
	void *result;

	/* Watch for errors */
	if (IS_ERROR(action))
	{
	    /* If we're in state #1 and the input is TT_EOF, then accept */
	    if ((state == 1) && (type == TT_EOF))
	    {
		Accept(self);
		return;
	    }

	    /* Otherwise it's an error */
	    fprintf(stderr, "*** ERROR (state=%d, type=%d)\n", state, type);
	    exit(0);
	}

	/* Shift if we can */
	if (IS_SHIFT(action))
	{
	    Push(self, SHIFT_GOTO(action), value);
	    return;
	}

	/* Locate the production we're going to use to reduce */
	reduction = REDUCTION(action);
	production = &productions[reduction];

	/* Pop stuff off of the stack */
	Pop(self, production -> count);

	/* Reduce */
	result = (production -> function)(self);

	/* Push the result of the reduction back onto the stack */
	Push(self, REDUCE_GOTO(Top(self), production), result);
    }
}





/* Reduce: <START> ::= <production-list> */
static void *Accept(Parser self)
{
    List productions;
    Grammar grammar;
    Nonterminal *nonterminals;
    Terminal *terminals;

    /* Pop the productions-list off the stack */
    Pop(self, 1);

    /* Locate the production list */
    productions = (List) self -> value_top[0];

    /* Copy the nonterminals out of the Hashtable and into the array */
    nonterminals = (Nonterminal *) calloc(Hashtable_size(self -> nonterminals), sizeof(Nonterminal));
    Hashtable_keysAndValuesDoWith(self -> nonterminals, DecodeNonterminal, nonterminals);
    terminals = (Terminal *) calloc(Hashtable_size(self -> terminals) + 1, sizeof(Terminal));
    Hashtable_keysAndValuesDoWith(self -> terminals, DecodeTerminal, terminals);

    /* Create a grammar */
    grammar = Grammar_alloc(
	productions,
	self -> nonterminal_count,
	nonterminals,
	self -> terminal_count,
	terminals);

    /* Call the callback if there is one */
    if (self -> callback != NULL)
    {
	(*self -> callback)(self -> context, grammar);
    }

    return NULL;
}

/* Reduce: <production-list> ::= <production-list> <production> */
static void *ProductionListExtend(Parser self)
{
    List list;
    Production production;

    /* Locate the list and production */
    list = (List) self -> value_top[0];
    production = (Production) self -> value_top[1];

    /* Append the production to the end of the list */
    List_addLast(list, production);

    /* Push the list back onto the stack */
    return list;
}

/* Reduce: <production-list> ::= <production> */
static void *ProductionListCreate(Parser self)
{
    Production production;
    List list;

    /* Locate the production */
    production = (Production) self -> value_top[0];

    /* Create a list and add the production to it */
    list = List_alloc();
    List_addLast(list, self -> value_top[0]);

    /* Push the list back onto the stack */
    return list;
}

/* Reduce: <production> ::= nonterm derives <exp-list> function */
static void *ProductionCreate(Parser self)
{
    Production production;
    Nonterminal nonterminal;
    char *function;
    List list;

    /* Look up the components of the production */
    nonterminal = (Nonterminal) self -> value_top[0];
    list = (List) self -> value_top[2];
    function = (char *) self -> value_top[3];

    /* Create a Production */
    production = Production_alloc(self -> production_count++, nonterminal, list, function);
    free(function);

    /* Push the production onto the stack */
    return production;
}

/* Reduce: <exp-list> ::= <exp-list> nonterm */
static void *ExpListAddNonterminal(Parser self)
{
    List list;
    Nonterminal nonterminal;

    /* Locate the list and nonterminal */
    list = (List) self -> value_top[0];
    nonterminal = (Nonterminal) self -> value_top[1];

    /* Append the nonterminal to the end of the List */
    List_addLast(list, nonterminal);

    /* Push the list back onto the stack */
    return list;
}

/* Reduce: <exp-list> ::= <exp-list> term */
static void *ExpListAddTerminal(Parser self)
{
    List list;
    Terminal terminal;

    /* Locate the list and terminal */
    list = (List) self -> value_top[0];
    terminal = (Terminal) self -> value_top[1];

    /* Append the terminal to the end of the list */
    List_addLast(list, terminal);

    /* Push the list back onto the stack */
    return list;
}

/* Reduce: <exp-list> ::= nonterm */
static void *ExpListCreateNonterminal(Parser self)
{
    List list;
    Nonterminal nonterminal;

    /* Locate the nonterminal */
    nonterminal = (Nonterminal) self -> value_top[0];

    /* Create a list and append the nonterminal to the end of it */
    list = List_alloc();
    List_addLast(list, nonterminal);

    /* Push the list onto the stack */
    return list;
}

/* Reduce: <exp-list> ::= term */
static void *ExpListCreateTerminal(Parser self)
{
    List list;
    Terminal terminal;

    /* Locate the terminal */
    terminal = (Terminal) self -> value_top[0];

    /* Create a list and append the terminal to the end of it */
    list = List_alloc();
    List_addLast(list, terminal);

    /* Push the list onto the stack */
    return list;
}



/* Locates the given Nonterminal (by name) or creates one if no such non-terminal exists */
static Nonterminal FindOrCreateNonterminal(Parser self, char *name)
{
    Nonterminal nonterminal = Hashtable_get(self -> nonterminals, name);

    if (nonterminal == NULL)
    {
	nonterminal = Nonterminal_alloc(name, self -> nonterminal_count++);
	Hashtable_put(self -> nonterminals, name, nonterminal);
    }

    return nonterminal;
}

/* Locates the given Terminal (by name) or creates one if no such terminal exists */
static Terminal FindOrCreateTerminal(Parser self, char *name)
{
    Terminal terminal = Hashtable_get(self -> terminals, name);

    if (terminal == NULL)
    {
	terminal = Terminal_alloc(name, self -> terminal_count++);
	Hashtable_put(self -> terminals, name, terminal);
    }

    return terminal;
}


/*
 *
 * Exported functions
 *
 */

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

    self -> top = self -> stack;
    *(self -> top) = 0;
    self -> value_top = self -> value_stack;
    self -> callback = callback;
    self -> context = context;
    self -> production_count = 0;
    self -> nonterminal_count = 0;
    self -> nonterminals = Hashtable_alloc(101);
    self -> terminal_count = 0;
    self -> terminals = Hashtable_alloc(101);

    /* Use an invalid terminal name to indicate end-of-file */
    FindOrCreateTerminal(self, "<EOF>");
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
    ShiftReduce(self, TT_derives, NULL);
}

/* Updates the receiver's state based on the next non-terminal token */
void Parser_acceptNonterminal(Parser self, char *value)
{
    Nonterminal nonterminal = FindOrCreateNonterminal(self, value);
    ShiftReduce(self, TT_nonterm, nonterminal);
}

/* Updates the receiver's state based on the next terminal token */
void Parser_acceptTerminal(Parser self, char *value)
{
    Terminal terminal = FindOrCreateTerminal(self, value);
    ShiftReduce(self, TT_term, terminal);
}

/* Updates the receiver's state based on the next stop token */
void Parser_acceptFunction(Parser self, char *value)
{
    ShiftReduce(self, TT_function, strdup(value));
}

/* Updates the receiver's state based on the end of input */
void Parser_acceptEOF(Parser self)
{
    ShiftReduce(self, TT_EOF, NULL);
}
