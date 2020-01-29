#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "sem_operations.h"

//Functions based on the ones given in the Lab

//Arguments: 1. semaphore id 2.semaphore number 3.value desired to be set
int set_semvalue(int sem_id,int sem_num,int sem_val)
{
    union semun sem_union;
    sem_union.val = sem_val;
        if (semctl(sem_id, sem_num, SETVAL, sem_union) == -1) return(0);
    return(1);
}

void del_semvalue(int sem_id,int nsems)
{
	union semun sem_union;
    for (int i =0; i<nsems;i++){
	    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
            fprintf(stderr, "Failed to delete semaphore\n");
    }
}

//Arguments: 1. semaphore id 2. semaphore number
int semaphore_p(int sem_id,int sem_num)
{
	struct sembuf sem_b;
	sem_b.sem_num = sem_num;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = 0;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

//Arguments: 1. semaphore id 2. semaphore number
int semaphore_v(int sem_id,int sem_num)
{
	struct sembuf sem_b;
	sem_b.sem_num = sem_num;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = 0;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}
