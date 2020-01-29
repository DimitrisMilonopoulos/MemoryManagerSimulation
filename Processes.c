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

int pm(int ntraces, int q, int pnum){
    FILE *file;
    size_t len=0;
    char *line=NULL;
    unsigned int hex;
    char action;
    int sem_id;
    int shm_id;
    int offset;
    struct Trace temp_trace = {};
    if(pnum==0){
        offset=0;
        sem_id = recover_sem("sem.key");
        file = fopen("gcc.trace","r");
        if(!file){
            printf("Error reading file\n");
            exit(EXIT_FAILURE);
        }
    }
    else if(pnum == 1){
        offset = q;
        sem_id = recover_sem("sem1.key");
        file = fopen("bzip.trace","r");
        if(!file){
            printf("Error reading file\n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("Wrong argument\n");
        exit(EXIT_FAILURE);
    }

    shm_id = recover_shared("shared.key",sizeof(struct Trace)*2*q);
    struct Trace *shared = (struct Trace*)shmat(shm_id,0,0);

    int count=0;
    while(count < ntraces){
        if (!semaphore_p(sem_id, 1)){ //If empty is True(1) then down() its semaphore
            exit(EXIT_FAILURE);
        }
        if (!semaphore_p(sem_id, 0)){ //If the shared memory is not busy, available is True(1)
            exit(EXIT_FAILURE);
        }
        for(int i=0;i<q;i++){
            if( (getline(&line,&len,file)!=-1)||count != ntraces ){
                sscanf(line,"%x %c",&hex, &action);
                hex /= 4096; //Cut off the last 3 offset bits
                temp_trace.pageNum = hex;
                temp_trace.action = action;
                shared[i+offset] = temp_trace;
                count++;
            }
            else{
                //File ended or traces completed
                break;
            }
        }
        if (!semaphore_v(sem_id, 0)) //No need for the shared mem to stay busy, so we up() the available
            exit(EXIT_FAILURE);
        if (!semaphore_v(sem_id, 2)){ //Now the shared memory is full, so we up() the full semaphores
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    if(line){
        free(line);
    }


    
}

int mm(int ntraces, int q, int k, int frames){
    int sem_id,sem_id1,shm_id;
    int total_disk_reads=0,total_disk_writes=0,total_entries=0;

    //Get the semaphores and the shared memory
    sem_id = recover_sem("sem.key");
    sem_id1 = recover_sem("sem1.key");
    shm_id = recover_shared("shared.key",sizeof(struct Trace)*2*q);
    struct Trace *shared = (struct Trace*)shmat(shm_id,0,0);
    int buckets;
    if(frames <3){
        buckets = 1;
    }
    else{
        buckets = frames/3;
    }
    struct HashTable*table = createHash(buckets);
    printf("Created\n");
    int iterations = ntraces;
    int count=0;
    int count2 = 0;
    int MaxFrames = 0;

    while(count<iterations){
        if (!semaphore_p(sem_id, 2)){ //down() full semaphore 
            exit(EXIT_FAILURE);
        }
        if (!semaphore_p(sem_id, 0)){ //down() available semaphore
            exit(EXIT_FAILURE);
        }
        //Start of Critical Section for Process 0
        for(int i = 0;i < q;i++){
            if (count == iterations){
                break;
            }
            if (table->nentries1 == k){
                if(findHash(shared[i].pageNum,table,0)==NULL){ //This will result into a pagefault
                    //We need to flush the hash table
                    //First delete the buckets
                    printf("Flush 1\n");
                    for (int j=0;j<table->nBuckets;j++){
                        deletePages(j,table,0);
                    }
                    table->nentries1 = 0;
                }
            }
            printf("1 Page %x Action %c\n",shared[i].pageNum, shared[i].action);
            insertHash(shared[i],table,0);
            if (table->nentries1 + table->nentries2 > MaxFrames){
                MaxFrames = table->nentries1 + table->nentries2;
            }
            count ++;
            
        }
        //End of Critical Section for Process 0
        if (!semaphore_v(sem_id, 0)){ //up() available semaphore
            exit(EXIT_FAILURE);
        }
        if (!semaphore_v(sem_id, 1)){ //up() empty semaphore
            exit(EXIT_FAILURE);}


        //For the second process
        if (!semaphore_p(sem_id1, 2)){ //down() full semaphore 
            exit(EXIT_FAILURE);
        }
        if (!semaphore_p(sem_id1, 0)){ //down() available semaphore
            exit(EXIT_FAILURE);
        }
        //Start of Critical Session for Process 1
        for(int i = q;i < q*2;i++){
            if (count2 ==ntraces){
                break;
            }
            if (table->nentries2 == k){
                
                if(findHash(shared[i].pageNum,table,1)==NULL){ //This will result into a pagefault
                    //We need to flush the hash table
                    //First delete the buckets
                    printf("Flush 2\n");
                    for (int j=0;j<table->nBuckets;j++){
                        deletePages(j,table,1);
                    }
                    table->nentries2 = 0;
                }
            }
            printf("2 Page %x Action %c\n",shared[i].pageNum, shared[i].action);
            insertHash(shared[i],table,1);
            if (table->nentries1 + table->nentries2 > MaxFrames){
                MaxFrames = table->nentries1 + table->nentries2;
            }
            count2++;
            
        }
        //End of Critical Section for Process 1
        if (!semaphore_v(sem_id1, 0)){ //up() available semaphore
            exit(EXIT_FAILURE);
        }
        if (!semaphore_v(sem_id1, 1)){ //up() empty semaphore
            exit(EXIT_FAILURE);}
    }
    printf("Before closing the simulation: \n");
    printf("\nTotal disk Reads: %d\n",table->DiskReads);
    printf("Total disk Writes: %d\n",table->DiskWrites);
    printf("Total page Faults: %d\n",table->totalEntries);
    printf("Max frames in use: %d\n",MaxFrames);
    printf("Total traces examined: %d\n",ntraces*2);

    for (int j=0;j<table->nBuckets;j++){
        deletePages(j,table,0);
    }
    for (int j=0;j<table->nBuckets;j++){
        deletePages(j,table,1);
    }

    printf("\nAfter closing the simulation: \n");
    printf("\nTotal disk Reads: %d\n",table->DiskReads);
    printf("Total disk Writes: %d\n",table->DiskWrites);
    printf("Total page Faults: %d\n",table->totalEntries);
    printf("Max frames in use: %d\n",MaxFrames);
    printf("Total traces examined: %d\n",ntraces*2);

    deleteHashTable(table);

    


}

