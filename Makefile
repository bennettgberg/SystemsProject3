all: scannerCSVsort

scannerCSVsort: mergesort.c
	gcc -g -Wall -Werror -fsanitize=address mergesort.c scannerCSVsorter.c -o scannerCSVsorter
	
clean:
	rm scannerCSVsorter
