#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 1024

static int state_stack[STACK_SIZE];
static int *state_top = state_stack;

static void *value_stack[STACK_SIZE];
static void **value_top = value_stack;

static void *Accept()
{
    printf("Accept!\n");
    return NULL;
}

static void *ExtendDisjunction()
{
    printf("<disjunction> ::= <disjunction> BOOL_OR <xor-exp>\n");
    return NULL;
}

static void *DisjunctionFromXorExp()
{
    printf("<disjunction> ::= <xor-exp>\n");
    return NULL;
}

static void *ExtendConjunction()
{
    printf("<conjunction> ::= <conjunction> BOOL_AND <bool-exp>\n");
    return NULL;
}

static void *XorExpFromConjunction()
{
    printf("<xor-exp> ::= <conjunction>\n");
    return NULL;
}

static void *ConjunctionFromBoolExp()
{
    printf("<conjunction> ::= <bool-exp>\n");
    return NULL;
}


static void *BoolExpFromComparison()
{
    printf("<bool-exp> ::= <value> <predicate> <value>\n");
    return NULL;
}

static void *BoolExpFromBoolFunction()
{
    printf("<bool-exp> ::= <bool-function>\n");
    return NULL;
}

static void *BoolExpFromNegation()
{
    printf("<bool-exp> ::= BANG <bool-exp>\n");
    return NULL;
}

static void *CreateNestedBoolExp()
{
    printf("<bool-exp> ::= LPAREN <disjunction> RPAREN\n");
    return NULL;
}

static void *NumValueFromValueFunction()
{
    printf("<num-value> ::= <value-function>\n");
    return NULL;
}

static void *ValueFromString()
{
    printf("<value> ::= STRING\n");
    return NULL;
}

static void *ValueFromBitDisjunction()
{
    printf("<value> ::= <bit-disjunction>\n");
    return NULL;
}

static void *PredicateFromEQ()
{
    printf("<predicate> ::= EQ\n");
    return NULL;
}

static void *PredicateFromLE()
{
    printf("<predicate> ::= LE\n");
    return NULL;
}

static void *CreatePrimaryFunction()
{
    printf("<value-function> ::= PRIMARY LPAREN <value> RPAREN\n");
    return NULL;
}

static void *BitDisjunctionFromBitXorExp()
{
    printf("<bit-disjunction> ::= <bit-xor-exp>\n");
    return NULL;
}

static void *BitXorExpFromBitConjunction()
{
    printf("<bit-xor-exp> ::= <bit-conjunction>\n");
    return NULL;
}

static void *BitConjunctionFromBitShiftExp()
{
    printf("<bit-conjunction> ::= <bit-shift-exp>\n");
    return NULL;
}

static void *BitShiftExpFromSum()
{
    printf("<bit-shift-exp> ::= <sum>\n");
    return NULL;
}

static void *SumFromPlusExp()
{
    printf("<sum> ::= <sum> PLUS <product>\n");
    return NULL;
}

static void *SumFromMinusExp()
{
    printf("<sum> ::= <sum> MINUS <product>\n");
    return NULL;
}

static void *SumFromProduct()
{
    printf("<sum> ::= <product>\n");
    return NULL;
}

static void *ProductFromTimesExp()
{
    printf("<product> ::= <product> TIMES <num-exp>\n");
    return NULL;
}

static void *ProductFromModExp()
{
    printf("<product> ::= <product> MOD <num-exp>\n");
    return NULL;
}

static void *ProductFromNumExp()
{
    printf("<product> ::= <num-exp>\n");
    return NULL;
}

static void *NumExpFromNumValue()
{
    printf("<num-exp> ::= <num-value>\n");
    return NULL;
}

static void *NumValueFromINT32()
{
    printf("<num-value> ::= INT32\n");
    return NULL;
}

static void *NumValueFromName()
{
    printf("<num-value> ::= <name>\n");
    return NULL;
}

static void *CreateNestedNumValue()
{
    printf("<num-value> ::= LPAREN <value> RPAREN\n");
    return NULL;
}

static void *NameFromId()
{
    printf("<name> ::= ID\n");
    return NULL;
}

static void *CreateExistsFunction()
{
    printf("<bool-function> ::= EXISTS LPAREN <name> RPAREN\n");
    return NULL;
}

static void *CreateIsInt32Function()
{
    printf("<bool-function> ::= IS_INT32 LPAREN <name> RPAREN\n");
    return NULL;
}

static void *CreateRegexFunction()
{
    printf("<bool-function> ::= REGEX LPAREN <name> COMMA <arg-list> RPAREN\n");
    return NULL;
}

static void *CreateDecomposeFunction()
{
    printf("<value-function> ::= DECOMPOSE LPAREN <value> RPAREN\n");
    return NULL;
}

static void *CreateArgList()
{
    printf("<arg-list> ::= <value>\n");
    return NULL;
}


static void *Function()
{
    return NULL;
}


#include "e4new.h"

/* Push a state and value onto the stack */
static void Push(int state, void *value)
{
    *(++state_top) = state;
    *(value_top++) = value;
}

/* Pop states and values off the stack */
static void Pop(int count)
{
    state_top -= count;
    value_top -= count;
}

/* Answers the top state on the stack */
static int Top()
{
    return *state_top;
}


static void ShiftReduce(terminal_t type, void *value)
{
    while (1)
    {
	int state = Top();
	int action = sr_table[state][type];
	struct production *production;
	int reduction;
	void *result;

	/* Watch for errors */
	if (IS_ERROR(action))
	{
	    fprintf(stderr, "*** ERROR (state=%d, type=%d)\n", state, type);
	    exit(0);
	}

	/* Accept if we can */
	if (IS_ACCEPT(action))
	{
	    Accept();
	    return;
	}

	/* Shift if we can */
	if (IS_SHIFT(action))
	{
	    printf("s%d\n", SHIFT_GOTO(action));
	    Push(SHIFT_GOTO(action), value);
	    return;
	}

	/* Locate the production we're going to use to reduce */
	reduction = REDUCTION(action);
	production = &productions[reduction];

	printf("r%d, ", reduction);
	fflush(stdout);

	/* Pop stuff off of the stack */
	Pop(production -> count);

	/* Reduce */
	result = (production -> function)();

	/* Push the result of the reduction back onto the stack */
	Push(REDUCE_GOTO(Top(), production), result);
    }
}


int main(int argc, char *argv[])
{
    *state_top = 0;
    printf("hello sailor\n");

    ShiftReduce(TT_EXISTS, NULL);
    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_RPAREN, NULL);
    ShiftReduce(TT_BOOL_OR, NULL);
    ShiftReduce(TT_BANG, NULL);
    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_REGEX, NULL);
    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_COMMA, NULL);
    ShiftReduce(TT_STRING, ".*[Mm]ail.*");
    ShiftReduce(TT_RPAREN, NULL);
    ShiftReduce(TT_BOOL_OR, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_EQ, NULL);
    ShiftReduce(TT_STRING, "Rakoto");
    ShiftReduce(TT_BOOL_OR, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_EQ, NULL);
    ShiftReduce(TT_STRING, "lawley-rcvstore");
    ShiftReduce(TT_BOOL_OR, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_EQ, NULL);
    ShiftReduce(TT_STRING, "weather");
    ShiftReduce(TT_BOOL_OR, NULL);
    ShiftReduce(TT_ID, "TICKERTAPE");
    ShiftReduce(TT_EQ, NULL);
    ShiftReduce(TT_STRING, "fourskins");
    ShiftReduce(TT_RPAREN, NULL);

    ShiftReduce(TT_EOF, NULL);
    return 0;
}
