
parsing: parsing.c mpc.c eval.o
	gcc -std=c99 -Wall parsing.c eval.o mpc.c -ledit -lm -o parsing
%.o : %.c mpc.h eval.h
	gcc -Wall -g -c $<


