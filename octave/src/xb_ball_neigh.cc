//This GNU Octave interface provides a wrapper for the "neigh"
//function defined in xb_ball: it will return the crystal's
//information, such as pointers to the neighbours
//and coordinates of the centroid.
#define O_DOC_STRING "..."

#include <stdlib.h>
#include <string.h> //memcpy

#include "xb_ball.h"
#include "xb_error.h"
#include "xb_ball_some_crystals.h"

#include <octave/oct.h>
#include <octave/oct-map.h>

DEFUN_DLD( xb_ball_neigh, args, , O_DOC_STRING ){
	unsigned int nargin = args.length(), howmany = 0;
	int *indexes = (int*)malloc( sizeof(int) );
	
	//work out how many and which crystal we have to return:
	//if we are given just one argument and it's a scalar,
	//return that.
	//If we are given an array, return an array of crystals.
	//if we are given many arguments, return according to those
	if( nargin == 2 && args(0).is_integer_type() ){
		indexes[0] = args(0).int_value();
		howmany = 1;
	} else if( nargin == 2 && !args(0).is_scalar_type() && args(0).is_integer_type() ){
		int32NDArray idx = args(0).int32_array_value();
		howmany = idx.numel();
		indexes = (int*)realloc( indexes, howmany*sizeof(int) );
		memcpy( indexes, idx.fortran_vec(), howmany*sizeof(int) );
	} else if( nargin > 2 ){
		indexes = (int*)realloc( indexes, nargin*sizeof(int) );
		for( int i=0; i < nargin-1; ++i ){
			if( args(i).is_integer_type() )	indexes[i] = args(i).int32_array_value()(i);
		}
	} else error( "Wrong combination of arguments." );
	
	int order = args(nargin-1).int_value();

	//check if we have too many arguments
	if( howmany > 162 ){
		error( "There are only 162 crystals: with %d neighbour maps requested you are for sure asking some more than once. Review your choices.", howmany );
	}

	//loop make the maps, and return them
	unsigned int *neighs, n_neighs;
	octave_value_list o_list;
	XB::xb_ball the_ball;
	for( int i=0; i < howmany; ++i ){
		neighs = XB::neigh( the_ball.at(indexes[i]), order, n_neighs );
		o_list.append( xb_ball_some_crystals( neighs, n_neighs ) );
	}
	
	return o_list;
}
		
