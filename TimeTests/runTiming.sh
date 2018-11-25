#!/bin/bash


for idx in 0 1 2 4 8 16 32 64 128 256
do
	mkdir Test$idx
	cd Test$idx
	#cp ~/Systems/SystemsProject3/TimeTests/scannerCSVsorter .
	for j in `seq 0 $idx`
	do
		cp ~/Systems/SystemsProject3/TimeTests/movie_metadata.csv movie_metadata$j.csv
	done
	#cd 
	echo "Running with multiple directories, $idx files"
	cd ~/Systems/SystemsProject3/TimeTests/Test0
	cp ../multiThreadSorter_thread .
	time ./multiThreadSorter_thread -c movie_title -o ..
	cd -
done
