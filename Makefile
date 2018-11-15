all: multiThreadSorter_thread

scannerCSVsort: mergesort.c
	gcc -g -Wall -Werror -fsanitize=address mergesort.c fixcolumns.c multiThreadSorter_thread.c -o multiThreadSorter_thread
	
clean:
	rm multiThreadSorter_thread
