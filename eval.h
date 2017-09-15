#include "mpc.h"
#include "lenv.h"


/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_DOUBLE, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN };

typedef struct lval {
    char type;
    double num;
    char *err;
    char *sym;
    int count;
    struct lval **cell;
    lbuiltin fun;
} lval;

typedef lval*(*lbuiltin)(lenv*, lval*);



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
lval* lval_eval(lenv* e, lval* v);
lval *lval_eval_sexpr(lenv* e, lval* v);
lval *lval_pop(lval *, int);
lval *lval_take(lval *, int);
lval* builtin_op(lenv* e, lval* inner, char* op);
lval *builtin_tail(lval *);
lval *builtin_head(lval *);
lval *builtin_eval(lenv* e, lval *a);
lval *builtin_list(lval *);
lval *lval_join(lval *, lval *);
lval *builtin_join(lval *);
lval *builtin(lval *, char *);
lval* lval_fun(lbuiltin func);
lval* lval_copy(lval* v);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);



#define LASSERT(args, cond, err) \
  if (!(cond)) { lval_del(args); return lval_err(err); }
