//This functions will apply (the calibration and) smear
//to a collection of probably simulated data.

#ifndef XB_SMEAR__H
#define XB_SMEAR__H

#include <vector>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "xb_error.h"
#include "xb_read_calib.h" //calinf structure
#include "xb_data.h"

namespace XB{
	//----------------------------------------------------------------------------
	//the main thing:
	//NOTE: both XB::smear and Xb::calib are multithreaded if support for
	//      OpenMP is enabled.
	
	//the smear function. Given a calibration, this function applies
	//a GAUSSIAN smear according to the energy resolution.
	//NOTE: if an energy resolution is non constant --i.e. there are more
	//      than one point saved, data entries within the interpolation
	//      range will use interpolation, while outside of it will use
	//      a polynomial regression of it.
	int smear_gaussian( std::vector<data> &xb_book, const calinf *cryscalib );

	//the calib function applies the calibration to the data. Linear behavior
	//is assumed --kind of forcefully, since that's how the calibration info
	//is passed.
	//NOTE: morally, one should first calibrated and then smear, since that's
	//      the order in which things are done when calculating energy resolution
	//      and calibration. In practice, nothing will break (just possibly
	//      your data)
	int calib( std::vector<data> &xb_book, const calinf *cryscalib );
	
	//----------------------------------------------------------------------------
	//auxiliary functions
	
	//Get the (interpolated/fitted) energy resolution at a certain energy
	//NOTE: as for now, the interpolation/fit is recalculated every single
	//      call. This might change in the future.
	double get_dE_E_at( calinf &this_crystal_calib, double e );
}

#endif
