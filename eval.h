#include "mpc.h"

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

typedef struct {
	  int type;
	  long num;
	      int err;
} lval;

long eval_op(long x, char* op, long y);
long eval(mpc_ast_t* t);

