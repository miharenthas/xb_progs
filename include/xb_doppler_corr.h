//this is the header file for the doppler correction routine

#ifndef XB_DOPPLER_CORR__H
#define XB_DOPPLER_CORR__H

#include <math.h>
#include <string.h>
#include <functional>
#include <vector>

#include <gsl/gsl_fit.h>

#include "xb_error.h"
#include "xb_data.h"
#include "xb_ball.h"
#include "xb_cluster.h" //this is for the angular distance function,
                        //which is there for historical reasons.

namespace XB{
	//----------------------------------------------------------------------------
	//this functional converts in_betas to reasonable beta_0
	typedef class _xb_beta_0_interpolate : public std::unary_function< float, float > {
		public:
			//ctors
			_xb_beta_0_interpolate();
			_xb_beta_0_interpolate( std::vector<track_info*>& );
			~_xb_beta_0_interpolate();
			
			//a couple of get methods
			double get_slope();
			double get_intercept();
			double get_slope_err();
			double get_intercept_err();
			
			//the main thing:
			float operator()( float in_beta );
			
			//assignment op
			_xb_beta_0_interpolate &operator=( const _xb_beta_0_interpolate& );
		private:
			//variables and stuff
			double cov[4]; //data, and covariance matrix
			double slope, intercept, J; //self explaining and sum of the squares
	} b_interp;
	
	//-----------------------------------------------------------------------------
	//this function takes ONE event,
	//the relativistic beta relative to that event
	//and the crystal where the beam exits for that event.
	void doppler_correct( data &evnt, float beta, versor &directon ); //with the tracker's versor
	void doppler_correct( data &evnt, float beta, unsigned int beam_line ); //with just the beam line
	void doppler_correct( data &evnt, float beta, float b_altitude, float b_azimuth ); //with the angles
	
	//these perform the same action, but on clusters.
	void doppler_correct( clusterZ &klz, float beta, versor &directon ); //with the tracker's versor
	void doppler_correct( clusterZ &klz, float beta, unsigned int beam_line ); //with just the beam line
	void doppler_correct( clusterZ &klz, float beta, float b_altitude, float b_azimuth ); //with the angles
	
}

#endif
