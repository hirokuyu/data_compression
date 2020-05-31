/*my_rand.cpp random sequence generator*/
#include "my_rand.h"

static int x=0;

void my_srand( int i )
{
	x = i;
}
/*random number between 0 and 0x7FFFFFFF*/
int my_rand()
{
	return x=
		(x*0x2DAAB5+0x6496205) &0x7FFFFFFF;
}
