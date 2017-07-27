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
	settings.target_ctr = 0;
	settings.target_alt = 180;
	settings.target_azi = 180;
	settings.target_nrg = 0;
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
		{ "input", argument_required, NULL, 'i' },
		{ "output", argument_required, NULL, 'o' },
		{ "nb-bins", argument_required, NULL, 'b' },
		{ "range", argument_requires, NULL, 'R' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "interactive", no_argument, NULL, 'I' },
		{ "gnuplot-term", argument_required, NULL, 's' },
		{ "y-scale", argument_required, NULL, 'l' },
		{ "title", argument_required, NULL, 't' },
		{ "histogram-mode", argument_required, NULL, 'H' },
		{ "select-multiplicity", argument_required, NULL, 'm' },
		{ "select-centroid", argument_required, NULL, 'c' },
		{ "drone", argument_required, NULL, 'D' },
		{ NULL, NULL, NULL, NULL }
	};

	//input parsing
	char iota = 0;
	while( (iota = getopt( argc, argv, "i:o:b:R:vIs:l:t:m:H:c:" )) != -1 ){
		switch( iota ){
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
			case 'D' :
				settings.drone_flag = true;
				sscanf( optarg, "%c:%s::%c:%s", &settings.drone.in_pof,
				        &settings.drone.insream, &settings.drone.out_pof,
				        &settings.drone.outstream );
				settings.drone.in = ( settings.drone.in_pof == 'p' )?
				                    popen( settings.drone.instream, "r" ) :
				                    fopen( settings.drone.instream, "r" );
				settings.drone.out = ( settings.drone.out_pof == 'p' )?
				                     popen( settings.drone.outstream, "w" ) :
				                     fopen( settings.drone.outstream, "w" );
			default :
				exit( 1 );
				//help note will come later
		}
	}
	
	//consistency checks
	if( settings.interactive && !settings.in_flag ){
		fprintf( stderr, "Cannot be interactive and read from a pipe...\n" );
		exit( 1 );
	}

	if( settings.verbose ) printf( "*** Welcome in the spectrum making program! ***\n" );
	
	//command line and program object fingering
	//...
	
	//final ops
	if( settings.verbose ) printf( "Exiting...\n" );
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
	settings.target_alt = sts.target_alt;
	settings.target_azi = sts.target_azi;
	settings.target_nrg = sts.target_nrg;
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
		settings.in = sts.in;
		settings.out. sts.out;
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
	if( settings.verbose ) puts( "Unloading files." );
	for( int i=0; i < settings.in_f_count && settings.in_flag; ++i )
		event_klZ[i].clear();
}
		

//------------------------------------------------------------------------------------
//_multiplicity pruner
void xb_make_spc::target_multiplicity( moreorless m ){
	if( settings.target_mul != 0 ){
		//create a comparison functional 
		isntm isnt_mul( settings.target_mul );
		std::vector<XB::cluster>::iterator last;
		
		//find all the non interesting multiplicities
		for( int i=0; i < settings.in_f_count; ++i ){
			last = std::remove_if( event_klZ[i].begin(), event_klZ[i].end(), isnt_mul );
			
			if( last == event_klZ[i].end() ) continue;
			if( m == MORE ) event_klZ[i].erase( last, even_klZ[i].end() );
			else if( m == LESS ) event_klZ[i].erase( event_klZ[i].begin(), last );
		}
	}
}

//------------------------------------------------------------------------------------
//more advanced data cutter
void xb_make_spc::select( XB::selsel selector_type, moreorless m ){
	//in order to make good use of polymorphism
	std::unary_function< XB::cluster > *in_kl_selector;
	
	switch( selector_type ){
		case XB::IS_NOT_MULTIPLICITY : //just do the old stuff
			target_multiplicity();
			return;
		case XB::IS_CENTROID :
			in_kl_selector = new XB::is_centroid( settings.target_ctr );
			break;
		case XB::IS_MORE_CRYSTALS :	
			in_kl_selector = new XB::is_more_crystal( settings.target_nb_cry );
			break;
		case XB::IS_MORE_ALTITUDE :
			in_kl_selector = new XB::is_more_altitude( settings.target_alt );
			break;
		case XB::IS_MORE_AZIMUTH :
			in_kl_selector = new XB::is_more_azimuth( setting.target_azi );
			break;
		case XB::IS_MORE_NRG :
			in_kl_selector = new XB::is_more_nrg( settings.target_nrg );
			break;
		default :
			throw XB::error( "Wrong selector type!", "xb_make_spc::select" );
			break;
	}
	
	std::vector<XB::cluster>::iterator k_last;
	std::vector<XB::cluster> *kl;
	
	//remove the unwanted data (yes, the policy has changed)
	for( int f=0; f < settings.in_f_count; ++f ){
		for( int k=0; k < event_klZ[f].size(); ++k ){
			kl = &event_klZ[f][k].clusters; //useful aliasing
			
			k_last = std::remove_if( kl->begin(), kl->end(), *in_kl_selector );
			
			if( k_last == kl->end() ) continue;
			if( m == MORE ) kl->erase( kl->begin(), k_last );
			else if( m == LESS ) kl->erase( k_last, kl->end() );
		}
	}
}

//------------------------------------------------------------------------------------
//reset the object
void xb_make_spc::reset( p_opts &sts ){
	//some internal flags
	bool redo_files = false;
	bool redo_hists = false;
	
	//copy
	settings.in_flag = sts.in_flag; 
	settings.out_flag = sts.out_flag;
	settings.verbose = sts.verbose;
	settings.interactive = sts.interactive;
	
	if( settings.in_f_count != sts.in_f_count ){
		redo_files = true;
		settings.in_f_count = sts.in_f_count;
	}
	
	if( settings.num_bins != sts.num_bins ){
		settings.num_bins = sts.num_bins;
		redo_hists = true;
	}
	
	if( settings.target_mul != sts.target_mul ){
		settings.target_mul = sts.target_mul;
		redo_hists = true;
	}
	
	settings.gp_opt.term = sts.gp_opt.term;
	settings.gp_opt.is_log = sts.gp_opt.is_log;
	
	if( settings.histo_mode != sts.histo_mode ){
		settings.histo_mode = sts.histo_mode;
		redo_hists = true;
	}
	
	//copy the strings
	for( int i=0; i < settings.in_f_count; ++i ){
		if( strcmp( settings.in_fname[i], sts.in_fname[i] ) ){
			strcpy( settings.in_fname[i], sts.in_fname[i] );
			redo_files = true;
		}
	}
	
	strcpy( settings.out_fname, sts.out_fname );
	strcpy( settings.gp_opt.title, sts.gp_opt.title );
	
	if( settings.range[0] != sts.range[0] ||
	    settings.range[1] != sts.range[1] ){
		settings.range[0] = sts.range[0];
		settings.range[1] = sts.range[1];
		redo_hists = true;
	}
	
	//consistency checks
	//can't compare or subtract less than two histograms
	if( settings.in_f_count <= 1 ){
		settings.histo_mode = XB::JOIN;
		redo_hists = true;
	}
	
	//trigger the mods
	if( redo_files ){
		this->load_files();
		redo_hists = true;
	}
	
	if( redo_hists ) this->populate_histogram();
}

//------------------------------------------------------------------------------------
//implementation of the drawing function
void xb_make_spc::draw_histogram(){
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
				 klZ != last[0]; ++klZ ){
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
