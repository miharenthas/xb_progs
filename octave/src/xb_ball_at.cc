//This GNU Octave interface provides a wrapper for the "at"
//method of the calss XB::ball: it will return the crystal's
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

DEFUN_DLD( xb_ball_at, args, , O_DOC_STRING ){
	unsigned int nargin = args.length(), howmany = 0;
	unsigned int *indexes = (unsigned int*)malloc( sizeof(int) );
	
	//work out how many and which crystal we have to return:
	//if we are given just one argument and it's a scalar,
	//return that.
	//If we are given an array, return an array of crystals.
	//if we are given many arguments, return according to those
	if( nargin == 1 && args(0).is_scalar_type() ){
		indexes[0] = args(0).int_value();
		howmany = 1;
	} else if( nargin == 1 && !args(0).is_scalar_type() ){
		uint32NDArray idx = args(0).uint32_array_value();
		howmany = idx.numel();
		indexes = (unsigned int*)realloc( indexes, howmany*sizeof(int) );
		memcpy( indexes, idx.fortran_vec(), howmany*sizeof(int) );
	} else if( nargin > 1 ){
		indexes = (unsigned int*)realloc( indexes, nargin*sizeof(int) );
		for( int i=0; i < nargin; ++i ){
			if( args(i).is_scalar_type() ){
				indexes[i] = args(i).int_value();
				++howmany;
			}
		}
	} else error( "Wrong combination of arguments." );
	
	//so, now let's get the crystals
	octave_map o_map;
	try{
		o_map = xb_ball_some_crystals( indexes, howmany );
	} catch( XB::error e ){
		error( "%s", e.what() );
	}
	
	return octave_value_list( o_map );
}
