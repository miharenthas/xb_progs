//implementation of all the cut operating stuff

#include "xb_cut.h"

namespace XB{
	//============================================================================
	//classes:

	//============================================================================
	//the point holder... this may be overkill
	//but I'm allergic to objects that are born in a scope
	//and die in another.
	
	//----------------------------------------------------------------------------
	//constructors:
	//default
	_xb_pt_holder::_xb_pt_holder(){
		_pt[0] = 0;
		_pt[1] = 0;
	}
	
	//parametric
	_xb_pt_holder::_xb_pt_holder( double &x, double &y ){
		_pt[0] = x;
		_pt[1] = y;
	}
	
	//from an array
	_xb_pt_holder::_xb_pt_holder( double *pt ){
		memcpy( _pt, pt, 2*sizeof(double) );
	}
	
	//copy
	_xb_pt_holder::_xb_pt_holder( _xb_pt_holder &given ){
		memcpy( _pt, given._pt, 2*sizeof(double) );
	}
	
	//----------------------------------------------------------------------------
	//operators:
	
	//----------------------------------------------------------------------------
	//bracket access
	double &_xb_pt_holder::operator[]( unsigned int index ){
		return _pt[index%2]; //this has a cyclic behavior
		                     //it's a very coarse way to
		                     //avoid overruns
	}
	
	//----------------------------------------------------------------------------
	//assignment
	_xb_pt_holder &_xb_pt_holder::operator=( _xb_pt_holder &right ){
		memcpy( _pt, right._pt, 2*sizeof(double) );
	}
	
	//----------------------------------------------------------------------------
	//cast to double
	_xb_pt_holder::operator double*(){
		return _pt;
	}
} //end of namespace
