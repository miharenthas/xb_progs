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
/*
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
	settings.mol_mul = MORE;
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
				sscanf( optarg, "%1s %s %1s %s", &settings.drone.in_pof,
				        settings.drone.insream, &settings.drone.out_pof,
				        settings.drone.outstream );
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
	
	xb_make_spc da_prog();
	
	//command line and program object fingering
	int breaker = DO_EXECUTE;
	while( breaker != DO_EXIT && ( settings.interactive || settings.drone_flag ) ){
		da_prog.reset( settings );
		if( settings.draw_flag ) da_prog.draw_histogram();
	
		if( settings.interactive ) XB::cml_loop_prompt( stdin, settings );
		else if( settings.drone_flag ) XB::cml_loop( settings.drone.in, settings );
	}
	
	//final ops
	if( settings.verbose ) printf( "Exiting...\n" );
	if( settings.drone_flag ){
		( settings.drone.in_pof == 'p' )? pclose( settings.drone.in ) : fclose( settings.drone.in );
		( settings.drone.out_pof == 'p' )? pclose( settings.drone.out ) : fclose( settings.drone.out );
	}
*/
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
	_do_hists = 1; _do_files = 1; _do_data = 1;
	
	load_files(); //and issues the file loading.
}
/*
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
	if( !_do_files ) return; //do nothing if there's nothing to do.
	
	if( settings.verbose && !settings.in_flag ) printf( "Reading from STDIN...\n" );
	
	//get the clusters from somewhere
	for( int i=0; i < settings.in_f_count && settings.in_flag; ++i ){
		if( settings.verbose && settings.in_flag )
			printf( "Reading from: %s...\n", settings.in_fname[i] );
		XB::load( settings.in_fname[i], event_klZ[i] );
	}
	
	if( !settings.in_flag ) XB::load( stdin, event_klZ[0] );
	_do_files = 0; //work done.
}

//------------------------------------------------------------------------------------
//file unloader
void xb_make_spc::clear_data(){
	if( settings.verbose ) puts( "Clearing data." );
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
	_do_hists = 0;
	_do_file = 0;
	_do_data = 0;

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
		++_do_files; ++_do_hists;
		settings.in_f_count = sts.in_f_count;
	}
	
	//if the number of bins has changed, redo the histogram
	if( settings.num_bins != sts.num_bins ){
		++_do_hists;
		settings.num_bins = sts.num_bins;
	}
	
	if( !memcmp( &settings.target_mul, &sts.target_mul,
	             6*( sizeof(moreorless) + sizeof(unsigned int) ) ){
		++_do_data;
		settings.target_mul = sts.target_mul;
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
		settings.mol_nrg = sts.mol_nrg;
	}
	
	settings.gp_opt.term = sts.gp_opt.term;
	settings.gp_opt.is_log = sts.gp_opt.is_log;

	strcpy( settings.gp_opt.x_label, sts.gp_opt.x_label );
	strcpy( settings.gp_opt.title, sts.gp_opt.title );
	
	if( settings.histo_mode != sts.histo_mode ){
		++_do_hists;
		settings.histo_mode = sts.histo_mode;
	}
	
	if( settings.drone_flag ){
		strcpy( settings.drone.instream, sts.drone.instream );
		strcpy( settings.drone.outstream, sts.drone.outstream );
		settings.in_pof = sts.in_pof;
		settings.out_pof = sts.out_pof;
		settings.in = sts.in;
		settings.out = sts.out;
	}
	
	//copy the strings
	for( int i=0; i < settings.in_f_count; ++i ){
		if( !strcmp( settings.in_fname[i], sts.in_fname[i] ){
			++_do_files;
			strcpy( settings.in_fname[i], sts.in_fname[i] );
		}
	}
	
	strcpy( settings.out_fname, sts.out_fname );
	
	settings.range[0] = sts.range[0];
	settings.range[1] = sts.range[1];
	
	if( _do_files ){ unload_files(); load_files(); }
	hack_data();
	populate_histogram();
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
	if( !_do_hists ) return; //do nothing if there's nothing to do.

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
	
	_do_hists = 0; //work done.
	if( settings.verbose ) printf( "Done.\n" );
}

//------------------------------------------------------------------------------------
//A function to apply the cut to the data (in sequence, not clever right now)
void xb_make_spc::hack_data(){
	if( !_do_data ) return; //do nothing if nothing to do
	
	if( settings.target_mul ) select( XB::IS_NOT_MULTIPLICITY, settings.mol_mul );
	if( settings.tarteg_cry ) select( XB::IS_MORE_CRYSTALS, settings.mol_cry );
	if( settings.target_ctr ) select( XB::IS_CENTROID, settings.mol_ctr );
	if( settings.target_alt < 180 ) select( XB::IS_MORE_ALTITUDE, settings.mol_alt ); //?
	if( fabs( settings.target_azi ) < 180 ) select( XB::IS_MORE_AZIMUTH, settings.mol_azi ); //?
	if( settings.target_nrg  ) select( XB::IS_MORE_NRG, settings.mol_nrg );
	
	_do_data = 0; //job done
}

//------------------------------------------------------------------------------------
//reload the data
void xb_make_spc::reload_data(){
	clear_data();
	_do_files = 1;
	load_files();
}

//------------------------------------------------------------------------------------
//save the histogram
void xb_make_spc::save_histogram(){
	FILE *out;
	if( settings.out_flag ) out = fopen( settings.out_fname, "w" );
	else out = stdout;
	
	for( int i=0; i < settings.in_f_count; ++i ){
		if( !histo[i] ) continue;
		fwrite( out, histo[i]->n, sizeof(size_t) );
		fwrite( out, histo[i]->range, (histo[i]->n+1)*sizeof(double) );
		fwrite( out, histo[i]->bin, histo[i]->n*sizeof(double) );
	}
	
	if( settings.out_flag ) fclose( out );
}

//------------------------------------------------------------------------------------
//save the data
void xb_make_spc::save_data(){
	FILE *out;
	char command[310];
	
	if( settings.out_flag ){
		strcpy( command, "bzip2 -z > " );
		strcat( command, settings.out_fname );
		out = popen( command, "w" );
	}	else out = stdout;
	
	for( int i=0; i < settings.in_f_count; ++i ){
		XB::write( out, event_klZ[i], ( i )? 0 : 1 );
	}

}

//------------------------------------------------------------------------------------
//put the histogram on the drone output
void xb_make_spc::put_histogram(){
	FILE *out = settings.drone.out;
	
	for( int i=0; i < settings.in_f_count; ++i ){
		if( !histo[i] ) continue;
		for( int b=0; b < histo[i]->n; ++i )
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
	
	for( int i=0; i < settings.in_f_count; ++i ){
		XB::write( out, event_klZ[i], ( i )? 0 : 1 );
	}
}
*/	
