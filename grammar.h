#define IS_ERROR(action) ((action) == 0)
#define IS_REDUCE(action) ((action) < 8)
#define IS_SHIFT(action) (! IS_REDUCE(action))
#define REDUCTION(action) (action)
#define REDUCE_GOTO(state, action) \
    (goto_table[state][production_type[REDUCTION(action)]])
#define SHIFT_GOTO(action) ((action) - 8)

typedef enum
{
    TT_EOF = 0,
    TT_nonterm,
    TT_derives,
    TT_function,
    TT_term,
} terminal_t;

static void (*productions[8])() =
{
    Accept,
    ProductionListExtend,
    ProductionListCreate,
    ProductionCreate,
    ExpListAddNonterminal,
    ExpListAddTerminal,
    ExpListCreateNonterminal,
    ExpListCreateTerminal,
};

static int production_type[8] =
{
    0, /* <START> ::= <production-list> */
    1, /* <production-list> ::= <production-list> <production> */
    1, /* <production-list> ::= <production> */
    2, /* <production> ::= nonterm derives <exp-list> function */
    3, /* <exp-list> ::= <exp-list> nonterm */
    3, /* <exp-list> ::= <exp-list> term */
    3, /* <exp-list> ::= nonterm */
    3, /* <exp-list> ::= term */
};

#define ERR 0
#define R(x) (x)
#define S(x) (x + 8)

static int sr_table[12][5] =
{
    { ERR, S(3), ERR, ERR, ERR },
    { ERR, S(3), ERR, ERR, ERR },
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

static int goto_table[12][4] = 
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

