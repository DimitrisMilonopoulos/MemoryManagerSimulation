#include <stdio.h>
#include <stdlib.h>
#include "HashTable.h"

unsigned int Hash(unsigned int num,int nbuckets){
    long p = 1302397;
    long a = 35759;
    long b = 128932;
    
    return ((a*num+b) % p) % nbuckets; 
}

/*The hash table will be a dynamicaly allocated array of arrayItem Structs*/

struct node* findHash(int key, struct HashTable* ht,int process){
    struct node *result = NULL;
    struct node *temp;
    int index = Hash(key, ht->nBuckets);
    temp = ht->HashArray[index]->head;

    while(temp!=NULL){
        if(temp->key == key && temp->process==process){
            result = temp;
            break;
        }
        temp = temp->next;
    }

    return result;

}

int deleteBucket(int index, struct HashTable* ht){
    struct node* curr;
    struct node* next;

    curr = ht->HashArray[index]->head;
    if (curr == NULL){
        return 1;
    } 

    while(curr != NULL){
        if(curr->dirty == 1){
            ht->DiskWrites++;
        }
        next = curr->next;
        free(curr);
        curr=next;
    }
    ht->HashArray[index]->head =NULL;
    ht->HashArray[index]->tail =NULL;

    return 1;
}

int deletePages(int index, struct HashTable* ht,int process){
    struct node* curr;
    struct node* prev;
    struct node* next;

    curr = ht->HashArray[index]->head;
    prev = curr;
    if (curr == NULL){
        return 1;
    } 

    while(curr != NULL){
        if(curr->process==process){
            if(curr->dirty == 1){
                ht->DiskWrites++;
            }
            if(curr == ht->HashArray[index]->head){//If we are going to delete the first node
                ht->HashArray[index]->head = curr->next;

                if (curr->next ==NULL){ //Means that we only have one node 
                    ht->HashArray[index]->tail = NULL;
                    free(curr);
                    curr = NULL;
                }
                else{
                    prev = curr->next;
                    free(curr);
                    curr = prev;
                }
                

                
            }
            else if(curr->next == NULL){ //Last node of the list
                ht->HashArray[index]->tail=prev;
                prev->next = NULL;
                free(curr);
                curr = prev->next;
            }
            else{//We are in the middle
                prev->next = curr->next;
                free(curr);
            curr = prev->next;
            }

            //Previous remains unchanged 
            
        }
        else{
            prev = curr;
            curr=curr->next;}
    }

    return 1;
}
struct HashTable* createHash(int nBuckets){
    struct HashTable* table = malloc(sizeof(struct HashTable));
    table->nBuckets = nBuckets;
    table->nentries1 = 0;
    table->nentries2 = 0;
    table->DiskReads=0;
    table->DiskWrites=0;
    table->totalEntries=0;
    table->HashArray = malloc(sizeof(struct arrayItem*)*nBuckets);
    for(int i=0;i<nBuckets;i++){
        table->HashArray[i] = malloc(sizeof(struct arrayItem));
    }
    //initialize the values
    for(int i=0;i<nBuckets;i++){
        table->HashArray[i]->head = NULL;
        table->HashArray[i]->tail = NULL;
    }

    return table;
}

void deleteHashTable(struct HashTable* table){
    if (table == NULL){
        return;
    }

    for(int i=0; i<table->nBuckets;i++){
        deleteBucket(i,table);
        free(table->HashArray[i]);
    }
    free(table->HashArray);
    free(table);
}
int insertHash(struct Trace trace,struct HashTable *ht,int process){
    
    int index;
    struct node *find_index;
    //create the item to add in the linked list
    struct node * newNode;
    index = Hash(trace.pageNum, ht->nBuckets);

    

    if(ht->HashArray[index]->head==NULL){ //Means that the linked list in this position is empty
        ht->DiskReads++;
        newNode= malloc(sizeof(struct node));
        printf("Not found pageNum %x!\n",trace.pageNum);
        if (newNode == NULL){
            printf("Error\n");
            return -1;
        }

        newNode->key = trace.pageNum;
        if(trace.action == 'W')
            newNode->dirty =1;
        else
            newNode->dirty =0;
        newNode->next = NULL;
        newNode->process=process;

        ht->HashArray[index]->head = newNode;
        ht->HashArray[index]->tail = newNode;
        if (process == 0){
            ht->nentries1++;
        }
        else if(process == 1){
            ht->nentries2++;
        }

        ht->totalEntries++;
        return 0;

    }
    else{
        find_index = findHash(trace.pageNum,ht,process);
        if(find_index == NULL){//means there isn't any other entry with this key in the hash Table
            printf("Not found pageNum %x!\n",trace.pageNum);
            ht->DiskReads++;
           newNode= malloc(sizeof(struct node));
            if (newNode == NULL){
                printf("Error\n");
                return -1;
            }

            newNode->key = trace.pageNum;
            if(trace.action == 'W'){
                newNode->dirty =1;
                }
            else
                newNode->dirty =0;
            newNode->process=process;
            newNode->next = NULL;
        
            ht->HashArray[index]->tail->next = newNode;
            ht->HashArray[index]->tail = newNode;
            if (process==0){
                ht->nentries1++;
            }
            else if(process == 1){
                ht->nentries2++;
            }
            ht->totalEntries++;
        }
        else{
            if(trace.action == 'W'){
                if(find_index->dirty ==0){
                    find_index->dirty=1;
                } 
            }
        }
    }
    return -1;

}

