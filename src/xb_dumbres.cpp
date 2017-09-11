//this program is a preview of what xb_resolutor will do,
//but instead of being fully programmable, it will operate
//on the "defaults", as found in the original Crystal Ball
//TDR.

#include <stdio.h>
#include <getopt.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "xb_data.h"
#include "xb_io.h"

#define VERBOSE 0x00000001
#define IN_FLAG 0x00000002
#define OUT_FLAG 0x00000004

//------------------------------------------------------------------------------------
//global (sigh) flag bitfield
int flagger = 0;

//------------------------------------------------------------------------------------
//a couple of functions that will remind you of what you saw in xb_smear.h
//but are a simplified version of those.
void dumb_smear_gaussian( std::vector<XB::data> &xb_book );
inline float dumb_get_res_at( float e );
inline float dumb_get_sigma( const float res, const float e );

//------------------------------------------------------------------------------------
//the main
int main( int argc, char **argv ){
	char in_fname[64][256];
	char fcount = 0; //it just has to go up to 64...
	char out_fname[256];
	
	for( int i=1; i < argc; ++i ){
		if( argv[i][0] == '-' ) break;
		strncpy( in_fname[fcount], argv[i], 256 );
		++fcount;
		flagger = flagger | IN_FLAG;
	}
	
	//declare the options
	struct option opts[] = {
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "outfile", required_argument, NULL, 'o' },
		{ NULL, 0, NULL, 0 }
	};
	
	//and read them
	char iota = 0; int idx;
	while( ( iota = getopt_long( argc, argv, "vo:", opts, &idx ) ) != -1 ){
		switch( iota ){
			case 'v' :
				flagger = flagger | VERBOSE;
				break;
			case 'o' :
				strncpy( out_fname, optarg, 256 );
				flagger = flagger | OUT_FLAG;
				break;
		}
	}
	
	//begin to work. Standard greetings
	if( flagger & VERBOSE ) printf( "*** Welcome in dumbres, the dumb resolutor ***\n" );
	
	//load the data --and it has to be data.
	std::vector<XB::data> xb_book, data_buf;
	if( flagger & IN_FLAG ) for( int f=0; f < fcount; ++f ){
		if( flagger & VERBOSE ) printf( "Reading from file \"%s\"...\n", in_fname[f] );
		XB::load( in_fname[f], data_buf );
		xb_book.insert( xb_book.end(), data_buf.begin(), data_buf.end() );
	}
	else{
		if( flagger & VERBOSE ) printf( "Reading from STDIN..." );
		XB::load( stdin, xb_book );
	}
	
	//Doing the correction
	if( flagger & VERBOSE ) printf( "Applying finite resolution to data...\n" );
	dumb_smear_gaussian( xb_book );
	
	//put the data
	if( flagger & VERBOSE ) printf( "Done. Putting the data...\n" );
	if( flagger & OUT_FLAG ) XB::write( out_fname, xb_book );
	else XB::write( stdout, xb_book );
	
	if( flagger & VERBOSE ) printf( "Done. Goodbye.\n" );
	return 0;
}

//------------------------------------------------------------------------------------
//implementation of the get the energy resolution routine
inline float dumb_get_res_at( float e ){
	//resolution at 662KeV is .078
	//resolution at 1332KeV is .055
	//this means: A = 2.0059
	//            B = 3.9147e-05
	//for the formula
	// R( e ) = A/sqrt( e ) + B
	
	#define A 2.0059
	#define B 3.9147e-05
	
	return A/sqrt( e ) + B;
	
	#undef A
	#undef B
}

//------------------------------------------------------------------------------------
//get the sigma given the FWHM
inline float dumb_get_sigma( const float res, const float e ){
	return 0.4246609*res*e;
}

//------------------------------------------------------------------------------------
//this function is a simplified copy of smear_gaussian in xb_smear.h
void dumb_smear_gaussian( std::vector<XB::data> &xb_book ){
	gsl_rng *rng = gsl_rng_alloc( gsl_rng_ranlux );
	
	float dE_E, rnd_sample, sigma, *current_e;
	if( flagger & VERBOSE ) printf( "Processing entry 00000000" );
	for( int i=0; i < xb_book.size(); ++i ){
		if( flagger & VERBOSE ) printf( "\b\b\b\b\b\b\b\b%08d", i );
		if( xb_book[i].empty_e )
			current_e = xb_book[i].he;
		else
			current_e = xb_book[i].e;
			
		for( int c=0; c < xb_book[i].n; ++c ){
			dE_E = dumb_get_res_at( current_e[c] );
			sigma = dumb_get_sigma( dE_E, current_e[c] );
			rnd_sample = gsl_ran_gaussian( rng, sigma );
		
			//attach the randomization to the energy
			current_e[c] += rnd_sample;
		}
	}
	if( flagger & VERBOSE ) printf( " done.\n" );
	
	//cleanup
	gsl_rng_free( rng );
}
