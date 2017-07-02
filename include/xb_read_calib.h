//This functions and structures will handle calibration information
//for the crystals (in principle, obrained with the "cryscalib" script
//but the format should be sensible enough to be general).

#ifndef XB_READ_CALIB__H
#define XB_READ_CALIB__H

#include <stdio.h>
#include <string.h>

#include "xb_error.h"

namespace XB{
	//----------------------------------------------------------------------------
	//the data structure
	typedef struct _xb_crystal_calibration_info {
		float _pees[2]; //linear calibration paramenters
		float _perr[2]; //errors associated
		float *dE_E; //a pointer to an array of resolution information
		short int size; //the size of dE_E (2 bytes is enough)
	} calinf;
	
	//associated allocation/deallocation utils
	int calinf_alloc( calinf &given, int size ); //size refers to how big
	                                             //dE_E has to be.
	void calinf_free( caling &given );
	
	//----------------------------------------------------------------------------
	//reader function
	//NOTE: cryscalib is a 162-element array (array, not a vector)
	//      pre allocated and that will be populated reading from
	//      stream. There's no point in being clever here: the number
	//      of crystal is fixed. Empty crystals will be 0-ed
	int read_calib( calinf *cryscalib, FILE *stream );
} 

#endif
