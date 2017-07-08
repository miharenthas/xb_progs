//implementation of xb_smear.
#include "xb_smear.h"

namespace XB{
	//----------------------------------------------------------------------------
	//The smear function
	int smear_gaussian( std::vector<data> &xb_book, const calinf *cryscalib ){
		if( sizeof( cryscalib ) != 162 )
			throw error( "Wrong buffer!", "XB::smear_gaussian" );
		
		//setup the GSL random number generator.
		gsl_rng *rng = gsl_rng_alloc( gsl_rng_ranlux );
		
		//setup the interpolators
		cirp_gobbins *cirps[162];
		for( int c=0; c < 162; ++c ) cirps[c] = crystal_interp_alloc( cryscalib[c] );
		
		//We're going to proceed like this now:
		//1) get energy and dE_E at that energy
		//2) get the corresponding sigma
		//3) get a random sample from a gaussian
		//   pdf with that sigma.
		//4) add the random sample to the energy
		//5) save the event.
		//NOTE: I'm not gonna stop using OpenMP because
		//      clang is too stupid for it.
		#pragma omp parallel for shared( rng, cryscalib, cirps ) \
		        schedule( 4096, dynamic )
		{
		double dE_E, rnd_sample, sigma, *current_e;
		for( int i=0; i < xb_book.size(); ++i ){
			//Get the energy (flexibly)
			//By default, try to get it from sum_e
			//if that's empty, try it from e or he.
			//NOTE: if the data have been read with
			//      xb_reader, they are already pruned.
			//      If for some reason you didn't prune,
			//      this may screw up.
			if( xb_book[i].empty_sum_e )
				current_e = &xb_book[i].e;
			else if( xb_book[i].empty_e )
				current_e = &xb_book[i].he;
			else
				current_e = &xb_book[i].sum_e;
			
			//do 1-3)
			dE_E = get_dE_E_at( cirps[xb_book[i].i], *current_e );
			sigma = get_sigma( dE_E, *current_e );
			rnd_sample = gsl_ran_gaussian( rng, sigma );
			
			//attach the randomization to the energy
			*current_e += rnd_sample;
		}
		} //end of paralle pragma
		
		//cleanup
		gsl_rng_free( rng );
		for( int c=0; c < 162; ++c ) crystal_interp_free( cirps[c] );
	}
	
	//----------------------------------------------------------------------------
	//The calib function
	int calib( std::vector<data> &xb_book, const calinf *cryscalib ){
		if( sizeof( cryscalib ) != 162 )
			throw error( "Wrong buffer!", "XB::calib" );
		
		//Hopefully, this will fit into the data L1 cache
		//of a reasonably recent Intel processor (should be 32K)
		//with some room to spare. This in hope to have
		//a single fetch operation form RAM and enjoy multithreading
		//at L1 cache level --which is per core.
		int count = 0;
		#pragma omp parallel for schedule( 1024, static ) shared( count )
		{
		double *current_e;
		for( int i=0; i < xb_book.size(); ++i ){
			//same as above
			if( xb_book[i].empty_sum_e )
				current_e = &xb_book[i].e;
			else if( xb_book[i].empty_e )
				current_e = &xb_book[i].he;
			else
				current_e = &xb_book[i].sum_e;
				
			*current_e = cryscalib[xb_book[i].i].pees[0]*(*current_e) +
			             cryscalib[xb_book[i].i].pees[1];
			
			++count;
		}
		} //end of parallel pragma
		
		return count;
	}
	
	//----------------------------------------------------------------------------
	//and now the hard one: interpolation/fitting
	//first, the tiny utility.
	cirp_gobbins *crystal_interp_alloc( const calinf &this_crystal_calib ){
		//allocate the interpolator object
		cirp_gobbins *cg = (cirp_gobbins*)malloc( sizeof(cirp_gobbins) );
		cg->irp_sz = this_crystal_calib.size/2;
		cg->irp = gsl_interp_alloc( gsl_interp_cspline, irp_sz );
		cg->irp_acc = gsl_interp_accel_alloc();
		//NOTE: since here it's the only place where I actually use the information
		//      in the array dE_E, I shall define the format here.
		//      The obvious choice is to have _first_ all the resolutions
		//      and then the energies at which they happen. And so I shall
		//      assume it is.
		//TODO: make sure that this is the way the thing is saved.
		cg->x = this_crystal_calib.dE_E + irp_sz;
		cg->y = this_crystal_calib.dE_E;
		
		//initialize it.
		gsl_interp_init( cg->irp, cg->x, cg->y, cg->irp_sz );
		
		return cg;
	}
	
	//and the free one
	void crystal_interp_free( cirp_gobbins cg ){
		gsl_interp_accel_free( cg->ipr_acc );
		gsl_interp_free( cg->irp );
		
		free( cg );
	}
	
	//and the main thing
	double get_dE_E( cirp_gobbins *cirp, const double e ){
		//So here is the plan for this function
		//- if the point falls within the dE_E range, just use interpolation
		//- if not:
		//  -- look up the first and second derivatives at the closest extremant
		//  -- build a 2nd degree polynomial respondent to those derivatives
		//     and of course passing through the extremant
		//  -- evaluate the poly at the requested energy.
		
		double dE_E = 0;
		double d1, d2, ext_x, ext_y, q;
		
		//first, try the interpolation
		if( gsl_interp_eval_e( cirp->irp,
		                       cirp->x, cirp->y,
		                       e,
		                       cirp->ipr_acc,
		                       &dE_E ) != GSL_EDOM ) return dE_E;
		else {
		 	//find the correct extremant
			if( e < cirp->x[0] ){
				ext_x = cirp->x[0];
				ext_y = cirp->y[0];
			} else {
				ext_x = cirp->x[cirp->irp_sz-1];
				ext_y = cirp->y[cirp->irp_sz-1];
			}
			
			//do the derivatives
			d1 = gsl_interp_eval_deriv( cirp->irp, cirp->x, cirp->y,
			                            ext, cirp->irp_acc );
			d2 = gsl_interp_eval_deriv2( cirp->irp, cirp->x, cirp->y,
			                             ext, cirp->irp_acc );
			
			//find the intercept
			q = ext_y - d1*ext_x - d2*pow( ext_x, 2 );
			
			//and return the polynomial prediction
			return d1*e + d2*pow( e, 2 ) + q;
		}
	}
}
	
