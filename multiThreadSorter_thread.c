#include "fixcolumns.h"

// implementation

pthread_mutex_t lock;
int counter;
char * header_to_sort, output_directory;
pthread_t tid[256];

void * file_sort(void * file_to_sort) {
	char * fname = (char*)file_to_sort;
	if( strlen(fname) >= 4 && 
	fname[name_len - 1] == 'v' && 
	fname[name_len - 2] == 's' && 
	fname[name_len - 3] == 'c' &&
	fname[name_len - 4] == '.'){
	//separate directory_to_search and filename***********
		sort_file(fname, dts, filename, header_to_sort, output_directory);
	} //end file is .csv
	return NULL;
}

void * directory_scan(void * directory_to_search) {
	DIR *dir = opendir((char*)directory_to_search);
	int count = 0;
	int error;
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
				error = pthread_create(&tid[counter], NULL, directory_scan, (void *)new_name);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for directory %s: error %d", new_name, error);
					return NULL;
				}
				//lock the mutex to print and increment the shared counter.
				pthread_mutex_lock(&lock); 
				printf("%d ", tid[counter++]);
				//now that we're done with it, unlock.
				pthread_mutex_unlock(&lock);
				free(new_name);
			}
			else { //de is actually a file.
				error = pthread_create(&tid[counter], NULL, file_sort, (void *)new_name);
				if(error != 0){
					fprintf(stderr, "Error: could not create thread for directory %s: error %d", new_name, error);
					return NULL;
				}
				pthread_mutex_lock(&lock); 
				printf("%d ", tid[counter++]);
				pthread_mutex_unlock(&lock);
				pthread_join(tid[counter], NULL);
				free(new_name);
				return NULL;
			} //end this is a file
		} //end while readdir not null
		
		closedir(dir);
	} //end if dir not null
	return NULL;
}

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 7 || argc % 2 == 0) {
		perror("incorrect arguments");
		fprintf(stdout, "incorrect arguments");
		return 0;
	}
	int i = 1;
	header_to_sort = NULL;
	output_directory = NULL;
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
	directory_scan((void*)directory_to_search);
	//destroy mutex lock now that we're done with it.
	pthread_mutex_destroy(&lock);
	printf("\nTotal number of threads: %d\n", counter); //Just size instead of size+1 because main process doesn't count.
	return 0;
}
