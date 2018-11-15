#include "simpleCSVsorter.h"
#include "mergesort.h"

char get_type(char* val) {
	if(strcmp(val, "color") == 0 || strcmp(val, "director_name") == 0 || strcmp(val, "actor_2_name") == 0 || strcmp(val, "genres") == 0 || strcmp(val, "actor_1_name") == 0 || strcmp(val, "movie_title") == 0 || strcmp(val, "actor_3_name") == 0 || strcmp(val, "plot_keywords") == 0  || strcmp(val, "movie_imdb_link") == 0 || strcmp(val, "language") == 0 || strcmp(val, "country") == 0 || strcmp(val, "content_rating") == 0) {
		return STR;
	}
	else if(strcmp(val, "duration") == 0)
		return INT;
	return INT;
}

void print_row(datarow* row) {
	int i = 0;
	for(i = 0; i < row->size; i++){
		printf("%s", row->cells[i].original);
		if(i < row->size - 1) printf(",");
	}

	printf("\n");
}


int main(int argc, char* argv[]) {
	if(argc < 3)
		perror("incorrect arguments");
	int i = 1;
	char* header_to_sort = NULL;
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-c") == 0) {
			i++;
			header_to_sort = argv[i];
		} 
	}
	char sort_type = get_type(header_to_sort);
	char buff[BUFSIZ];
	char *read = fgets(buff, sizeof buff, stdin);
	int no_of_cols = 0;
	char** headers = split_by_comma(buff, &no_of_cols);
	int cell_index = -1;
	for(i = 0; i < no_of_cols; i++) {
		if(!strcmp(headers[i], header_to_sort)) {
			cell_index = i;
			break;
		}
	}
	if(cell_index != -1) {
		table* main_table = create_table();
		main_table->header = headers;
		read = fgets(buff, sizeof buff, stdin);
		while(read != NULL) {
			int nc = 0;
			char** split_line = split_by_comma(buff, &nc);
			cell* cells = get_cells(split_line, sort_type, cell_index, nc);
			datarow row = create_datarow(cells, nc);
			append(main_table, &row); 
			read = fgets(buff, sizeof buff, stdin);
		}
		datarow* sorted = mergesort(main_table->rows, cell_index, main_table->size);
		int j;
		for(j = 0; j < main_table->size; ++j){
			print_row(&sorted[j]);
		}
//		if(sorted) {}
	}
	return 0;
}
