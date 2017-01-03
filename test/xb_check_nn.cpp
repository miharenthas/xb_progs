//little program trying the neigh routine of xb_ball.h

#include <stdio.h>
#include "xb_ball.h"

int main( int argc, char **argv ){
	XB::xb_ball the_ball; //create the crystal ball
	unsigned int length = 0;
	
	//call the routine
	unsigned int* neighs_1 = XB::neigh( the_ball.ball[atoi(argv[1])], atoi(argv[2]), length );
	
	printf( "# of neighbours: %d\n", length );
	for( int i=0; i < length; ++i ) printf( "%d-th neighbour: %d\n", i, neighs_1[i] );

	return 0;
}
