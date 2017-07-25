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

//------------------------------------------------------------------------------------
//drone info structure
typedef struct _drone_settings{
	char instream[256];
	char outstream[256];
	FILE *in;
	FILE *out;
} d_opts;

//------------------------------------------------------------------------------------
//the program's setting container
typedef struct _program_settings{
	//input variables
	char in_fname[64][256]; //input file names
	char out_fname[256]; //output file name
	int drone_flag; //if true, the program works in drone mode.
	int in_flag; //if true, read from file (2 for pipe).
	int out_flag; //if true, save to file (2 for pipe).
	int draw_flag; //if true, draw (also to file).
	int verbose; //if true, print verbose output.
	int interactive; //if true, display prompt.
	unsigned int in_f_count; //number of input files.
	unsigned int num_bins; //number of bins with which to create the histogram
	unsigned int target_mul; //target multiplicity -- 0 indicates "sum all"
	unsigned int target_ctr; //target centroid -- 0 indicates "all"
	unsigned int target_alt; //target altitude -- 0 indicates "all"
	unsigned int target_azi; //target azimuth -- 0 indicates "all"
	unsigned int target_nrg; //target energy
	float range[2]; //the range of the histogram
	XB::gp_options gp_opt; //gnuplot options
	XB::histogram_mode histo_mode; //the histogram population mode.
	d_opts drone; //the drone stuff.
} p_opts;
		

//------------------------------------------------------------------------------------
//the program's class
class xb_make_spc{
	public:
		typedef enum more_or_less{
			MORE,
			LESS
		} moreorless;
	
		//ctors, dtor
		xb_make_spc( p_opts &settings ); //custom constructor
		~xb_make_spc(); //death maker
		
		//methods
		void populate_histogram(); //populate the histogram
		void draw_histogram(); //draw the histogram (with gnuplot)
		void save_histogram(); //save the histogram to file.
		void select( XB::selsel selector_type, moreorless m ); //select inside the cluster structure
		
		void reset( p_opts &settings ); //update the settings
	private:
		xb_make_spc(); //default constructor. This class cannot be instantiated without valid settings.
		
		void load_files(); //file loader
		void unload_files(); //file unloader
		void target_multiplicity(); //set the target_multiplicity
		
		std::vector<XB::clusterZ> event_klZ[64]; //the clusters
		gnuplot_ctrl *gp_h; //the handle to the gnuplot session
		gsl_histogram *histo[64]; //the histograms
		p_opts settings; //the settings
};

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
	settings.target_alt = 0;
	settings.target_azi = 0;
	settings.target_nrg = 0;
	settings.range[0] = 0;
	settings.range[1] = 8000;
	settings.gp_opt.term = XB::QT;
	settings.gp_opt.is_log = true;
	strcpy( settings.gp_opt.x_label, "KeV" );
	strcpy( settings.gp_opt.title, "XB_spectrum" );
	
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
				sscanf( optarg, "%s::%s", settings.drone.insrean, settings.drone.outstream );
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
	
	//major function calls
	xb_make_spc the_prog( settings );	
	the_prog.populate_histogram();
	the_prog.draw_histogram();

	//ask what to do
	while( settings.interactive ){
		//get the input
		printf( "xb_make_spc> " );
		fgets( command, 512, stdin ); //hack allert!
		
		//parse it
		//it's largely the same as the launch options
		//except for:
		//E -- exit the program
		//G -- execute again with the new options
		switch( command[0] ){
			//NOTE: this will be reimplemented later.
			/*case 'i':
				sscanf( &command[1], "%s", comm );
				if( strlen( comm ) < 256 ){
					strcpy( in_fname, comm );
					in_flag = true;
				}else{
					printf( "File name too long!\n" );
					exit( 1 );
				}
				break;*/
			case 'o':
				sscanf( &command[1], "%s", comm );
				if( strlen( comm ) < 256 ){
					strcpy( settings.out_fname, comm );
					settings.out_flag = true;
				}else{
					printf( "File name too long.\n" );
				}
				break;
			case 'b':
				sscanf( &command[1], "%s", comm );
				settings.num_bins = atoi( comm );
				break;
			case 'R':
				sscanf( &command[1], "%s", comm );
				sscanf( comm, "%f:%f", &settings.range[0], &settings.range[1] );
				break;
			case 's':
				sscanf( &command[1], "%s", comm );
				if( !strcmp( comm, "qt" ) ) settings.gp_opt.term = XB::QT;
				else if( !strcmp( comm, "png" ) ) settings.gp_opt.term = XB::PNG;
				else{
					printf( "Invalid gnuplot terminal.\n" );
				}
				break;
			case 'l':
				sscanf( &command[1], "%s", comm );
				if( !strcmp( comm, "yes" ) ) settings.gp_opt.is_log = true;
				else if( !strcmp( comm, "no" ) ) settings.gp_opt.is_log = false;
				else{
					printf( "Invalid plot scale.\n" );
				}
				break;
			case 't':
				sscanf( &command[1], "%s", comm );
				if( strlen( comm ) < 256 ) strcpy( settings.gp_opt.title, comm );
				else{
					printf( "Title is too long.\n" );
				}
				break;
			case 'm':
				sscanf( &command[1], "%s", comm );
				settings.target_mul = atoi( comm );
				break;
			case 'H':
				sscanf( &command[1], "%s", comm );
				if( !strcmp( comm, "compare" ) ) settings.histo_mode = XB::COMPARE;
				else if( !strcmp( comm, "subtract" ) ) settings.histo_mode = XB::SUBTRACT;
				else settings.histo_mode = XB::JOIN;
				break;
			case 'E':
				goto __THE_END__;
				break;
			case 'G':
				//update the program settings
				the_prog.reset( settings );
				the_prog.draw_histogram();
				break;
			default :
				printf( "Sorry, I don't know %s", command );
				break;
		}
	}
	
	__THE_END__:
	
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
void xb_make_spc::target_multiplicity(){
	if( settings.target_mul != 0 ){
		//create a comparison functional 
		isntm isnt_mul( settings.target_mul );
		std::vector<XB::cluster>::iterator last;
		
		//find all the non interesting multiplicities
		for( int i=0; i < settings.in_f_count; ++i ){
			last = std::remove_if( event_klZ[i].begin(), event_klZ[i].end(), isnt_mul );
			
			event_klZ[i].erase( last, even_klZ[i].end() );
		}
	}
}

//------------------------------------------------------------------------------------
//more advanced data cutter
void xb_make_spc::select( XB::selsel selector_type, xb_make_spc::moreorless m ){
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
