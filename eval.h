#include "mpc.h"


/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_DOUBLE, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };


typedef struct lval {
    char type;
    double num;
    char *err;
    char *sym;
    int count;
    struct lval **cell;
} lval;

lval eval_op(lval, char *, lval);
lval eval(mpc_ast_t *);

lval *lval_num(double);
lval *lval_double(double);
lval *lval_err(char *);
lval *lval_sym(char *);
lval *lval_sexpr(void);
int zero_check(lval *);
void lval_del(lval *);
lval *lval_add(lval *, lval *);
lval *lval_read_num(mpc_ast_t *);
lval *lval_read(mpc_ast_t *);
void lval_expr_print(lval *, char, char);
void lval_print(lval *);
void lval_println(lval *);
lval *lval_eval(lval *);
lval *lval_eval_sexpr(lval *);
lval *lval_pop(lval *, int);
lval *lval_take(lval *, int);
lval *builtin_op(lval *, char *);