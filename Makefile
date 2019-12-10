myshell: myshell.o input.o
	cc -o myshell myshell.o input.o

myshell.o: myshell.c input.h
	cc -c myshell.c

input.o: input.c input.h
	cc -c input.c

clean: 
	rm myshell myshell.o input.o
