#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 1024

static int state_stack[STACK_SIZE];
static int *state_top = state_stack;

static void *value_stack[STACK_SIZE];
static void **value_top = value_stack;

static void *Accept()
{
    printf("<sub-exp> ::= <disjunction> [ACCEPT]\n");
    return NULL;
}

static void *CreateNestedFunctionExp()
{
    printf("<function-exp> ::= LPAREN <function-exp> RPAREN\n");
    return NULL;
}

static void *FunctionExpFromFunction()
{
    printf("<function-exp> ::= <function>\n");
    return NULL;
}

static void *CreateFunction()
{
    printf("<function> ::= ID LPAREN <arg-list> RPAREN\n");
    return NULL;
}


static void *ExtendDisjunction()
{
    printf("<disjunction> ::= <disjunction> OR <xor-exp>\n");
    return NULL;
}

static void *DisjunctionFromXorExp()
{
    printf("<disjunction> ::= <xor-exp>\n");
    return NULL;
}

static void *ExtendConjunction()
{
    printf("<conjunction> ::= <conjunction> AND <bool-exp>\n");
    return NULL;
}

static void *ExtendXorExp()
{
    printf("<xor-exp> ::= <xor-exp> XOR <conjunction>\n");
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

static void *BoolExpFromFunctionExp()
{
    printf("<bool-exp> ::= <function>\n");
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

static void *NumValueFromFunctionExp()
{
    printf("<num-value> ::= <function-exp>\n");
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
    int state;
    int action;

    /* Reduce as many times as possible */
    while (IS_REDUCE(action = sr_table[Top()][type]))
    {
	struct production *production;
	int reduction;
	void *result;

	/* Locate the production we're going to use to do the reduction */
	reduction = REDUCTION(action);
	production = productions + reduction;

	/* Print an unhelpful message */
	printf("r%d, ", reduction);
	fflush(stdout);

	/* Pop stuff off the stack */
	Pop(production -> count);

	/* Reduce */
	result = (production -> function)();

	/* Push the result of the reduction back onto the stack */
	Push(REDUCE_GOTO(Top(), production), result);
    }

    /* And then (hopefully) shift */
    if (IS_SHIFT(action))
    {
	printf("s%d\n", SHIFT_GOTO(action));
	Push(SHIFT_GOTO(action), value);
	return;
    }

    /* Accept if we can */
    if (IS_ACCEPT(action))
    {
	/* Pop the result off of the stack */
	Pop(1);

	/* Accept the result */
	Accept();
	return;
    }

    /* Otherwise it must be an error */
    fprintf(stderr, "*** ERROR (state=%d, type=%d)\n", state, type);
    exit(0);
}


int main(int argc, char *argv[])
{
    *state_top = 0;
    printf("hello sailor\n");

    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_ID, "sizeof");
    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_ID, "BOB");
    ShiftReduce(TT_RPAREN, NULL);
    ShiftReduce(TT_RPAREN, NULL);
    ShiftReduce(TT_EQ, NULL);
    ShiftReduce(TT_ID, "sizeof");
    ShiftReduce(TT_LPAREN, NULL);
    ShiftReduce(TT_ID, "DAVE");
    ShiftReduce(TT_RPAREN, NULL);

    ShiftReduce(TT_EOF, NULL);
    printf("sizeof(short)=%d\n", sizeof(short));
    return 0;
}
