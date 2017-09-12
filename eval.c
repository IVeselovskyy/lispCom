#include <string.h>
#include <math.h>
#include "eval.h"

int zero_check(lval *y) {
    if(y->num == 0.0) {
        return 1;
    }
    return 0;
}
/* Create a new number type lval */
lval *lval_num(double x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Create a new number type lval */
lval *lval_double(double x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_DOUBLE;
    v->num = x;
    return v;
}


/* Create a new error type lval */
lval *lval_err(char *m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;}

lval *lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/* A pointer to a new empty Sexpr lval */
lval *lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* A pointer to a new empty Qexpr lval */
lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {

    switch (v->type) {
        /* Do nothing special for number type */
        case LVAL_NUM: break;

            /* For Err or Sym free the string data */
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

            /* If Sexpr or Qexpr then delete all elements inside */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            /* Also free the memory allocated to contain the pointers */
            free(v->cell);
            break;
    }

    /* Free the memory allocated for the "lval" struct itself */
    free(v);
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    double x;
    // TODO: check what happens if the int is too large


    if(strchr(t->contents, '.')) {
        x = strtod(t->contents, NULL);
        printf("%lf double\n", x);
        return errno != ERANGE ? lval_double(x) : lval_err("Number is too large");

    } else {
        x = strtol(t->contents, NULL, 10);
        printf("%lf not double\n", x);
        return errno != ERANGE ? lval_num(x) : lval_err("Number is too large");
    }
}

lval* lval_read(mpc_ast_t* t) {

    /* If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* If root (>) or sexpr then create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }

        if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {

        /* Print Value contained within */
        lval_print(v->cell[i]);

        /* Don't print trailing space if last element */
        if (i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:      printf("%ld", (long)v->num); break;
        case LVAL_DOUBLE:   printf("%lf", v->num); break;
        case LVAL_ERR:      printf("Error: %s", v->err); break;
        case LVAL_SYM:      printf("%s", v->sym); break;
        case LVAL_SEXPR:    lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR:    lval_expr_print(v, '{', '}'); break;
    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* lval_eval(lval* v) {
    /* Evaluate Sexpressions */
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
    /* All other lval types remain the same */
    return v;
}

lval* lval_eval_sexpr(lval* v) {

    /* Evaluate Children */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    /* Error Checking */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    /* Empty Expression */
    if (v->count == 0) { return v; }

    /* Single Expression */
    if (v->count == 1) { return lval_take(v, 0); }

    /* Ensure First Element is Symbol */
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression Does not start with symbol!");
    }

    /* Call builtin with operator */
    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_pop(lval* v, int i) {
    /* Find the item at "i" */
    lval* x = v->cell[i];

    /* Shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i+1],
            sizeof(lval*) * (v->count-i-1));

    /* Decrease the count of items in the list */
    v->count--;

    /* Reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}



/* Use operator string to see which operation to perform */
lval* builtin_op(lval* a, char* op) {

    /* Ensure all arguments are numbers */
    for (int i = 0; i < a->count; i++) {
        if ((a->cell[i]->type != LVAL_NUM) && (a->cell[i]->type != LVAL_DOUBLE)) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }

    /* Pop the first element */
    lval* x = lval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    // TODO: Make it so that x type changes if it's NUM, but y is double
    /* While there are still elements remaining */
    while (a->count > 0) {
        lval *y = lval_pop(a, 0);

        
        if (strcmp(op, "+") == 0) { x->num += y->num; }

        if (strcmp(op, "-") == 0) { x->num -= y->num; }

        if (strcmp(op, "*") == 0) { x->num *= y->num; }

        if (strcmp(op, "^") == 0) { x->num = pow(x->num, y->num); }

        if (strcmp(op, "%") == 0) {
            if ((y->type == LVAL_DOUBLE) || (x->type == LVAL_DOUBLE)) {
                x = lval_err("Can't use double");
            }
            if (zero_check(y) == 1) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Dividing by zero");
            }
            x->num = (int) x->num % (int) y->num;
        }

        if (strcmp(op, "/") == 0) {
            if (zero_check(y) == 1) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Dividing by zero");
            }
            x->num = x->num / y->num;
        }

        if (strcmp(op, "//") == 0) {
            if ((y->type == LVAL_DOUBLE) || (x->type == LVAL_DOUBLE)) {
                x = lval_err("Can't use double");
            }
            if (zero_check(y) == 1) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Dividing by zero");
            }
            x->num = (int) x->num / (int) y->num;
        }
        lval_del(y);
    }
    lval_del(a);
    return x;

}

lval *builtin_head(lval *a) {
    LASSERT(a, a->count == 1,
            "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'head' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,
            "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}

lval *builtin_tail(lval *a) {
    LASSERT(a, a->count == 1,
            "Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,
            "Function 'tail' passed {}!");

    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

lval *builtin_list(lval *a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_eval(lval *a) {
    LASSERT(a, a->count == 1,
            "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval *lval_join(lval *x, lval *y) {

    /* For each cell in 'y' add it to 'x' */
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }

    /* Delete the empty 'y' and return 'x' */
    lval_del(y);
    return x;
}

lval *builtin_join(lval *a) {

    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
                "Function 'join' passed incorrect type.");
    }

    lval* x = lval_pop(a, 0);

    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

lval *builtin(lval *a, char *func) {
    if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strstr("+-/*", func)) { return builtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}


//lval eval(mpc_ast_t* t) {
//    /* If tagged as number return it directly-> */
//    if (strstr(t->tag, "number")) {
//        errno = 0;
//        double x;
//        // TODO: check what happens if the int is too large
//
//
//        if(strchr(t->contents, '.')) {
//            x = strtod(t->contents, NULL);
//            printf("%fl double\n", x);
//            return errno != ERANGE ? lval_double(x) : lval_err("Number is too large");
//
//        } else {
//            x = strtol(t->contents, NULL, 10);
//            printf("%fl not double\n", x);
//            return errno != ERANGE ? lval_num(x) : lval_err("Number is too large");
//        }
//    }
//
//    /* The operator is always second child. */
//    char* op = t->children[1]->contents;
//
//    /* We store the third child in `x` */
//    lval x = eval(t->children[2]);
//
//    if ((t->children_num == 4) && (strcmp(op, "-") == 0)) {
//        return lval_num(-x->num);
//    }
//    /* Iterate the remaining children and combining. */
//    int i = 3;
//    while (strstr(t->children[i]->tag, "expr")) {
//
//        // Added functionality to detect nested errors
//        lval second_op = eval(t->children[i]);
//        if(second_op.type == LVAL_ERR) {
//            return second_op;
//        }
//        x = eval_op(x, op, second_op);
//        i++;
//    }
//
//    return x;
//}
