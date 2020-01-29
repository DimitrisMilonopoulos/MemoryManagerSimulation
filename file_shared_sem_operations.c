#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include "file_shared_sem_operations.h"

int create_sem(char * filename, int semnumber,int ftok_val){
    key_t sem_key;
    int sem_fd;
    int sem_id;
    sem_key = ftok("main.c",ftok_val);
    //write the key to a file for the child processes to obtain it
    sem_fd = open(filename,O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, 0664);
    if (sem_fd < 0){
        perror("Could not open sem.key\n");
        exit(1);
    }

    //Write the key in the file
    if (write(sem_fd, &sem_key, sizeof(key_t))<0){
        perror("Could not write key to file");
        exit(2);
    }
    //Done with the key
    close(sem_fd);
    //create the semaphore
    sem_id = semget(sem_key,semnumber, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id < 0){
        perror("Could not create sem");
        unlink("sem.key");
        exit(3);
    }

    return sem_id;
}

int recover_sem(char* filename){
    int sem_fd;
    key_t sem_key;
    int sem_id;
    //retrieve the semaphore key
    sem_fd = open(filename, O_RDONLY);
    if (sem_fd<0){
        perror("Could not open sem key for reading");
        exit(1);
    }

    if(read(sem_fd,&sem_key, sizeof(key_t)) !=sizeof(key_t)){
        perror("Error reading the semaphore key");
        exit(2);
    }
    close(sem_fd);
    //get the id of the semaphore
    sem_id = semget(sem_key,0, 0);
    if(sem_id<0){
        perror("Could not obtain semaphore");
        exit(3);
    }
    return sem_id;
}

void delete_sem(char* filename){
    int sem_fd;
    key_t sem_key;
    int sem_id;
    //retrieve the semaphore key
    sem_fd = open(filename, O_RDONLY);
    if (sem_fd<0){
        perror("Could not open sem key for reading");
        exit(1);
    }

    if(read(sem_fd,&sem_key, sizeof(key_t)) !=sizeof(key_t)){
        perror("Error reading the semaphore key");
        exit(2);
    }

    close(sem_fd);
    if (unlink(filename)<0){
        perror("Could not unlink key file");
    }
    //get the id of the semaphore
    sem_id = semget(sem_key,0,0);
    if (semctl(sem_id,0,IPC_RMID)<0){
        perror("Could not delete semaphore");
    }
}

int create_shared(char *filename, size_t size, int ftok_val){
    int segment_id;
    key_t shared_mem_key;
    shared_mem_key = ftok("main.c",ftok_val);
    int mem_fd;
    //write the key to a file for the child processes to obtain it
    mem_fd = open(filename,O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, 0664);
    if (mem_fd < 0){
        fprintf(stderr, "Could not open %s", filename);
        perror("");
        exit(1);
    }

    //Write the key in the file
    if (write(mem_fd, &shared_mem_key, sizeof(key_t))<0){
        fprintf(stderr, "Could not write key to file %s", filename);
        perror("");
        exit(2);
    }
    //Done with the key
    close(mem_fd);


    //create shared memory
    segment_id = shmget(shared_mem_key,size, IPC_CREAT |IPC_EXCL |  0666);
    if (segment_id < 0){
        perror("shmget error\n");
        exit(1);
    }

    return segment_id;
}

int recover_shared(char* filename,size_t size){
    int sem_fd;
    key_t sem_key;
    int shared_id;

    //retrieve the key
    sem_fd = open(filename, O_RDONLY);
    if (sem_fd<0){
        perror("Could not open sem key for reading");
        exit(1);
    }

    if(read(sem_fd,&sem_key, sizeof(key_t)) !=sizeof(key_t)){
        perror("Error reading the shared segment key");
        exit(2);
    }

    close(sem_fd);
    //Get the shared memory ID
    shared_id = shmget(sem_key,size,0);
    if (shared_id < 0){
        perror("Could not obtain shared memory segment");
    }

    return shared_id;
}

void delete_shared(char *filename, size_t size){
    key_t shared_key;
    int shm_fd;
    int shared_id;
    
    //retrieve the key
    shm_fd = open(filename, O_RDONLY);
    if (shm_fd<0){
        perror("Could not open sem key for reading");
        exit(1);
    }

    if(read(shm_fd,&shared_key, sizeof(key_t)) !=sizeof(key_t)){
        perror("Error reading the shared segment key");
        exit(2);
    }

    close(shm_fd);
    if (unlink(filename)<0){
        perror("Could not unlink key file");
        exit(1);
    }
    //retrieve id and delete the shared memory
    shared_id = shmget(shared_key,size,0);
    if(shmctl(shared_id,IPC_RMID,0)<0){
        perror("Deleting shared memory failed.\n");
    }
}