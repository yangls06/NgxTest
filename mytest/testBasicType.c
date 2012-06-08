#include <stdio.h>
#include "../src/core/ngx_core.h"

int main(void){
	// test basic type
	ngx_uint_t a = 1000;
	ngx_int_t b = -1000;
	printf("%d + %d = %d\n", a, b, a+b);
	return 0;
}
