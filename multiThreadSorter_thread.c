#include "scannerCSVsorter.h"

// implementation
char get_type(char* val) {
	if(strcmp(val, "color") == 0 || strcmp(val, "director_name") == 0 || strcmp(val, "actor_2_name") == 0 || strcmp(val, "genres") == 0 || strcmp(val, "actor_1_name") == 0 || strcmp(val, "movie_title") == 0 || strcmp(val, "actor_3_name") == 0 || strcmp(val, "plot_keywords") == 0  || strcmp(val, "movie_imdb_link") == 0 || strcmp(val, "language") == 0 || strcmp(val, "country") == 0 || strcmp(val, "content_rating") == 0) {
		return STR;
	}
	else if( strcmp(val, "num_critic_for_reviews") == 0 ||  strcmp(val, "director_facebook_likes") == 0 ||  strcmp(val, "actor_3_facebook_likes") == 0 ||  strcmp(val, "actor_1_facebook_likes") == 0 ||  strcmp(val, "gross") == 0 ||  strcmp(val, "num_voted_users") == 0 ||  strcmp(val, "cast_total_facebook_likes") == 0 ||  strcmp(val, "facenumber_in_poster") == 0 ||  strcmp(val, "num_user_for_reviews") == 0 ||  strcmp(val, "budget") == 0 ||  strcmp(val, "title_year") == 0 ||  strcmp(val, "actor_2_facebook_likes") == 0 ||  strcmp(val, "imdb_score") == 0 ||  strcmp(val, "aspect_ratio") == 0 ||  strcmp(val, "movie_facebook_likes") == 0 || strcmp(val, "duration") == 0) 
		return FLOAT;
	return 'E'; //for Error
}

void print_row(datarow* row, FILE* stream) {
	int i = 0;
	for(i = 0; i < row->size; i++) {
		fprintf(stream, "%s", row->cells[i].original);
		if(i < row->size - 1) fprintf(stream, ",");
	}

	fprintf(stream, "\n");
}

void print_header(char** vals, int n, FILE* stream) {
	int i = 0;
	for(i = 0; i < n; i++) {
		fprintf(stream, "%s", vals[i]);
		if(i < n - 1) fprintf(stream, ",");
	}
	fprintf(stream, "\n");
}

datarow create_datarow(cell* cells, int col_count) {
	datarow row;
	row.size = col_count;
	row.cells = cells;
	return row;
}

table* create_table() {
	table* tab = (table*)malloc(sizeof(table));
	tab->rows = (datarow*)malloc(TABLE_SIZE * sizeof(datarow));
	tab->max_size = TABLE_SIZE;
	tab->size = 0;
	return tab;
}

void append(table* tab, datarow* row) {
	if(tab->size < tab->max_size) {
		tab->rows[tab->size] = *row;
		tab->size++;
	}
	else {
		datarow* temp = tab->rows;
		tab->max_size *= 2;
		tab->rows = (datarow*)malloc(tab->max_size * sizeof(datarow));
		int i = 0;
		for(i = 0; i < tab->size; i++)
			tab->rows[i] = temp[i];
		free(temp);
		append(tab, row);
	}
}

char** split_by_comma(char* line, int* len) {
	int i = 0, prev_i = 0;
	int count = 1;
	int max_size = 1;
	bool inside_quote = false;
	int line_length = 0;
	while(line[i] != '\0' && line[i] != '\n') {
		if(line[i] == '"')
			inside_quote = inside_quote ? false : true;
		if(line[i] == ',' && inside_quote == false) {
			count++;
			max_size = max(max_size, i - prev_i);
			prev_i = i;
		}
		i++;
		line_length++;
	}
	inside_quote = false;
	char** words = (char**)malloc(count * sizeof(char*));
	for(i = 0; i < count; i++) {
		words[i] = (char*)malloc(max_size * sizeof(char));
		words[i][0] = '\0';
	}
	int curr_word = 0;
	int curr_i = 0;
	for(i = 0; i < line_length; i++) {
		if(line[i] == '"')
        	       	inside_quote = inside_quote ? false : true;
		if(line[i] == ',' && inside_quote == false) {
			words[curr_word++][curr_i] = '\0';
			curr_i = 0;
		}
		else {
			words[curr_word][curr_i++] = line[i];
		}
	}
	*len = count;
	return words;
}


cell* get_cells(char** pre_cell, char data_type, int index, int len) {
	int i = 0;
	cell* cells = (cell*)malloc(len * sizeof(cell));
	for(i = 0; i < len; i++) {
		cells[i].original = pre_cell[i];
		cells[i].is_empty = pre_cell[i][0] == '\0' ? true : false;
		if(cells[i].is_empty) {
			cells[i].str_len = 0;
		}
		if(i == index) {
			cells[i].data_type = data_type;
			int st = 0;
			int end = 0;
			while(pre_cell[i][st] == ' ' || pre_cell[i][st] == '\t' || pre_cell[i][st] == '"') st++;
			while(pre_cell[i][end] != '\0') end++;
			end--;
			while(end > -1 && (pre_cell[i][end] == ' ' || pre_cell[i][end] == '\t' || pre_cell[i][end] == '"')) end--;
			if(end <= st) cells[i].is_empty = true;
			else {
				cells[i].is_empty = false;
				int j = 0;
				cells[i].str_datum = (char*)malloc((end - st + 2) * sizeof(char));
				for(j = st; j <= end; j++)
					cells[i].str_datum[j - st] = pre_cell[i][j];
				cells[i].str_datum[j - st] = '\0';
				cells[i].str_len = end - st + 1;
				if(data_type == INT)
					cells[i].num_datum = atoi(cells[i].str_datum);
				else if(data_type == FLOAT)
					cells[i].f_datum = atof(cells[i].str_datum);
			}
		}
	}
	return cells;
}

int sort_file(char* file_path, char* dts, char* filename, char* header_to_sort, char* od) {
	FILE* fp = fopen(file_path, "r");
	char sort_type = get_type(header_to_sort);
	char buff[BUFSIZ];
	char *read = fgets(buff, sizeof buff, fp);
	int no_of_cols = 0;
	char** headers = split_by_comma(buff, &no_of_cols);
	int cell_index = -1;
	char* check_for_pre_sort = (char*)malloc(strlen(header_to_sort) + 10);
	sprintf(check_for_pre_sort, "%s-sorted-", header_to_sort);
	if(strstr(filename, check_for_pre_sort) != NULL) return 0;
	int i;
	for(i = 0; i < no_of_cols; i++) {
		if(!strcmp(headers[i], header_to_sort)) {
			cell_index = i;
			break;
		}
	}
	if(cell_index != -1) {
		table* main_table = create_table();
		main_table->header = headers;
		read = fgets(buff, sizeof buff, fp);
		while(read != NULL) {
			int nc = 0;
			char** split_line = split_by_comma(buff, &nc);
			if(nc != no_of_cols) {
				fprintf(stderr, "Could not sort file %s: incorrect format", filename);
				exit(0);
			}
			cell* cells = get_cells(split_line, sort_type, cell_index, nc);
			datarow row = create_datarow(cells, nc);
			append(main_table, &row); 
			read = fgets(buff, sizeof buff, fp);
		}
		datarow* sorted = mergesort(main_table->rows, cell_index, main_table->size);
		FILE* fout;
		if(od==NULL) { // od is null means that there is no specified output directory
			char* new_name = (char*)malloc(strlen(file_path) + strlen(header_to_sort) + 10);
			sprintf(new_name, "%s/%s-sorted-%s", dts, header_to_sort, filename);
			if((fout=fopen(new_name, "w"))==NULL) {
				perror("Cannot open file.\n");
				exit(0);
			}
		}
		else { // od is specified already.                                    
			char* new_name = (char*)malloc(strlen(od) + strlen(header_to_sort) + 10);
			if(od[strlen(od)-1] != '/')
				sprintf(new_name, "%s/%s-sorted-%s", od, header_to_sort, filename);
			else
				sprintf(new_name, "%s%s-sorted-%s", od, header_to_sort, filename);
				
			if((fout=fopen(new_name, "w"))==NULL) {
				perror("Cannot open file.\n");
				exit(0);
			}
		}
		print_header(headers, no_of_cols, fout);
		int j;
		for(j = 0; j < main_table->size; ++j){
			print_row(&(sorted[j]), fout);
		}
		fclose(fout);
	}
	else {
		fprintf(stderr, "Column %s does not exist in file %s", header_to_sort, file_path);
		exit(0);
	}
	fclose(fp);
	return 1;
}


int recursive_scan_and_sort(char* dts, char* header, char* od, pid_t *pids, int *size, int* lock) {
	DIR *dir = opendir(dts);
	pid_t fpid, dpid; //directory pid and file pid
	int count = 0;
	if(dir != NULL) {
		struct dirent *de;
		de = readdir(dir); // skip .
		de = readdir(dir); // skip ..
		while((de = readdir(dir)) != NULL) {
			int name_len = strlen(de->d_name);
			int dir_len = strlen(dts);
			char* new_name = (char*)malloc(dir_len + name_len + 2);
			if(dts[dir_len - 1] == '/') {
				dts[dir_len - 1] = '\0';
			}
			sprintf(new_name, "%s/%s", dts, de->d_name);
			if(de->d_type & DT_DIR) {
				dpid = fork();
				if(dpid < 0){
					fprintf(stderr, "Error: could not fork for directory %s", new_name);
					exit(0);
				}
				else if(dpid > 0){
					int eval = 0;
					wait(&eval);
					count += WEXITSTATUS(eval);
					while(*lock == LOCKED);
					*lock = LOCKED;
					*size += 1;
					*lock = UNLOCKED;
				}
				else {
					printf("%d ", getpid());
					fflush(stdout);
					count += recursive_scan_and_sort(new_name, header, od, pids, size, lock);
					free(new_name);
					exit(count);
				}
			}
			else {
				fpid = fork();
				if(fpid == 0) {
					printf("%d ", getpid());
					fflush(stdout);
					if(
					name_len >= 4 && 
					de->d_name[name_len - 1] == 'v' && 
					de->d_name[name_len - 2] == 's' && 
					de->d_name[name_len - 3] == 'c' &&
					de->d_name[name_len - 4] == '.'){
						sort_file(new_name, dts, de->d_name, header, od);
					} //end file is .csv
					free(new_name);
					exit(1);
				} //end this is child process
				else {
					int eval = 0;
					wait(&eval);
					count += WEXITSTATUS(eval);
					while(*lock == LOCKED);
					*lock = LOCKED;
					*size += 1;
					*lock = UNLOCKED;
			     } //end this is parent process
			} //end not a directory
		} //end while readdir not null
		
		closedir(dir);
	} //end if dir not null
	return count;
}

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 7 || argc % 2 == 0) {
		perror("incorrect arguments");
		fprintf(stdout, "incorrect arguments");
		return 0;
	}
	int i = 1;
	char *header_to_sort = NULL, *directory_to_search = NULL, *output_directory = NULL;
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-c") == 0) {
			i++;
			header_to_sort = argv[i];
		} 
        else if(strcmp(argv[i], "-d") == 0) {
			i++;
			directory_to_search = argv[i];
		}
        else if(strcmp(argv[i], "-o") == 0) {
			i++;
			output_directory = argv[i];
		}  
	}
	if(header_to_sort == NULL) {
		perror("No header supplied as input.");
		fprintf(stdout, "No header supplied as input.");
		return 0;
	}
	if(get_type(header_to_sort) == 'E') {
		fprintf(stderr, "Error: %s is not a valid column header. Did not sort any files.\n", header_to_sort);
		fprintf(stdout, "Error: %s is not a valid column header. Did not sort any files.\n", header_to_sort);
		exit(0);
	}
    //if output directory isn't an absolute path, we need to store the current path.
	char * current_d = NULL;
    	if(output_directory != NULL && output_directory[0] != '/'){
        	int odsize = strlen(output_directory);
		current_d = (char*)malloc(1000*sizeof(char));
		current_d = getcwd(current_d, 1000);
		int currd_len = strlen(current_d);
		char * new_d = (char*)malloc((currd_len+odsize+2)*sizeof(char));
		strcpy(new_d, current_d);
		if(new_d[currd_len-1] != '/'){
			strcat(new_d, "/");
		}
		strcat(new_d, output_directory);
		output_directory = new_d;
	}
	if(directory_to_search == NULL) {
		if(current_d == NULL) {
			current_d = (char*)malloc(1000*sizeof(char));
			current_d = getcwd(current_d, 1000);
		}
		directory_to_search = current_d;
    	}
	// create shared memory across processes
	int size_id, lock_id;
	size_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	lock_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int *size, *lock;
	size = (int *)shmat(size_id, 0, 0);
	lock = (int *)shmat(lock_id, 0, 0);
	*size = 0;
	*lock = UNLOCKED;
	printf("Initial PID: %d\n", getpid());
	printf("PIDs of all child processes: \n");
	recursive_scan_and_sort(directory_to_search, header_to_sort, output_directory, NULL, size, lock);
	
	printf("\nTotal number of processes: %d\n", *size); //Just size instead of size+1 because main process doesn't count.
	return 0;
}
