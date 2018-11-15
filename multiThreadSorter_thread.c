#include "fixcolumns.h"

// implementation


void * file_sort(void * tp) {
	thread_pointer* my_tp = (thread_pointer*)tp;
	char ** input = my_tp->input;
	char * fname = *input;
	table * file_table = NULL;
	int name_len = strlen(fname);
	if( name_len >= 4 && 
	fname[name_len - 1] == 'v' && 
	fname[name_len - 2] == 's' && 
	fname[name_len - 3] == 'c' &&
	fname[name_len - 4] == '.'){
	//sort the file, get an array 
		file_table = sort_file(fname, header_ind);
	} //end file is .csv
	my_tp->output = file_table;
	return tp;
}

void * directory_scan(void * tp) {
	thread_pointer *my_tp = (thread_pointer*)tp;
	char ** input = my_tp->input;
	char* directory_to_search = *input; 
	DIR *dir = opendir(directory_to_search);
	int count = 0;
	int error;
	table * result;
	result->size = 0;
	if(dir != NULL) {
		struct dirent *de;
		de = readdir(dir); // skip .
		de = readdir(dir); // skip ..
		while((de = readdir(dir)) != NULL) {
			int name_len = strlen(de->d_name);
			int dir_len = strlen(directory_to_search);
			char* new_name = (char*)malloc(dir_len + name_len + 2);
			if(directory_to_search[dir_len - 1] == '/') {
				directory_to_search[dir_len - 1] = '\0';
			}
			sprintf(new_name, "%s/%s", directory_to_search, de->d_name);
			thread_pointer * tp = (thread_pointer*)malloc(sizeof(thread_pointer));
			tp->input = &new_name;
			if(de->d_type & DT_DIR) {
				error = pthread_create(&tid[counter], NULL, directory_scan, (void *)tp);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for directory %s: error %d", new_name, error);
					return NULL;
				}
				//lock the mutex to print and increment the shared counter.
				pthread_mutex_lock(&lock); 
				printf("%d ", tid[counter++]);
				//now that we're done with it, unlock.
				pthread_mutex_unlock(&lock);
				pthread_join(tid[counter-1], NULL);
				
				free(new_name);
			}
			else { //de is actually a file.
				error = pthread_create(&tid[counter], NULL, file_sort, (void *)tp);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for directory %s: error %d", new_name, error);
					return NULL;
				}
				pthread_mutex_lock(&lock); 
				printf("%d ", tid[counter++]);
				pthread_mutex_unlock(&lock);
				pthread_join(tid[counter-1], NULL);
				free(new_name);	
				return NULL;
			} //end this is a file
		//Now, merge results to result table. Should the merge function free the now-unused memory??
			table * output_table = ((thread_pointer*)tp)->output;	
			int new_size = result->size + output_table->size;
			result->rows = merge(result->rows, result->size, output_table->rows, output_table->size, header_ind);
			result->size = new_size;
		} //end while readdir not null	
		closedir(dir);
	} //end if dir not null
	my_tp->output = result;	
	return tp;
}

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 7 || argc % 2 == 0) {
		perror("incorrect arguments");
		fprintf(stdout, "incorrect arguments");
		return 0;
	}
	int i = 1;
	char * header_to_sort = NULL;
	char * output_directory = NULL;
	char *directory_to_search = NULL;
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
	header_ind = get_type(header_to_sort);
	if(header_ind == 'E') {
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
	//initialize mutex lock.
	//run recursive_scan_and_sort.
	printf("Initial PID: %d\n", getpid());
	printf("TIDs of all spawned threads: ");
	int mut = pthread_mutex_init(&lock, NULL); 
	if (mut != 0) { 
		fprintf(stdout, "Error: mutex initialization failed: %d\n", mut); 
		fprintf(stderr, "Error: mutex initialization failed: %d\n", mut); 
		return 0; 
	} 
	thread_pointer * tp = (thread_pointer*)malloc(sizeof(thread_pointer));
	tp->input = &directory_to_search;
	void * all_data_void = directory_scan((void*)tp);
	thread_pointer * my_tp = (thread_pointer*)all_data_void;
	table * all_data = my_tp->output;
	char * od = output_directory;
	//print out all data to new file.
	FILE* fout;
	if(output_directory==NULL) { // od is null means that there is no specified output directory
		char* new_name = (char*)malloc(strlen(header_to_sort) + 21);
		sprintf(new_name, "AllFiles-sorted-%s.csv", header_to_sort);
		if((fout=fopen(new_name, "w"))==NULL) {
			perror("Cannot open file.\n");
			exit(0);
		}
	}
	else { // od is specified already.
		char* new_name = (char*)malloc(strlen(od) + strlen(header_to_sort) + 22);
		if(od[strlen(od)-1] != '/')
			sprintf(new_name, "%s/AllFiles-sorted-%s.csv", od, header_to_sort);
		else
			sprintf(new_name, "%sAllFiles-sorted-%s.csv", od, header_to_sort);

		if((fout=fopen(new_name, "w"))==NULL) {
			perror("Cannot open file.\n");
			exit(0);
		}
	}
	print_header(g_headers, 28, fout); //Is this correct way to print headers?
	for(i = 0; i < all_data->size; ++i){
		print_row(&(all_data->rows[i]), fout);
	}
	
	fclose(fout);
	//destroy mutex lock now that we're done with it.
	pthread_mutex_destroy(&lock);
	printf("\nTotal number of threads: %d\n", counter); //Just size instead of size+1 because main process doesn't count.
	return 0;
}
