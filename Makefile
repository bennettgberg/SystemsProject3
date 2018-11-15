all: multiThreadSorter_thread

multiThreadSorter_thread: mergesort.c fixcolumns.c
	gcc -g -fsanitize=address mergesort.c fixcolumns.c multiThreadSorter_thread.c -o multiThreadSorter_thread

hashtest: fixcolumns.c
	gcc -g -fsanitize=address mergesort.c fixcolumns.c hashtest.c -o hashtest
	./hashtest
	rm hashtest
	
clean:
	rm multiThreadSorter_thread
