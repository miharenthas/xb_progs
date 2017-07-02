//This functions will apply (the calibration and) smear
//to a collection of probably simulated data.

#ifndef XB_SMEAR__H
#define XB_SMEAR__H

#include <vector>

#include "xb_error.h"
#include "xb_read_calib.h" //calinf structure
#include "xb_data.h"

namespace XB{
	int smear( std::vector<data> &xb_book, calinf *cryscalib );

	//...
}

#endif
