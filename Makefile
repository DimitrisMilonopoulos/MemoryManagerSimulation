default: Simulation

Simulation: main.o HashTable.o sem_operations.o file_shared_sem_operations.o Processes.o
	gcc main.o HashTable.o sem_operations.o file_shared_sem_operations.o Processes.o -o Simulation

main.o: main.c
	gcc -g -c main.c
HashTable.o: HashTable.c HashTable.h
	gcc -g -c HashTable.c
sem_operations.o: sem_operations.c sem_operations.h
	gcc -g -c sem_operations.c
file_shared_sem_operations.o: file_shared_sem_operations.c file_shared_sem_operations.h
	gcc -g -c file_shared_sem_operations.c
Processes.o: Processes.c Processes.h file_shared_sem_operations.h sem_operations.h HashTable.h
	gcc -g -c Processes.c

clean:
	rm -f *.key *.o Simulation