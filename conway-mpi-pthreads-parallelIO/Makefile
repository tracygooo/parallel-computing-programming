all: clcg4.h clcg4.c main.c
	gcc -I. -Wall -O3 -c clcg4.c -o clcg4.o
	mpicc -I. -Wall -O3 main.c clcg4.o -o main -lpthread
	#mpicc -I. -Wall -O3 assignment4-5.c clcg4.o -o assignment4-5 -lpthread
