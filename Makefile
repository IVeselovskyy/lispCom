
parsing: parsing.c mpc.c eval.o lenv.o
	gcc -std=c99 -Wall parsing.c eval.o lenv.o mpc.c -ledit -lm -o parsing
%.o : %.c mpc.h eval.h lenv.h
	gcc -Wall -g -c $<


