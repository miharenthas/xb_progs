//this class (and structures) represents the crystal ball. As for now, it's a passtime to look for the nearest neighbour
#ifndef XB_BALL__H
#define XB_BALL__H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "xb_error.h"

namespace XB{
	enum crystal_shape_kind{
		PENTAGON = 0,
		HEX_1,
		HEX_2,
		HEX_3
	};

	//this structure represents a crystal.
	//for now, it's just a member of a multiply
	//linked list.
	typedef struct _xb_one_crystal {
		unsigned int i; //the index of the crystal
		crystal_shape_kind sh; //shape kind
		float altitude; //altitude of the crystal (radians)
		float azimuth; //azimuth of the crystal (radians)
		struct _xb_one_crystal *neigh[6]; //the neighbours (very important)
		struct _xb_one_crystal *opp; //the opposite crystal
	} xb_cry;

	//for now, the job of this class is just to compile the linked list
	typedef class _xb_ball_object{
		public:
			//ctor, dtor
			_xb_ball_object(); //default constructor
			~_xb_ball_object(); //defaut destructor
		
			//access method
			xb_cry &at( const unsigned int i );
			
			//crystals
			xb_cry ball[162];
	} xb_ball;
	
	//a function to find the up-to-nth order neigbours
	//returns an array of indeces
	unsigned int* neigh( xb_cry &crystal, unsigned int order, unsigned int &length );
}

#endif
