#include "fixcolumns.h"


int main() {
	initialize_headers();
	printf("%d\n", exists("color"));
	printf("%d\n", exists("color "));
	printf("%c\n", get_type("color"));
	testcols();
	return 0;
}
