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
#include <sys/sem.h>
#include <time.h>

#include "file_shared_sem_operations.h"
#include "sem_operations.h"
#include "HashTable.h"
#include "Processes.h"

int main(int argc, char **argv){

    if(argc <4){
        printf("Not enough arguments!\n");
        return -1;
    }

    int k = atoi(argv[1]);
    int frames = atoi(argv[2]);
    if(k > frames/2){
        printf("Error: K is bigger than half the frames!\n");
        return -1;
    }
    int q = atoi(argv[3]);
    int ntraces;
    if (argc==5){
        ntraces = atoi(argv[4]);
    }
    else{
        ntraces = 1000000;
    }
    

    int sem_id = create_sem("sem.key",3,436); 
    int sem_id1 = create_sem("sem1.key",3,588); 

    if(!set_semvalue(sem_id,0,1)){ //available = 1
        perror("Could not set value semaphore");
        exit(4);
    }
    
    if(!set_semvalue(sem_id,1,1)){ //empty = 1
        perror("Could not set value semaphore");
        exit(4);
    }
     if(!set_semvalue(sem_id,2,0)){ //full = 0
        perror("Could not set value semaphore");
        exit(4);
    }
    
        if(!set_semvalue(sem_id1,0,1)){ //available = 1
        perror("Could not set value semaphore");
        exit(4);
    }
    
    if(!set_semvalue(sem_id1,1,1)){ //empty = 1
        perror("Could not set value semaphore");
        exit(4);
    }
     if(!set_semvalue(sem_id1,2,0)){ //full = 0
        perror("Could not set value semaphore");
        exit(4);
    }
    
    create_shared("shared.key",sizeof(struct Trace)*2*q,501);

    int nBuckets = 10;

    struct Trace trace;
    trace.pageNum = 7732;
    trace.action = 'W';

    FILE *file;
    size_t len=0;
    char *line = NULL;
    pid_t pid;

    int hex;
    char action;
    pid = fork();
    if(pid==0){
        pm(ntraces,q,0);
        exit(1);
    }

    pid = fork();
    if(pid==0){
        pm(ntraces,q,1);
        exit(1);
    }

    pid = fork();
    if(pid==0){
        mm(ntraces,q,k,frames);
        exit(1);
    }

    for(int i=0;i<3;i++){
        wait(NULL);
    }

    
    delete_shared("shared.key", sizeof(struct Trace)*2*q);
    delete_sem("sem.key");
    delete_sem("sem1.key");
}