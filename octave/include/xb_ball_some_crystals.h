//header file for the routine for conversion of an array of indexes into
//an Octave's map-array of crystals

#ifndef O_XB_BALL_SOME_CRYSTALS__H
#define O_XB_BALL_SOME_CRYSTALS__H

#include "xb_ball.h"
#include "xb_error.h"

#include <octave/oct-map.h>
#include <octave/Array.h>

//takes in an array of indexes and its length and returns a map array.
octave_map xb_ball_some_crystals( const unsigned int *indexes, const unsigned int howmany );

#endif


