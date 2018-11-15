#include "fixcolumns.h"

// implementation

pthread_mutex_t lock;
int counter;

int recursive_scan_and_sort(char* dts, char* header, char* od) {
	DIR *dir = opendir(dts);
	pid_t ftid, dtid; //directory pid and file pid
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
					count += recursive_scan_and_sort(new_name, header, od);
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
	//initialize mutex lock.
	//run recursive_scan_and_sort.
	printf("Initial PID: %d\n", getpid());
	printf("TIDs of all spawned threads: ");
	recursive_scan_and_sort(directory_to_search, header_to_sort, output_directory);
	
	printf("\nTotal number of threads: %d\n", counter); //Just size instead of size+1 because main process doesn't count.
	return 0;
}
