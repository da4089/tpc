#include <stdio.h>
#include <stdlib.h>

#include "e4.h"

#define STACK_SIZE 256

static int *state_top;
static int state_stack[STACK_SIZE];

static void **token_top;
static void *token_stack[STACK_SIZE];


/* Push a state and token onto the stack */
static void Push(int state, void *tdata)
{
    /* Push the state */
    state_top++;
    *state_top = state;

    /* Push the token */
    *token_top = tdata;
    token_top++;
}

static int Top()
{
    printf("top: %d\n", *state_top);
    return *state_top;
}

static void Accept()
{
    printf("accept\n");
    exit(0);
}

static void Reduce(int reduction)
{
    printf("REDUCE (reduction=%d)\n", reduction);
/*    REDUCE_GOTO(Top(), reduction);*/
    exit(0);
}

static void Error(int ttype, void *tdata)
{
    printf("error\n");
}


static void ShiftReduce(terminal_t ttype, void *tdata)
{
    while (1)
    {
	int state = Top();
	int action = sr_table[state][ttype];

	/* Watch for errors */
	if (IS_ERROR(action))
	{
	    fprintf(stderr, "*** ERROR\n");
	    exit(0);
	}

	/* Shift if we can */
	if (IS_SHIFT(action))
	{
	    printf("shift \"%s\"\n", tdata);
	    Push(SHIFT_GOTO(action), tdata);
	    return;
	}

	/* Otherwise keep reducing */
	Reduce(REDUCTION(action));
    }
}

int main(int argc, char *argv)
{
    char *tdata = "tdata";

    printf("hello sailor\n");

    /* Initialize the stack with only the 0 state */
    state_top = state_stack;
    *state_top = 0;
    token_top = token_stack;

    ShiftReduce(TT_EQ, "==");
    ShiftReduce(TT_EQ, "==");
    ShiftReduce(TT_EQ, "==");
}
