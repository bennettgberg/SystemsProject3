#ifndef FIXCOLS
#define FIXCOLS

#include "multiThreadSorter_thread.h"

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

// sort a file, returns 1
int sort_file(char* file_path, char* directory_path, char* filename, char* header, char* od);

#endif