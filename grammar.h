#define IS_ERROR(action) ((action) == 0)
#define IS_ACCEPT(action) ((action) == 20)
#define IS_REDUCE(action) (0 < (action) && (action) < 8)
#define IS_SHIFT(action) (8 <= (action) && (action) < 20)
#define REDUCTION(action) (action)
#define REDUCE_GOTO(state, production) \
    (goto_table[state][production -> nonterm_type])
#define SHIFT_GOTO(action) ((action) - 8)

typedef enum
{
    TT_EOF = 0,
    TT_nonterm,
    TT_derives,
    TT_function,
    TT_term,
} terminal_t;

struct production
{
    void *(*function)();
    int nonterm_type;
    int count;
};

static struct production productions[8] =
{
    /* <grammar> ::= <production-list> */
    { Accept, 0, 1 },

    /* <production-list> ::= <production-list> <production> */
    { ProductionListExtend, 1, 2 },

    /* <production-list> ::= <production> */
    { ProductionListCreate, 1, 1 },

    /* <production> ::= nonterm derives <exp-list> function */
    { ProductionCreate, 2, 4 },

    /* <exp-list> ::= <exp-list> nonterm */
    { ExpListAddNonterminal, 3, 2 },

    /* <exp-list> ::= <exp-list> term */
    { ExpListAddTerminal, 3, 2 },

    /* <exp-list> ::= nonterm */
    { ExpListCreateNonterminal, 3, 1 },

    /* <exp-list> ::= term */
    { ExpListCreateTerminal, 3, 1 }
};

#define ERR 0
#define ACC 20
#define R(x) (x)
#define S(x) (x + 8)

static unsigned char sr_table[12][5] =
{
    { ERR, S(3), ERR, ERR, ERR },
    { ACC, S(3), ERR, ERR, ERR },
    { R(2), R(2), ERR, ERR, ERR },
    { ERR, ERR, S(5), ERR, ERR },
    { R(1), R(1), ERR, ERR, ERR },
    { ERR, S(7), ERR, ERR, S(8) },
    { ERR, S(9), ERR, S(10), S(11) },
    { ERR, R(6), ERR, R(6), R(6) },
    { ERR, R(7), ERR, R(7), R(7) },
    { ERR, R(4), ERR, R(4), R(4) },
    { R(3), R(3), ERR, ERR, ERR },
    { ERR, R(5), ERR, R(5), R(5) },
};

#undef ERR
#undef R
#undef S

static unsigned char goto_table[12][4] = 
{
    { 0, 1, 2, 0 },
    { 0, 0, 4, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 6 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

