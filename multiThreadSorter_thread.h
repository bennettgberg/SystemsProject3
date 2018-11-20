#ifndef SIMPLECSVSORTER
#define SIMPLECSVSORTER

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

// define data types
#define bool char
#define true 1

#define false 0
#define INT 'i'
#define STR 's'
#define DATE 'd'
#define FLOAT 'f'

#define LOCKED 0
#define UNLOCKED 1

#define TABLE_SIZE 5000

#define max(a, b) ((a) > (b) ? (a) : (b))


typedef struct {
        char* original;
        char* str_datum;
        int str_len;
        int num_datum;
	float f_datum;
        char data_type;
        bool is_empty;
} cell;

typedef struct {
        cell* cells;
        int size;
} datarow;

typedef struct {
        datarow* rows;
        char** header;
        int size;
        int max_size;
} table;

typedef struct {
	char ** input;
	table * output;
//	int ** children;
} thread_pointer;

// mergesort
int compare(datarow A, datarow B, int index);

datarow * merge(datarow * A, int nA, datarow * B, int nB, int index);

datarow * sort(datarow * data, int ndata, int index);

datarow * mergesort(datarow * data, int index, int nrows );

// recursive threading function to scan directories, returns sorted table
void * directory_scan(void * );

//threading function to call sort_file, returns sorted table
void * file_sort(void * );

//
unsigned long hash(unsigned char *str);

//Global variables
pthread_mutex_t lock;
int counter;
int header_ind;     //index of the header to sort on.
pthread_t tid[500];
int pid0;

#endif
