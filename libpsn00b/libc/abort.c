#include <stdio.h>

void abort() {

	printf("abort()\n");
	
	while(1);

}