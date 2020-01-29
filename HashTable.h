#ifndef _HASH_
#define _HASH_
#define MAX_BUCKETS

struct node{
    int key;
    char process;
    int dirty;
    struct node* next;
};

struct Trace{
    unsigned int pageNum;
    char action;
};

struct arrayItem{
    struct node* head; 
    //Pointing to the first element of the linked list at an index in the hash Table
    struct node* tail;
    //Pointing to the last element of the linked list at an index in the hash Table
};

struct HashTable{
    struct arrayItem** HashArray;
    int nBuckets;
    int nentries1;
    int nentries2;
    int DiskReads;
    int DiskWrites;
    int totalEntries;
};

unsigned int Hash(unsigned int,int);
struct HashTable* createHash(int);
int insertHash(struct Trace,struct HashTable*,int);
struct node* findHash(int, struct HashTable*,int);
void deleteHashTable(struct HashTable*);
int deleteBucket(int, struct HashTable*);
int deletePages(int, struct HashTable*,int);
#endif