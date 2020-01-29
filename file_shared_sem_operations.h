#ifndef FILE_SHARED_SEM_OPERATIONS_H
#define FILE_SHARED_SEM_OPERATIONS_H

int create_sem(char *,int,int); //create a semaphore and save its key to a file
int recover_sem(char*); //recove semaphore id from a key file
void delete_sem(char *); //delete a semaphore from a key file along with the file

int create_shared(char *,size_t,int); //create a shared-mem segment of a given size and save its key to a file
int recover_shared(char*, size_t);    //recover a memory segment of a given size from a key file
void delete_shared(char *,size_t);    //delete a memory segment of given size from a key file
#endif