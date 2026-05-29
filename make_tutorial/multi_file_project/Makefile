helloMake : main.o hello.o
	gcc -o helloMake main.o hello.o

main.o: main.c hello.h
	gcc -c main.c

hello.o: hello.c hello.h
	gcc -c hello.c 
