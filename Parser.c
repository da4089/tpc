/* $Id: Parser.c,v 1.10 1999/02/16 09:32:30 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
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

/* The types of terminals */
typedef enum
{
    DerivesValue,
    NonterminalValue,
    TerminalValue,
    StopValue
} TerminalType;

/* The types of non-terminals */
typedef enum
{
    ProductionValue,
    ListValue
} NonterminalType;


/* The ReduceFunction type */
typedef void (*ReduceFunction)(Parser self);


/*
 *
 * Static function headers
 *
 */
static void Push(Parser self, int state, void *value);
static void Pop(Parser self, int count);
static int Top(Parser self);

static void ShiftReduce(Parser self, TerminalType type, void *value);

/* Production reducers */
static void Accept(Parser self);
static void ProductionListExtend(Parser self);
static void ProductionListCreate(Parser self);
static void ProductionCreate(Parser self);
static void ExpListAddNonterminal(Parser self);
static void ExpListAddTerminal(Parser self);
static void ExpListCreateNonterminal(Parser self);
static void ExpListCreateTerminal(Parser self);

#include "grammar.h"

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

    /* The receiver's List of Productions */
    List productions;
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


/* Perform all possible reductions and finally shift in the terminal */
static void ShiftReduce(Parser self, terminal_t type, void *value)
{
    while (1)
    {
	int state = Top(self);
	int action = sr_table[state][type];

	/* Watch for errors */
	if (IS_ERROR(action))
	{
	    fprintf(stderr, "*** ERROR\n");
	    exit(0);
	}

	/* Shift if we can */
	if (IS_SHIFT(action))
	{
	    Push(self, SHIFT_GOTO(action), value);
	    return;
	}

	/* Otherwise reduce and repeat */
	productions[REDUCTION(action)](self);
    }
}





#if 0
{
    int oldState = Top(self);
    int state = shiftTable[oldState][type];
    ReduceFunction function;

    /* If state is zero, then we've encountered an error */
    if (state == 0)
    {
	fprintf(stderr, "*** eek!  error (state=%d, type=%d)\n", oldState, type);
	exit(1);
    }

    /* Push the state onto the stack */
    Push(self, state, value);

    /* Reduce as much as possible */
    while ((function = reduceTable[Top(self)]) != NULL)
    {
	function(self);
    }
}
#endif

/* Reduce: <START> ::= <production-list> */
static void Accept(Parser self)
{
    printf("ACCEPT\n");
}

/* Reduce: <production-list> ::= <production-list> <production> */
static void ProductionListExtend(Parser self)
{
    Pop(self, 1);
    List_addLast(self -> productions, *(self -> value_top));
}

/* Reduce: <production-list> ::= <production> */
static void ProductionListCreate(Parser self)
{
    
}


/* Reduction 1: <production> ::= nonterm derives <exp-list> stop */
static void Reduce1(Parser self)
{
    Production production;
    Nonterminal nonterminal;
    List list;

    /* Pop the old states */
    Pop(self, 4);

    /* FIX THIS: should do something with the stuff on the stack */
    nonterminal = (Nonterminal) self -> value_top[0];
    list = (List) self -> value_top[2];

    /* Create a Production from the Nonterminal and List of components */
    production = Production_alloc(nonterminal, list, self -> production_count++);

    /* Go to the new state */
    Push(self, gotoTable[Top(self)][ProductionValue], production);
}

/* Reduction 2: <exp-list> ::= <exp-list> nonterm */
static void Reduce2(Parser self)
{
    List list;
    Nonterminal nonterminal;

    /* Pop the old states */
    Pop(self, 2);

    /* Add the nonterminal to the end of the list */
    list = self -> value_top[0];
    nonterminal = self -> value_top[1];
    List_addLast(list, nonterminal);

    /* Go to the new state */
    Push(self, gotoTable[Top(self)][ListValue], list);
}

/* Reduction 3: <exp-list> ::= <exp-list> term */
static void Reduce3(Parser self)
{
    List list;
    Terminal terminal;

    /* Pop the old states */
    Pop(self, 2);

    /* Add the terminal to the end of the list */
    list = (List) self -> value_top[0];
    terminal = (Terminal) self -> value_top[1];
    List_addLast(list, terminal);

    /* Go to the new state */
    Push(self, gotoTable[Top(self)][ListValue], list);
}

/* Reduction 4: <exp-list> ::= nonterm */
static void Reduce4(Parser self)
{
    Nonterminal nonterminal;
    List list;

    /* Pop the old states */
    Pop(self, 1);

    /* Create a list containing the nonterminal */
    nonterminal = self -> value_top[0];
    list = List_alloc();
    List_addLast(list, nonterminal);

    /* Go to the new state */
    Push(self, gotoTable[Top(self)][ListValue], list);
}

/* Reduction 5: <exp-list> ::= term */
static void Reduce5(Parser self)
{
    Terminal terminal;
    List list;

    /* Pop the old states */
    Pop(self, 1);

    /* Create a list containing the terminal */
    terminal = self -> value_top[0];
    list = List_alloc();
    List_addLast(list, terminal);

    /* Go to the new state */
    Push(self, gotoTable[Top(self)][ListValue], list);
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
    self -> productions = List_alloc();

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
    ShiftReduce(self, DerivesValue, NULL);
}

/* Updates the receiver's state based on the next non-terminal token */
void Parser_acceptNonterminal(Parser self, char *value)
{
    Nonterminal nonterminal = FindOrCreateNonterminal(self, value);
    ShiftReduce(self, NonterminalValue, nonterminal);
}

/* Updates the receiver's state based on the next terminal token */
void Parser_acceptTerminal(Parser self, char *value)
{
    Terminal terminal = FindOrCreateTerminal(self, value);
    ShiftReduce(self, TerminalValue, terminal);
}

/* Updates the receiver's state based on the next stop token */
void Parser_acceptStop(Parser self)
{
    ShiftReduce(self, StopValue, NULL);
}

/* Updates the receiver's state based on the end of input */
void Parser_acceptEOF(Parser self)
{
    Grammar grammar;
    Nonterminal *nonterminals;
    Terminal *terminals;

    /* Copy the nonterminals out of the Hashtable and into the array */
    nonterminals=(Nonterminal *) calloc(Hashtable_size(self -> nonterminals), sizeof(Nonterminal));
    Hashtable_keysAndValuesDoWith(self -> nonterminals, DecodeNonterminal, nonterminals);
    terminals = (Terminal *) calloc(Hashtable_size(self -> terminals) + 1, sizeof(Terminal));
    Hashtable_keysAndValuesDoWith(self -> terminals, DecodeTerminal, terminals);

    grammar = Grammar_alloc(
	self -> productions,
	self -> nonterminal_count,
	nonterminals,
	self -> terminal_count,
	terminals);

    /* Call the callback if there is one */
    if (self -> callback != NULL)
    {
	(*self -> callback)(self -> context, grammar);
    }
}

