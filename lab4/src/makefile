CC=gcc
CFLAGS=-I.

new: parallel_min_max zombie_process process_memory parallel_sum

parallel_min_max: parallel_min_max.c utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

zombie_process: zombie_process.c
	$(CC) -o zombie_process zombie_process.c $(CFLAGS)

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

utils.o: utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o: utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

sum.o: sum.c sum.h
	$(CC) -c sum.c $(CFLAGS)

parallel_sum.o: parallel_sum.c sum.h utils.h
	$(CC) -c parallel_sum.c $(CFLAGS)

parallel_sum: parallel_sum.o sum.o utils.o
	$(CC) -o parallel_sum parallel_sum.o sum.o utils.o $(CFLAGS) -pthread

clean:
	rm -f utils.o find_min_max.o parallel_min_max zombie_process process_memory parallel_sum.o sum.o parallel_sum
