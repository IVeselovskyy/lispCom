#include "mpc.h"

typedef struct lval lval;
typedef struct lenv {
    int count;
    char** syms;
    lval** vals;
} lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);


lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

