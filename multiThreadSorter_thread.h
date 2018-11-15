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

datarow create_datarow(cell* cells, int col_count);
table* create_table();
void append(table* tab, datarow* row);


// string processing
char** split_by_comma(char* line, int* len);

cell* get_cells(char** pre_cell, char data_type, int index, int len);

void print_header(char** vals, int n, FILE* stream);

// post processing
void print_row(datarow* row, FILE* stream);

char get_type(char* val);

// mergesort
int compare(datarow A, datarow B, int index);

datarow * merge(datarow * A, int nA, datarow * B, int nB, int index);

datarow * sort(datarow * data, int ndata, int index);

datarow * mergesort(datarow * data, int index, int nrows );

// recursive scanner sort, returns with number of forks created
int recursive_scan_and_sort(char* dts, char* header, char* od, pid_t *pids, int* size, int* lock);

// sort a file, returns 1
int sort_file(char* file_path, char* directory_path, char* filename, char* header, char* od);

//
unsigned long hash(unsigned char *str);
#endif
