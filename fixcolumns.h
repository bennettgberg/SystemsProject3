#ifndef FIXCOLS
#define FIXCOLS

#include "multiThreadSorter_thread.h"

#define HMAP 132
#define hmap(key) (abs(key) % HMAP)

datarow create_datarow(cell* cells, int col_count);
table* create_table();
void append(table* tab, datarow* row);


// string processing
char** split_by_comma(char* line, int* len);

cell* get_cells(char** pre_cell, char data_type, int index, int len, char** headers);

void print_header(char** vals, int n, FILE* stream);

// post processing
void print_row(datarow* row, FILE* stream);

char get_type(char* val);

// sort a file, returns 1
table* sort_file(char* file_path, int index);

typedef struct {
	int hash;
	int index;
} property;

void initialize_headers();

bool exists(char* header);

int get_index_nc(char* header);

property* get_header_p(char* header);

void testcols();



#endif
