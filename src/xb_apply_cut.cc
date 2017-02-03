//implementation of the cut data extractor shorhands
//declared in xb_apply_cut.h header

#include "xb_apply_cut.h"

namespace XB{

	//============================================================================
	//1D classes
	
	//----------------------------------------------------------------------------
	//cut on fragment number
	double cut_on_fragment_nb::operator()( unsigned int index ){
		return (double)_data_array.at( index ).n;
	}
	
	//----------------------------------------------------------------------------
	//cut on the inbeta
	double cut_on_inbeta::operator()( unsigned int index ){
		return _data_array.at( index ).in_beta;
	}
	
	//----------------------------------------------------------------------------
	//cut on beta_0
	double cut_on_beta_0::operator()( unsigned int index ){
		return _data_array.at( index ).beta_0;
	}
	
	//============================================================================
	//2D classes
	
	//----------------------------------------------------------------------------
	//the standard cut on charge and charge to mass ratio
	//which is expected to be largely the only one used intesively
	pt_holder cut_on_zaz::operator()( unsigned int index ){
		double inz = _data_array.at( index ).in_Z;
		double inaonz = _data_array.at( index ).in_A_on_Z;
		return pt_holder( inz, inaonz );
	}
}
