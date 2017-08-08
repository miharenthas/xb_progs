//this program does a spectrum based on the clusters

#include <unistd.h>
#include <getopt.h> //getopt long
#include <stdio.h>
#include <string.h>
#include <omp.h>

#include <vector>
#include <algorithm>
#include <functional>

#include <gsl/gsl_histogram.h>
extern "C"{
	#include "gnuplot_i.h"
}

#include "xb_draw_gsl_histogram.h"
#include "xb_cluster.h"
#include "xb_io.h"

#include "xb_make_spc/cmd_line.h"
#include "xb_make_spc/selectors.h"
#include "xb_make_spc/xb_make_spc.h"


//------------------------------------------------------------------------------------
//the main
int main( int argc, char **argv ){

	//default settings
	p_opts settings;
	settings.drone_flag = false;
	settings.in_flag = false;
	settings.out_flag = false;
	settings.draw_flag = false;
	settings.verbose = false;
	settings.interactive = false;
	settings.histo_mode = XB::JOIN;
	settings.in_f_count = 0;
	settings.num_bins = 500;
	settings.target_mul = 0;
	settings.target_cry = 0;
	settings.target_ctr = 0;
	settings.target_alt = 180;
	settings.target_azi = 180;
	settings.target_nrg = 0;
	settings.mol_mul = MORE;
	settings.mol_cry = MORE;
	settings.mol_ctr = MORE;
	settings.mol_alt = MORE;
	settings.mol_azi = MORE;
	settings.mol_nrg = MORE;
	settings.range[0] = 0;
	settings.range[1] = 8000;
	settings.gp_opt.term = XB::QT;
	settings.gp_opt.is_log = true;
	strcpy( settings.gp_opt.x_label, "KeV" );
	strcpy( settings.gp_opt.title, "XB_spectrum" );
	strcpy( settings.drone.instream, "stdin" );
	strcpy( settings.drone.outstream, "stdout" );
	settings.drone.in = stdin;
	settings.drone.out = stdout;
	
	//the program's program
	unsigned short breaker = DO_LOAD | DO_HACK_DATA | DO_POP_HISTO | DO_SAVE_HISTO | DO_EXIT;
	
	//some strings...
	char command[512], comm[512];

	//input parsing
	//first look at the first unminused entry
	for( int i=1; i < argc && i < 64; ++i ){
		if( argv[i][0] != '-' && strlen( argv[i] ) < 256 ){
			strcpy( settings.in_fname[i-1], argv[i] );
			++settings.in_f_count;
			settings.in_flag = true;
		} else if( argv[i][0] == '-' ) break; //exit the loop at the first option
		                                      //and silently discard file names that
		                                      //are too long.
	}

	//long options
	struct option opts[] = {
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o' },
		{ "save-data-instead", no_argument, NULL, 'S' },
		{ "nb-bins", required_argument, NULL, 'b' },
		{ "range", required_argument, NULL, 'R' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "interactive", no_argument, NULL, 'I' },
		{ "gnuplot-term", required_argument, NULL, 's' },
		{ "y-scale", required_argument, NULL, 'l' },
		{ "title", required_argument, NULL, 't' },
		{ "histogram-mode", required_argument, NULL, 'H' },
		{ "select-multiplicity", required_argument, NULL, 'm' },
		{ "select-centroid", required_argument, NULL, 'c' },
		{ "drone", required_argument, NULL, 'D' },
		{ "draw", no_argument, NULL, 'd' },
		{ NULL, no_argument, NULL, 0 }
	};

	//input parsing
	char iota = 0; int mute;
	while( (iota = getopt_long( argc, argv, "i:o:Sb:R:vIs:l:t:m:H:c:Dd", opts, &mute )) != -1 ){
		switch( iota ){
			char *token_bf;
			case 'i':
				if( strlen( optarg ) < 256 ){
					strcpy( settings.in_fname[0], optarg );
					settings.in_f_count = 1;
					settings.in_flag = true;
				}else{
					printf( "File name too long!\n" );
					exit( 1 );
				}
				break;
			case 'o':
				if( strlen( optarg ) < 256 ){
					strcpy( settings.out_fname, optarg );
					settings.out_flag = true;
				}else{
					printf( "File name too long!\n" );
					exit( 1 );
				}
				break;
			case 'S':
				breaker = breaker ^ DO_SAVE_HISTO;
				breaker = breaker | DO_SAVE_DATA;
				break;
			case 'b':
				settings.num_bins = atoi( optarg );
				break;
			case 'R':
				sscanf( optarg, "%f:%f", &settings.range[0], &settings.range[1] );
				break;
			case 'v':
				settings.verbose = true;
				break;
			case 'I':
				settings.interactive = true;
				settings.verbose = true;
				break;
			case 's':
				if( !strcmp( optarg, "qt" ) ) settings.gp_opt.term = XB::QT;
				else if( !strcmp( optarg, "png" ) ) settings.gp_opt.term = XB::PNG;
				else{
					printf( "Invalid gnuplot terminal.\n" );
					exit( 1 );
				}
				break;
			case 'l':
				if( !strcmp( optarg, "yes" ) ) settings.gp_opt.is_log = true;
				else if( !strcmp( optarg, "no" ) ) settings.gp_opt.is_log = false;
				else{
					printf( "Invalid plot scale.\n" );
					exit( 1 );
				}
				break;
			case 't':
				if( strlen( optarg ) < 256 ) strcpy( settings.gp_opt.title, optarg );
				else{
					printf( "Title is too long.\n" );
					exit( 1 );
				}
				break;
			case 'H':
				if( !strcmp( optarg, "compare" ) ) settings.histo_mode = XB::COMPARE;
				else if( !strcmp( optarg, "subtract" ) ) settings.histo_mode = XB::SUBTRACT;
				else settings.histo_mode = XB::JOIN;
				break;
			case 'm':
				settings.target_mul = atoi( optarg );
				break;
			case 'c' :
				settings.target_ctr = atoi( optarg );
				break;
			case 'D' : //drone setup
				settings.drone_flag = true;
				#define PREP_TK_BF token_bf = strtok( NULL, ":" );\
				                   if( token_bf == NULL ) exit( 7 );\
				                   while( *token_bf == ' ' ) token_bf++;\
				
				//parse the drone
				token_bf = strtok( optarg, ":" );
				if( token_bf == NULL ) exit( 7 );
				while( *token_bf == ' ' ) ++token_bf;
	
				sscanf( token_bf, "%1s", &settings.drone.in_pof ); PREP_TK_BF
				strcpy( settings.drone.instream, token_bf ); PREP_TK_BF
				sscanf( token_bf, "%1s", &settings.drone.out_pof ); PREP_TK_BF
				strcpy( settings.drone.outstream, token_bf );
				#undef PREP_TK_BF
				
				//open the drone
				if( !strcmp( settings.drone.instream, "stdin" ) ) settings.drone.in = stdin;
				else settings.drone.in = ( settings.drone.in_pof == 'p' )?
				                         popen( settings.drone.instream, "r" ) :
				                         fopen( settings.drone.instream, "r" );
				if( !strcmp( settings.drone.outstream, "stdout" ) ) settings.drone.out = stdout;
				else settings.drone.out = ( settings.drone.out_pof == 'p' )?
				                          popen( settings.drone.outstream, "w" ) :
				                          fopen( settings.drone.outstream, "w" );
				                          
				//if the drone is in some way broken, die immediately with the correct answer.
				if( settings.drone.in == NULL || settings.drone.out == NULL ){
					fprintf( stderr, "FATAL: drone throughtput is broken.\n" );
					exit( 42 );
				}
				break;
			case 'd':
				settings.draw_flag = true;
				break;
			default :
				exit( 0 );
				//help note will come later
		}
	}
	
	//consistency checks
	if( settings.interactive && !settings.in_flag ){
		fprintf( stderr, "Cannot be interactive and read from a pipe...\n" );
		exit( 1 );
	}

	if( settings.verbose ) printf( "*** Welcome in the spectrum making program! ***\n" );
	
	xb_make_spc da_prog;
	char *msg = (char*)calloc( 128, 1 );
	
	//command line and program object fingering
	try{
		if( settings.interactive ) breaker = XB::cml_loop_prompt( stdin, settings );
		else if( settings.drone_flag ) breaker = XB::cml_loop( settings.drone.in, settings );
		else{
			da_prog.reset( settings );
			da_prog.exec( breaker );
		}
	} catch( XB::error e ){
		strcpy( msg, e.what() );
		msg = strtok( msg, "!" );
		fprintf( stderr, "YOU made a mistake: %s\n", msg );
		goto __FROM_HERE__;
	}
	if( settings.draw_flag ) da_prog.draw_histogram();
	
	while( !( breaker & DO_EXIT ) && ( settings.interactive || settings.drone_flag ) ){
		da_prog.reset( settings );
		da_prog.exec( breaker );
		if( settings.draw_flag ) da_prog.draw_histogram();
		
		__FROM_HERE__:
		try{
			if( settings.interactive ) breaker = XB::cml_loop_prompt( stdin, settings );
			else if( settings.drone_flag ) breaker = XB::cml_loop( settings.drone.in, settings );
		} catch( XB::error e ){
			strcpy( msg, e.what() );
			msg = strtok( msg, "!" );
			fprintf( stderr, "YOU made a mistake: %s\n", msg );
			goto __FROM_HERE__;
		}
	}
	
	//final ops
	if( settings.verbose ) printf( "Exiting...\n" );
	if( settings.drone_flag ){
		( settings.drone.in_pof == 'p' )? pclose( settings.drone.in ) : fclose( settings.drone.in );
		( settings.drone.out_pof == 'p' )? pclose( settings.drone.out ) : fclose( settings.drone.out );
	}
	return 0;
}

//------------------------------------------------------------------------------------
//implementation of the program's object

//------------------------------------------------------------------------------------
//ctors, dtor
xb_make_spc::xb_make_spc() {}; //default constructor, do nothing

xb_make_spc::xb_make_spc( p_opts &sts ){
	//copy the stuff
	settings.drone_flag = sts.drone_flag;
	settings.in_flag = sts.in_flag;
	settings.out_flag = sts.out_flag;
	settings.verbose = sts.verbose;
	settings.interactive = sts.interactive;
	settings.in_f_count = sts.in_f_count;
	settings.num_bins = sts.num_bins;
	settings.target_mul = sts.target_mul;
	settings.target_ctr = sts.target_ctr;
	settings.target_cry = sts.target_cry;
	settings.target_alt = sts.target_alt;
	settings.target_azi = sts.target_azi;
	settings.target_nrg = sts.target_nrg;
	settings.mol_mul = sts.mol_mul;
	settings.mol_ctr = sts.mol_ctr;
	settings.mol_cry = sts.mol_cry;
	settings.mol_alt = sts.mol_alt;
	settings.mol_azi = sts.mol_azi;
	settings.mol_nrg = sts.mol_nrg;
	settings.gp_opt.term = sts.gp_opt.term;
	settings.gp_opt.is_log = sts.gp_opt.is_log;
	strcpy( settings.gp_opt.x_label, sts.gp_opt.x_label );
	settings.histo_mode = sts.histo_mode;
	
	if( settings.drone_flag ){
		strcpy( settings.drone.instream, sts.drone.instream );
		strcpy( settings.drone.outstream, sts.drone.outstream );
		settings.drone.in_pof = sts.drone.in_pof;
		settings.drone.out_pof = sts.drone.out_pof;
		settings.drone.in = sts.drone.in;
		settings.drone.out = sts.drone.out;
	}
	
	//copy the strings
	for( int i=0; i < settings.in_f_count; ++i )
		strcpy( settings.in_fname[i], sts.in_fname[i] );
	
	strcpy( settings.out_fname, sts.out_fname );
	strcpy( settings.gp_opt.title, sts.gp_opt.title );
	
	settings.range[0] = sts.range[0];
	settings.range[1] = sts.range[1];
	
	//can't compare or subtract less than two histograms
	if( settings.in_f_count <= 1 ) settings.histo_mode = XB::JOIN;
	
	//set the do stuff switches
	
	load_files(); //and issues the file loading.
}

xb_make_spc::~xb_make_spc(){
	for( int i=0; i < settings.in_f_count; ++i ){
		event_klZ[i].clear();
		if( histo[i] != NULL ) gsl_histogram_free( histo[i] );
	}
	
	//and very importantly, close gnuplot
	if( gp_h != NULL ) gnuplot_close( gp_h );
}

//------------------------------------------------------------------------------------
//the program's processor
void xb_make_spc::exec( short unsigned prog ){
	if( prog & DO_UNLOAD ) unload_files();
	if( prog & DO_LOAD ) load_files();
	if( prog & DO_HACK_DATA ) hack_data();
	if( prog & DO_POP_HISTO ) populate_histogram();
	if( prog & DO_PUT_HISTO ) put_histogram();
	if( prog & DO_PUT_DATA ) put_data();
	if( prog & DO_SAVE_HISTO ) save_histogram();
	if( prog & DO_SAVE_DATA ) save_data();
}

//------------------------------------------------------------------------------------
//file loader
void xb_make_spc::load_files(){
	if( settings.verbose && !settings.in_flag ) printf( "Reading from STDIN...\n" );
	
	//get the clusters from somewhere
	for( int i=0; i < settings.in_f_count && settings.in_flag; ++i ){
		if( settings.verbose && settings.in_flag )
			printf( "Reading from: %s...\n", settings.in_fname[i] );
		XB::load( settings.in_fname[i], event_klZ[i] );
	}
	
	if( !settings.in_flag ) XB::load( stdin, event_klZ[0] );
}

//------------------------------------------------------------------------------------
//file unloader
void xb_make_spc::unload_files(){
	if( settings.verbose ) puts( "Clearing data." );
	for( int i=0; i < settings.in_f_count && settings.in_flag; ++i )
		event_klZ[i].clear();
}
		

//------------------------------------------------------------------------------------
//data selector
//for the multipliticy
void xb_make_spc::target_multiplicity( moreorless m ){
	//create a comparison functional 
	XB::is_multiplicity is_mul( settings.target_mul, m ); //MORE --> return 0 on true
	std::vector<XB::clusterZ>::iterator last;
	
	//find all the non interesting multiplicities
	for( int i=0; i < settings.in_f_count; ++i ){
		last = std::remove_if( event_klZ[i].begin(), event_klZ[i].end(), is_mul );
		
		if( last == event_klZ[i].begin() ){ event_klZ[i].clear(); continue; }
		else if( last == event_klZ[i].end() ) continue;
		event_klZ[i].erase( last, event_klZ[i].end() );
	}
}

//------------------------------------------------------------------------------------
//more advanced data cutter
void xb_make_spc::select( XB::selsel selector_type, moreorless m ){
	//in order to make good use of polymorphism
	XB::xb_selector *in_kl_selector;
	switch( selector_type ){
		case XB::IS_MULTIPLICITY : //just do the old stuff
			target_multiplicity( m );
			settings.target_mul = 0;
			settings.mol_mul = MORE;
			break;
		case XB::IS_CENTROID :
			target_field< XB::is_centroid >( XB::is_centroid( settings.target_ctr, m ) );
			settings.target_ctr = 0;
			settings.mol_ctr = MORE;
			break;
		case XB::IS_MORE_CRYSTALS :	
			target_field< XB::is_more_crystals >( XB::is_more_crystals( settings.target_cry, m ) );
			settings.target_cry = 0;
			settings.mol_cry = MORE;
			break;
		case XB::IS_MORE_ALTITUDE :
			target_field< XB::is_more_altitude >( XB::is_more_altitude( settings.target_alt, m ) );
			settings.target_alt = 180;
			settings.mol_alt = MORE;
			break;
		case XB::IS_MORE_AZIMUTH :
			target_field< XB::is_more_azimuth >( XB::is_more_azimuth( settings.target_azi, m ) );
			settings.target_azi = 180;
			settings.mol_azi = MORE;
			break;
		case XB::IS_MORE_NRG :
			target_field< XB::is_more_energy >( XB::is_more_energy( settings.target_nrg, m ) );
			settings.target_nrg = 0;
			settings.mol_nrg = MORE;
			break;
		default :
			throw XB::error( "Wrong selector type!", "xb_make_spc::select" );
			break;
	}
}

//------------------------------------------------------------------------------------
//the actual selector actuator
template< class the_selector >
void xb_make_spc::target_field( const the_selector &in_kl_selector ){
	std::vector<XB::cluster>::iterator k_last;
	std::vector<XB::clusterZ>::iterator klz_last;
	std::vector<XB::cluster> *kl;
	XB::is_multiplicity chopper( 0, MORE );
	
	//remove the unwanted data (yes, the policy has changed)
	for( int f=0; f < settings.in_f_count; ++f ){
		for( int k=0; k < event_klZ[f].size(); ++k ){
			kl = &event_klZ[f][k].clusters; //useful aliasing
			
			k_last = std::remove_if( kl->begin(), kl->end(), in_kl_selector );
			
			if( k_last == kl->begin() ){ //mark for deletion
				event_klZ[f][k].n = 0;
				kl->clear();
				continue;
			} else if( k_last == kl->end() ) continue; //do nothing if keep all
			kl->erase( k_last, kl->end() );
			event_klZ[f][k].n = kl->size();
		}
		
		//remove those marked for deletion (and generally 0 multiplicities).
		if( settings.verbose ) printf( "Events before cut on file %s: %d.\n",
		    settings.in_fname[f], event_klZ[f].size() );
		klz_last = std::remove_if( event_klZ[f].begin(), event_klZ[f].end(), chopper );
		event_klZ[f].erase( event_klZ[f].begin(), klz_last );
		if( settings.verbose) printf( "Events after cut on file %s: %d.\n",
		    settings.in_fname[f], event_klZ[f].size() ); 
	}
}

//------------------------------------------------------------------------------------
//reset the object
void xb_make_spc::reset( p_opts &sts ){
	//smart copy the stuff
	//don't toch the drone, if so: die.
	if( settings.drone_flag != sts.drone_flag ){
		fprintf( stderr, "Can't touch drone!\n" );
		exit( 42 );
	}
	
	//nothing to think about here
	settings.in_flag = sts.in_flag;
	settings.out_flag = sts.out_flag;
	settings.verbose = sts.verbose;
	settings.interactive = sts.interactive;
	
	//if the number of files has changed, redo everything
	if( settings.in_f_count != sts.in_f_count ){
		settings.in_f_count = sts.in_f_count;
	}
	
	//if the number of bins has changed, redo the histogram
	if( settings.num_bins != sts.num_bins ){
		settings.num_bins = sts.num_bins;
	}
	
	if( memcmp( &settings.target_mul, &sts.target_mul,
	            6*( sizeof(moreorless) + sizeof(unsigned int) ) ) ){
		memcpy( &settings.target_mul, &sts.target_mul,
		        6*( sizeof(moreorless) + sizeof(unsigned int) ) );
		/*settings.target_mul = sts.target_mul;
		settings.target_cry = sts.target_cry;
		settings.target_ctr = sts.target_ctr;
		settings.target_alt = sts.target_alt;
		settings.target_azi = sts.target_azi;
		settings.target_nrg = sts.target_nrg;
		settings.mol_mul = sts.mol_mul;
		settings.mol_cry = sts.mol_cry;
		settings.mol_ctr = sts.mol_ctr;
		settings.mol_alt = sts.mol_alt;
		settings.mol_azi = sts.mol_azi;
		settings.mol_nrg = sts.mol_nrg;*/
	}
	
	settings.gp_opt.term = sts.gp_opt.term;
	settings.gp_opt.is_log = sts.gp_opt.is_log;

	strcpy( settings.gp_opt.x_label, sts.gp_opt.x_label );
	strcpy( settings.gp_opt.title, sts.gp_opt.title );
	
	if( settings.histo_mode != sts.histo_mode ){
		settings.histo_mode = sts.histo_mode;
	}
	
	if( settings.drone_flag ){
		strcpy( settings.drone.instream, sts.drone.instream );
		strcpy( settings.drone.outstream, sts.drone.outstream );
		settings.drone.in_pof = sts.drone.in_pof;
		settings.drone.out_pof = sts.drone.out_pof;
		settings.drone.in = sts.drone.in;
		settings.drone.out = sts.drone.out;
	}
	
	//copy the strings
	for( int i=0; i < settings.in_f_count; ++i ){
		if( strcmp( settings.in_fname[i], sts.in_fname[i] ) ){
			strcpy( settings.in_fname[i], sts.in_fname[i] );
		}
	}
	
	strcpy( settings.out_fname, sts.out_fname );
	
	settings.range[0] = sts.range[0];
	settings.range[1] = sts.range[1];
}

//------------------------------------------------------------------------------------
//implementation of the drawing function
void xb_make_spc::draw_histogram(){
	if( !histo[0] ){ fprintf( stderr, "No histogram. Maybe run \"hist; go\"?\n" ); return; }
	if( gp_h != NULL ) gnuplot_close( gp_h );

	strcpy( settings.gp_opt.out_fname, settings.out_fname );

	gp_h = XB::draw_gsl_histogram( histo, settings.gp_opt,
	                               settings.in_f_count,
	                               settings.histo_mode );	

	if( settings.verbose ){
		printf( "Issuing GNUplot command...\n" );
		for( int f=0; f < settings.in_f_count && settings.histo_mode == XB::COMPARE; ++f )
			printf( "\tWith %d from file \"%s\"\n", f+1, settings.in_fname[f] );
	}

}

//------------------------------------------------------------------------------------
//implementation of the histogram population
void xb_make_spc::populate_histogram(){

	switch( settings.histo_mode ){
		case XB::JOIN:
			if( settings.verbose ){ //produce one sum histogram
				printf( "Populating the histogram:\n" );
				printf( "\t#channels: %u\n", settings.num_bins );
				printf( "\tEvent_multiplicity: %u\n", settings.target_mul );
				printf( "\tRange (KeV): %f:%f\n", settings.range[0], settings.range[1] );
			}

			//do the histogram
			if( histo[0] != NULL ) gsl_histogram_free( histo[0] );
			histo[0] = gsl_histogram_alloc( settings.num_bins );
			gsl_histogram_set_ranges_uniform( histo[0], settings.range[0], settings.range[1] );
	
			//populate the histogram
			for( int i=0; i < settings.in_f_count; ++i ){
				for( std::vector<XB::clusterZ>::iterator klZ = event_klZ[i].begin();
					 klZ != event_klZ[i].end(); ++klZ ){
					for( int k=0;
					     k < ( settings.target_mul ? settings.target_mul : klZ->n );
					     ++k ){
						gsl_histogram_accumulate( histo[0], klZ->clusters[k].sum_e, 1. );
					}
				}
			}
			break;
		case XB::COMPARE:
			for( int i=0; i < settings.in_f_count; ++i ){
				if( settings.verbose ){ //produce many histograms that will be compared
				printf( "Populating the histogram #%d:\n", i );
				printf( "\t#channels: %u\n", settings.num_bins );
				printf( "\tEvent_multiplicity: %u\n", settings.target_mul );
				printf( "\tRange (KeV): %f:%f\n", settings.range[0], settings.range[1] );
				}
				
				//do the histogram
				if( histo[i] != NULL ) gsl_histogram_free( histo[i] );
				histo[i] = gsl_histogram_alloc( settings.num_bins );
				gsl_histogram_set_ranges_uniform( histo[i], settings.range[0], settings.range[1] );
				
				//populate the histograms
				for( std::vector<XB::clusterZ>::iterator klZ = event_klZ[i].begin();
					 klZ != event_klZ[i].end(); ++klZ ){
					for( int k=0;
					     k < ( settings.target_mul ? settings.target_mul : klZ->n );
					     ++k ){
						gsl_histogram_accumulate( histo[i], klZ->clusters[k].sum_e, 1. );
					}
				}
			}
			break;
		case XB::SUBTRACT:
			if( settings.verbose ){ //produce one histogram, subtracted
				printf( "Populating the histogram:\n" );
				printf( "\t#channels: %u\n", settings.num_bins );
				printf( "\tEvent_multiplicity: %u\n", settings.target_mul );
				printf( "\tRange (KeV): %f:%f\n", settings.range[0], settings.range[1] );
			}

			//do the histogram
			if( histo[0] != NULL ) gsl_histogram_free( histo[0] );
			histo[0] = gsl_histogram_alloc( settings.num_bins );
			gsl_histogram_set_ranges_uniform( histo[0], settings.range[0], settings.range[1] );
	
			//populate the histogram:
			//add the values of the first files
			for( std::vector<XB::clusterZ>::iterator klZ = event_klZ[0].begin();
				 klZ != event_klZ[0].end(); ++klZ ){
				for( int k=0;
				     k < ( settings.target_mul ? settings.target_mul : klZ->n );
				     ++k ){
					gsl_histogram_accumulate( histo[0], klZ->clusters[k].sum_e, 1. );
				}
			}
			
			//subtract the consequents:
			for( int i=1; i < settings.in_f_count; ++i ){
				for( std::vector<XB::clusterZ>::iterator klZ = event_klZ[i].begin();
					 klZ != event_klZ[i].end(); ++klZ ){
				for( int k=0;
				     k < ( settings.target_mul ? settings.target_mul : klZ->n );
				     ++k ){
						gsl_histogram_accumulate( histo[0], klZ->clusters[k].sum_e, -1. );
					}
				}
			}
			
			break;
	}
	
	if( settings.verbose ) printf( "Done.\n" );
}

//------------------------------------------------------------------------------------
//A function to apply the cut to the data (in sequence, not clever right now)
void xb_make_spc::hack_data(){
	if( settings.verbose ) puts( "Hacking data." );
	
	if( settings.target_mul ) select( XB::IS_MULTIPLICITY, settings.mol_mul );
	if( settings.target_cry ) select( XB::IS_MORE_CRYSTALS, settings.mol_cry );
	if( settings.target_ctr ) select( XB::IS_CENTROID, settings.mol_ctr );
	if( settings.target_alt < 180 ) select( XB::IS_MORE_ALTITUDE, settings.mol_alt ); //?
	if( fabs( settings.target_azi ) < 180 ) select( XB::IS_MORE_AZIMUTH, settings.mol_azi ); //?
	if( settings.target_nrg  ) select( XB::IS_MORE_NRG, settings.mol_nrg );
	
}

//------------------------------------------------------------------------------------
//save the histogram (either to file or to stdout)
void xb_make_spc::save_histogram(){
	FILE *out;
	if( settings.out_flag ) out = fopen( settings.out_fname, "w" );
	else out = stdout;
	if( settings.verbose && settings.out_flag)
		printf( "Saving histogram in %s...\n", settings.out_fname );
	
	for( int i=0; i < settings.in_f_count; ++i ){
		if( !histo[i] ) continue;
		fwrite( &histo[i]->n, 1, sizeof(size_t), out );
		fwrite( &histo[i]->range, 1, (histo[i]->n+1)*sizeof(double), out );
		fwrite( &histo[i]->bin, 1, histo[i]->n*sizeof(double), out );
	}
	
	if( settings.out_flag ) fclose( out );
}

//------------------------------------------------------------------------------------
//save the data (either to file or stdout)
void xb_make_spc::save_data(){
	FILE *out;
	char command[310];
	if( settings.verbose && settings.out_flag )
		printf( "Saving data in %s...\n", settings.out_fname );
	
	if( settings.out_flag ){
		strcpy( command, "bzip2 -z > " );
		strcat( command, settings.out_fname );
		out = popen( command, "w" );
	} else out = stdout;
	
	for( int i=0; i < settings.in_f_count; ++i ){
		XB::write( out, event_klZ[i], ( i )? 0 : 1 );
	}

	if( settings.out_flag ) fclose( out );

}

//------------------------------------------------------------------------------------
//put the histogram on the drone output
void xb_make_spc::put_histogram(){
	FILE *out = settings.drone.out;
	if( settings.verbose ) printf( "Putting histogram into DRONE out.\n" );
	
	for( int i=0; i < settings.in_f_count; ++i ){
		if( !histo[i] ) continue;
		for( int b=0; b < histo[i]->n; ++i ){
			fprintf( out, "%f %f\n",
			         (histo[i]->range[b] + histo[i]->range[b+1])/2,
			         histo[i]->bin[b] );
		}
		fprintf( out, "\n" );
	}
}

//--------------------------------------------------------------------------------------
void xb_make_spc::put_data(){
	FILE *out = settings.drone.out;
	if( settings.verbose ) printf( "Putting histogram into DRONE out.\n" );

	for( int i=0; i < settings.in_f_count; ++i ){
		XB::write( out, event_klZ[i], ( i )? 0 : 1 );
	}
}
