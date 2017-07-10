//This functions will apply (the calibration and) smear
//to a collection of probably simulated data.

#ifndef XB_SMEAR__H
#define XB_SMEAR__H

#include <math.h>
#include <vector>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_interp.h>

#include "xb_error.h"
#include "xb_read_calib.h" //calinf structure
#include "xb_data.h"

namespace XB{
	//A structure to hold the relevant bits for the
	//interpolation
	//NOTE: it's just a collection of pointers,
	//      and the two doubles are NOT owned.
	typedef struct _crystal_interpolation_gobbins{
		gsl_interp *irp;
		gsl_interp_accel *irp_acc;
		double *x;
		double *y;
		int irp_sz;
	} cirp_gobbins;

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
	double get_dE_E_at( cirp_gobbins *crystal_interp, const double e );
	//Utilities to allocate (and init) and free the gobbins
	cirp_gobbins *crystal_interp_alloc( const calinf &this_crystal_calib );
	void crystal_interp_free( cirp_gobbins *crystal_interp );
	
	//Get sigma from an energy resolution
	double get_sigma( const double dE_E, const double e );
}

#endif
