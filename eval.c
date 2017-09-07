#include <string.h>
#include <math.h>
#include "eval.h"

int zero_check(lval y) {
    if(y.num == 0.0) {
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

void lval_del(lval* v) {

    switch (v->type) {
        /* Do nothing special for number type */
        case LVAL_NUM: break;

            /* For Err or Sym free the string data */
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

            /* If Sexpr then delete all elements inside */
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
        printf("%fl double\n", x);
        return errno != ERANGE ? lval_double(x) : lval_err("Number is too large");

    } else {
        x = strtol(t->contents, NULL, 10);
        printf("%fl not double\n", x);
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

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
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
        case LVAL_NUM:      printf("%dl", (int)v->num); break;
        case LVAL_DOUBLE:   printf("%fl", v->num); break;
        case LVAL_ERR:      printf("Error: %s", v->err); break;
        case LVAL_SYM:      printf("%s", v->sym); break;
        case LVAL_SEXPR:    lval_expr_print(v, '(', ')'); break;
    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }


/* Use operator string to see which operation to perform */
lval eval_op(lval x, char* op, lval y) {
    //printf("%g. %g \n", x.num, y.num);

    if (strcmp(op, "+") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_double( x.num + y.num);
        }
        return lval_num( x.num + y.num);
    }

    if (strcmp(op, "-") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_double( x.num - y.num);
        }
        return lval_num(x.num - y.num); }

    if (strcmp(op, "*") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_double( x.num * y.num);
        }
        return lval_num(x.num * y.num); }

    if (strcmp(op, "/") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return zero_check(y) == 1 ? lval_err(LERR_DIV_ZERO) : lval_double(x.num / y.num);
        }
        return zero_check(y) == 1 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }

    if (strcmp(op, "^") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_double( x.num + y.num);
        }
        return lval_num(pow(x.num, y.num)); }

    if (strcmp(op, "%") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_err(LERR_BAD_NUM);
        }
        return zero_check(y) == 1 ? lval_err(LERR_DIV_ZERO) : lval_num((int)x.num % (int)y.num);
    }
    if (strcmp(op, "//") == 0) {
        if ((y.type == LVAL_DOUBLE) || (x.type == LVAL_DOUBLE)) {
            return lval_err(LERR_BAD_NUM);
        }
        return zero_check(y) == 1 ? lval_err(LERR_DIV_ZERO) : lval_num((int)x.num / (int)y.num);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number")) {
        errno = 0;
        double x;
        // TODO: check what happens if the int is too large


        if(strchr(t->contents, '.')) {
            x = strtod(t->contents, NULL);
            printf("%fl double\n", x);
            return errno != ERANGE ? lval_double(x) : lval_err(LERR_BAD_NUM);

        } else {
            x = strtol(t->contents, NULL, 10);
            printf("%fl not double\n", x);
            return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
        }
    }

    /* The operator is always second child. */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    lval x = eval(t->children[2]);

    if ((t->children_num == 4) && (strcmp(op, "-") == 0)) {
        return lval_num(-x.num);
    }
    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {

        // Added functionality to detect nested errors
        lval second_op = eval(t->children[i]);
        if(second_op.type == LVAL_ERR) {
            return second_op;
        }
        x = eval_op(x, op, second_op);
        i++;
    }

    return x;
}
