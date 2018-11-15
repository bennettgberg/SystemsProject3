#include <stdio.h>
#include <string.h>
#include "multiThreadSorter_thread.h"

//return > 0 if A is larger, < 0 if B is larger, 0 if A = B.
int compare(datarow A, datarow B, int index){
	if(A.cells[index].data_type == INT){
		if(A.cells[index].num_datum > B.cells[index].num_datum) {
			return 1;
		}
		else if(A.cells[index].num_datum < B.cells[index].num_datum){
			return -1;
		}
		return 0;
	}	
	else if(A.cells[index].data_type == FLOAT){
		if(A.cells[index].f_datum > B.cells[index].f_datum) {
			return 1;
		}
		else if(A.cells[index].f_datum < B.cells[index].f_datum){
			return -1;
		}
		return 0;
	}	
	return strcmp(A.cells[index].str_datum, B.cells[index].str_datum);
}

datarow * merge(datarow * A, int nA, datarow * B, int nB, int index){
	datarow * merged_data = malloc((nA + nB)*sizeof(datarow));
	int Acount = 0;
	int Bcount = 0;
	int Ccount = 0;
	while(Acount != nA && Bcount != nB){
		int comp = compare(A[Acount], B[Bcount], index);
		if(comp <= 0){
			merged_data[Ccount++] = A[Acount++];	
		}
		else {
			merged_data[Ccount++] = B[Bcount++];
		}
	}
	while(Acount != nA) {
		merged_data[Ccount++] = A[Acount++];
	} 
	while(Bcount != nB) {
		merged_data[Ccount++] = B[Bcount++];
	} 
	return merged_data;
}


datarow * sort(datarow * data, int ndata, int index) {
	if(ndata == 1){
		return data;
	}
	int nA = ndata/2;
	int nB = ndata/2;
	if(ndata % 2 != 0) nB++;
	datarow * A = malloc(nA * sizeof(datarow));
	datarow * B = malloc(nB * sizeof(datarow));
	A = memcpy(A, data, nA*sizeof(datarow));
	B = memcpy(B, &data[nA], nB*sizeof(datarow));
	A = sort(A, nA, index);
	B = sort(B, nB, index);
	return merge(A, nA, B, nB, index);
}


datarow * mergesort(datarow * data, int index, int nrows ) {
  //first do pre-processing and move all the null cells to the front.
  //array to hold final sorted data
	datarow * sorted_data = malloc(nrows*sizeof(datarow));
  //array to hold the non-null data (to merge-sort)
	datarow * to_sort = malloc(nrows*sizeof(datarow));
	int n_null = 0;
	int nto_sort = 0;
	int i;
	for(i = 0; i < nrows; ++i){
		if(data[i].cells[index].is_empty == true){
			sorted_data[n_null++] = data[i];
		}
		else {
			to_sort[nto_sort++] = data[i];
		}
	}

	to_sort = sort(to_sort, nto_sort, index);
	memcpy(&sorted_data[n_null], to_sort, nto_sort*sizeof(datarow));
	return sorted_data;

}
