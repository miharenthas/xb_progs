//this function(s) perform the doppler correction on an event.
//Only on one event because we may want to run it in parallel.

#include "xb_doppler_corr.h"

namespace XB{
	//----------------------------------------------------------------------------
	//implementation of the class "b_interp"
	
	//----------------------------------------------------------------------------
	//very uninteresting default constructor
	b_interp::_xb_beta_0_interpolate():
		slope( 0 ),
		intercept( 0 )
	{}
	
	//very interesting custom constructor,
	//the one that's supposed to be used.
	//it takes in the tracker info and sets up a linear fit
	//(despite being called "interpolation" ) of the beta_0( in_beta ) function.
	b_interp::_xb_beta_0_interpolate( std::vector<track_info*> &xb_track_book ){
		const unsigned int N = xb_track_book.size();
		
		//first, populate the arrays
		double* in_betas = (double*)malloc( N*sizeof(double) );
		double* beta_0s = (double*)malloc( N*sizeof(double) );
		
		//check...
		if( in_betas == NULL || beta_0s == NULL )
			throw error( "Memory error!", "XB::b_interp" );
		
		//copy the various betas
		for( int i=0; i < N; ++i ){
			in_betas[i] = xb_track_book[i]->in_beta;
			beta_0s[i] = xb_track_book[i]->beta_0;
		}
		
		//do the fit
		gsl_fit_linear( in_betas, 1, beta_0s, 1, N,
		                &intercept, &slope,
		                &cov[0], &cov[1], &cov[3], &J );
		cov[2] = cov[1];
		
		//no use for the data any longer
		free( in_betas );
		free( beta_0s );
	}
	
	//nothing do destruct...
	b_interp::~_xb_beta_0_interpolate(){}
	
	//----------------------------------------------------------------------------
	//implementation of the main thing of this class
	float b_interp::operator()( const float in_beta ){
		return intercept + slope*in_beta;
	}
	
	//the other operator
	b_interp &b_interp::operator=( const b_interp &given ) {
		//copy all the stuff
		memcpy( cov, given.cov, 4*sizeof(double) );
		slope = given.slope;
		intercept = given.intercept;
		J = given.J;
		
		return *this;
	}
	
	//----------------------------------------------------------------------------
	//get methods
	double b_interp::get_slope(){ return slope; }
	double b_interp::get_intercept(){ return intercept; }
	double b_interp::get_slope_err(){ return sqrt( cov[3] ); }
	double b_interp::get_intercept_err(){ return sqrt( cov[0] ); }
	
	//----------------------------------------------------------------------------
	//implementation of the library functions
	
	//----------------------------------------------------------------------------
	//single energy deposit correction
	
	//----------------------------------------------------------------------------
	//implementation of the routine for doppler-correcting using crystal
	//index from which the beam exits
	void doppler_correct( data &evnt, float beta, unsigned int beam_line ){
		//get a ball...
		xb_ball the_cb;
		
		//get the angles
		float b_altitude = the_cb.at( beam_line ).altitude; //and of the beam's out
		float b_azimuth = the_cb.at( beam_line ).azimuth;
		
		//call the engine
		doppler_correct( evnt, beta, b_altitude, b_azimuth );
	}
	
	//----------------------------------------------------------------------------
	//implementation of the interface with a given versor
	void doppler_correct( data &evnt, float beta, versor &direction ){
		float b_altitude = asin( direction.k ); 
		float b_azimuth = asin( direction.j );
		
		//call the engine
		doppler_correct( evnt, beta, b_altitude, b_azimuth );
		
	}
	
	//----------------------------------------------------------------------------
	//implementation of the "engine"
	void doppler_correct( data &evnt, float beta, float b_altitude, float b_azimuth ){
		//get a ball...
		xb_ball the_cb;
	
		//get gamma from the beta
		float gamma = 1./sqrt( 1. - pow( beta, 2 ) );
		float altitude, azimuth, inclination; //the current crystal's angles, and
		                                      //its inclination from the beam line

		//loop on the energy deposits
		for( int i=0; i < evnt.n; ++i ){
			//check there's something to correct first
			//if both energies are set to 0, there's no point in
			//doing anything to them
			if( !evnt.e[i] && !evnt.he[i] ) continue;

			//get the angles of the current crystal
			try{
				altitude = the_cb.at( evnt.i[i] ).altitude;
				azimuth = the_cb.at( evnt.i[i] ).azimuth;
			} catch( XB::error e ){
				throw( XB::error( e.what , "XB::doppler_correct" ) );
			}
						
			//calculate the aperture from the beam line
			inclination = angular_distance( b_altitude, b_azimuth,
			                                altitude, azimuth );			
			
			//do the correction
			if( evnt.e[i] ) evnt.e[i] *= gamma*(1. - beta*cos( inclination ) );
			else evnt.he[i] *= gamma*(1. - beta*cos( inclination ) );
		}
	}

	//----------------------------------------------------------------------------
	//cluster correction
	
	//----------------------------------------------------------------------------
	//implementation of the routine for doppler-correcting using crystal
	//index from which the beam exits
	void doppler_correct( clusterZ &klz, float beta, unsigned int beam_line ){
		//get a ball...
		xb_ball the_cb;
		
		//get the angles
		float b_altitude = the_cb.at( beam_line ).altitude; //and of the beam's out
		float b_azimuth = the_cb.at( beam_line ).azimuth;
		
		//call the engine
		doppler_correct( klz, beta, b_altitude, b_azimuth );
	}
	
	//----------------------------------------------------------------------------
	//implementation of the interface with a given versor
	void doppler_correct( clusterZ &klz, float beta, versor &direction ){
		float b_altitude = asin( direction.k ); 
		float b_azimuth = asin( direction.j );
		
		//call the engine
		doppler_correct( klz, beta, b_altitude, b_azimuth );
		
	}
	
	//----------------------------------------------------------------------------
	//implementation of the "engine"
	void doppler_correct( clusterZ &klz, float beta, float b_altitude, float b_azimuth ){
	
		//get gamma from the beta
		float gamma = 1./sqrt( 1. - pow( beta, 2 ) );
		float altitude, azimuth, inclination; //the current crystal's angles, and
		                                      //its inclination from the beam line

		//loop on the energy deposits
		for( int i=0; i < klz.n; ++i ){
			//if the cluster is empty, jump
			if( !klz.clusters[i].n ) continue;

			//retrieve the coordinates of the centroid
			//NOTE: these may not be coincide with a crystal
			//      therefore it's better to get them directly
			//      from the cluster.
			altitude = klz.clusters[i].c_altitude;
			azimuth = klz.clusters[i].c_azimuth;
						
			//calculate the aperture from the beam line
			inclination = angular_distance( b_altitude, b_azimuth,
			                                altitude, azimuth );			
			
			//do the correction
			klz.clusters[i].sum_e *= gamma*(1. - beta*cos( inclination ) );
			for( int c=0; c < klz.clusters[i].n; ++c )
				klz.clusters[i].crys_e[c] *= gamma*(1. - beta*cos( inclination ) );
		}
	}
}
