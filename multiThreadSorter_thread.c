#include "fixcolumns.h"

// implementation

void * file_sort(void * tp_in) {
	thread_pointer* my_tp = (thread_pointer*)tp_in;
	char ** input = my_tp->input;
	thread_pointer* tp_out = (thread_pointer*)malloc(sizeof(thread_pointer));
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
	pthread_mutex_lock(&lock); 
	printf("Initial PID: %d\n", pid0);
	printf("TIDs of all spawned threads: ");
	printf("\nTotal number of threads: 0\n");
	pthread_mutex_unlock(&lock);
	tp_out->output = file_table;
	tp_out->children = (int*)malloc(sizeof(int*));
	tp_out->tot_count = 0;
	return tp_out;
}

void * directory_scan(void * tp_in) {
	thread_pointer *my_tp = (thread_pointer*)tp_in;
	char ** input = my_tp->input;
	thread_pointer *tp_out = (thread_pointer*)malloc(sizeof(thread_pointer)); //output thread_pointer
	char* directory_to_search = *input; 
	DIR *dir = opendir(directory_to_search);
	int count = 0;
	int error;
	table * result = (table*)malloc(sizeof(table));
	result->size = 0;
	int * children = (int*)malloc(500*sizeof(int)); //array of child/grandchild thread IDs
	pthread_t * chilin = (pthread_t*)malloc(500*sizeof(pthread_t));
	int my_count = 0; //count of how many threads I've spawned
	thread_pointer ** tps = (thread_pointer**)malloc(500*sizeof(thread_pointer*)); //array of thread_pointers to be used as input/output for all new threads
	tp_out->tot_count = 0;
	char** new_names = (char**)malloc(500*sizeof(char*));
	if(dir != NULL) {
		struct dirent *de;
		de = readdir(dir); // skip .
		de = readdir(dir); // skip ..
		while((de = readdir(dir)) != NULL) {
			int name_len = strlen(de->d_name);
			if(name_len == 0){ //if no files left in this directory
				break;
			}
			int dir_len = strlen(directory_to_search);
			new_names[my_count] = (char*)malloc(dir_len + name_len + 2);
			if(directory_to_search[dir_len - 1] == '/') {
				directory_to_search[dir_len - 1] = '\0';
			}
			sprintf(new_names[my_count], "%s/%s", directory_to_search, de->d_name);
			tps[my_count] = (thread_pointer*)malloc(sizeof(thread_pointer));
			tps[my_count]->input = &new_names[my_count];
		//lock mutex to access counter. 
			pthread_mutex_lock(&lock1); 
			pthread_t * tidctr = &tid[counter];
			pthread_mutex_unlock(&lock1);
			if(de->d_type & DT_DIR) {
				error = pthread_create(tidctr, NULL, directory_scan, (void *)tps[my_count]);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for directory %s: error %d", new_names[my_count], error);
					return tp_out;
				}
			} //end de is a directory
			else { //de is actually a file.
				error = pthread_create(tidctr, NULL, file_sort, (void *)tps[my_count]);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for file %s: error %d", new_names[my_count], error);
					return tp_out;
				}
			} //end this is a file
			pthread_mutex_lock(&lock1); 
			children[my_count] = counter; //save ID (integer) to print later
			chilin[my_count++] = tid[counter++]; //save TID so we can join later
			pthread_mutex_unlock(&lock1);
			tp_out->tot_count++; 
		} //end while readdir not null	
		closedir(dir);
	} //end if dir not null
	table * my_output;	
	int new_size;
	thread_pointer * tp;
	int i;
	for(i = 0; i < my_count; ++i){
		pthread_join(chilin[i], NULL);
		tp = (thread_pointer*)tps[i];
		//Now, merge results to result table. Should the merge function free the now-unused memory??
		my_output = tp->output;
		if(my_output) {
			new_size = result->size + my_output->size;
			result->rows = merge(result->rows, result->size, my_output->rows, my_output->size, header_ind);
			result->size = new_size;
		}
		//add IDs of children to my list.
		if(tp->tot_count > 0) {
			memcpy(&tp_out->children[tp_out->tot_count], &tp->children, tp->tot_count*sizeof(int));
		}
		tp_out->tot_count += tp->tot_count;
		//free memory?
	}
	tp_out->output = result;	
	tp_out->children = children;
	pthread_mutex_lock(&lock); 
	printf("Initial PID: %d\n", pid0);
	printf("TIDs of all spawned threads: ");
	for(i = 0; i < tp_out->tot_count; ++i){
		printf("%d, ", tp_out->children[i]);
	}
	printf("\nTotal number of threads: %d\n", tp_out->tot_count);
	pthread_mutex_unlock(&lock);
	return tp_out;
}

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 7 || argc % 2 == 0) {
		perror("incorrect arguments");
		fprintf(stdout, "incorrect arguments");
		return 0;
	}
	initialize_headers();
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
			if(directory_to_search != NULL) {
				perror("incorrect arguments, cannot have more than one -d");
				fprintf(stdout, "incorrect arguments, cannot have more than one -d");
				return 0;
			}
			directory_to_search = argv[i];
		}
        else if(strcmp(argv[i], "-o") == 0) {
			i++;
			if(output_directory != NULL) {
				perror("incorrect arguments, cannot have more than one -o");
				fprintf(stdout, "incorrect arguments, cannot have more than one -o");
				return 0;
			}
			output_directory = argv[i];
		}  
	}
	if(header_to_sort == NULL) {
		perror("No header supplied as input.");
		fprintf(stdout, "No header supplied as input.");
		return 0;
	}
	char header_type = get_type(header_to_sort);
	header_ind = get_index_nc(header_to_sort);
	if(header_type == 'E') {
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
	pid0 = getpid();
	int mut = pthread_mutex_init(&lock, NULL); 
	int mut1 = pthread_mutex_init(&lock1, NULL); 
	if (mut != 0) { 
		fprintf(stdout, "Error: mutex initialization failed: %d\n", mut); 
		fprintf(stderr, "Error: mutex initialization failed: %d\n", mut); 
		return 0; 
	} 
	thread_pointer * tp = (thread_pointer*)malloc(sizeof(thread_pointer));
	tp->input = &directory_to_search;

//Call directory_scan to start recursion!
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
	print_header(fout); //Is this correct way to print headers?
	for(i = 0; i < all_data->size; ++i){
		print_row(&(all_data->rows[i]), fout);
	}
	
	fclose(fout);
	//destroy mutex lock now that we're done with it.
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&lock1);
//	printf("Initial PID: %d\n", pid0); //already print this in first directory_scan call!
//	printf("TIDs of all spawned threads: ");
//	for(i = 0; i < counter; ++i) {
//		printf("%d, ", my_tp->children[i]);
//	}
//	printf("\nTotal number of threads: %d\n", counter); 
	return 0;
}
