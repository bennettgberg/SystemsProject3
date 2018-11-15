#include "stringproc.h"


int main(int argc, char* argv[]) {
	int count = 0;
	char** test1 = split_by_comma("a,b,c,d,\"eee, kk\",,130", &count);
	cell* cells1 = get_cells(test1, STR, 4, 7);
	int i = 0;
	for(i = 0; i < count; i++)
		printf("%s\n", test1[i]);
	printf("str, %s\n", cells1[4].str_datum);
	return 0;
}
