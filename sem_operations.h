#ifndef SEM_OPERATIONS_H
#define SEM_OPERATIONS_H
union semun{
    int val;
    struct semid_ds *buff;
    unsigned short *array;
}semun;

int set_semvalue(int,int,int ); //sets value to a semaphore
void del_semvalue(int, int);    //deletes a semaphore value
int semaphore_p(int, int);      //up() the semaphore
int semaphore_v(int, int);      //down() the semaphore

#endif