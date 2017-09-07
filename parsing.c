#include <editline/readline.h>
#include <editline/history.h>
#include "mpc.h"
#include "eval.h"

int main(int argc, char** argv) {
    /* Create Some Parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                \
      number   : /-?[0-9]+[.][0-9]+/  | /-?[0-9]+/ ;           \
      operator : '+' | '-' | '*' | '^' | '%' | \"//\" | '/' ;  \
      sexpr    : '(' <expr>* ')' ;                             \
      expr     : <number> | '(' <operator> <expr>+ ')' ;       \
      lispy    : /^/ <operator> <expr>+ /$/ ;                  \
    ",
              Number, Symbol, Sexpr, Expr, Lispy);

    puts("Lispy Version 0.2");
    puts("Press Ctrl+c to Exit\n");

    while (1) {

        char* input = readline("lispy> ");
        add_history(input);

        /* Attempt to parse the user input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On success print and delete the AST */
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise print and delete the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    // Unreachable

    /* Undefine and delete our parsers */
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

    return 0;
}
