#define IS_ERROR(action) ((action) == 0)
#define IS_ACCEPT(action) ((action) == 28)
#define IS_REDUCE(action) (0 < (action) && (action) < 11)
#define IS_SHIFT(action) (11 <= (action) && (action) < 28)
#define REDUCTION(action) (action)
#define REDUCE_GOTO(state, production) \
    (goto_table[state][production -> nonterm_type])
#define SHIFT_GOTO(action) ((action) - 11)

typedef enum
{
    TT_EOF = 0,
    TT_DERIVES,
    TT_ID,
    TT_STRING,
    TT_LBRACKET,
    TT_RBRACKET,
} terminal_t;

struct production
{
    void *(*function)();
    int nonterm_type;
    int count;
};

static struct production productions[11] =
{
    /* 0: <grammar> ::= <production-list> */
    { accept_grammar, 0, 1 },

    /* 1: <production-list> ::= <production-list> <production> */
    { extend_production_list, 1, 2 },

    /* 2: <production-list> ::= <production> */
    { make_production_list, 1, 1 },

    /* 3: <production> ::= <nonterminal> DERIVES <exp-list> <function> */
    { make_production, 2, 4 },

    /* 4: <exp-list> ::= <exp-list> <nonterminal> */
    { add_component, 4, 2 },

    /* 5: <exp-list> ::= <exp-list> <terminal> */
    { add_component, 4, 2 },

    /* 6: <exp-list> ::= <nonterminal> */
    { make_exp_list, 4, 1 },

    /* 7: <exp-list> ::= <terminal> */
    { make_exp_list, 4, 1 },

    /* 8: <nonterminal> ::= ID */
    { make_nonterminal, 3, 1 },

    /* 9: <terminal> ::= STRING */
    { make_terminal, 6, 1 },

    /* 10: <function> ::= LBRACKET ID RBRACKET */
    { make_function, 5, 3 }
};

#define ERR 0
#define ACC 28
#define R(x) (x)
#define S(x) (x + 11)

static unsigned char sr_table[17][6] =
{
    { ERR, ERR, S(4), ERR, ERR, ERR },
    { ACC, ERR, S(4), ERR, ERR, ERR },
    { R(2), ERR, R(2), ERR, ERR, ERR },
    { ERR, S(6), ERR, ERR, ERR, ERR },
    { ERR, R(8), R(8), R(8), R(8), ERR },
    { R(1), ERR, R(1), ERR, ERR, ERR },
    { ERR, ERR, S(4), S(10), ERR, ERR },
    { ERR, ERR, R(6), R(6), R(6), ERR },
    { ERR, ERR, S(4), S(10), S(14), ERR },
    { ERR, ERR, R(7), R(7), R(7), ERR },
    { ERR, ERR, R(9), R(9), R(9), ERR },
    { ERR, ERR, R(4), R(4), R(4), ERR },
    { R(3), ERR, R(3), ERR, ERR, ERR },
    { ERR, ERR, R(5), R(5), R(5), ERR },
    { ERR, ERR, S(15), ERR, ERR, ERR },
    { ERR, ERR, ERR, ERR, ERR, S(16) },
    { R(10), ERR, R(10), ERR, ERR, ERR },
};

#undef ERR
#undef R
#undef S

static unsigned char goto_table[17][7] = 
{
    { 0, 1, 2, 3, 0, 0, 0 },
    { 0, 0, 5, 3, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 7, 8, 0, 9 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 11, 0, 12, 13 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
};

