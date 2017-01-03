//a program to apply a cut to a set of data

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "xb_cut.h"
#include "xb_data.h"
#include "xb_io.h"
#include "xb_draw_cut.h"
#include "xb_apply_cut.h"
#include "xb_parse_cnf_file.h"

//------------------------------------------------------------------------------------
int main( int argc, char **argv ){
	char in_fname[64][256], out_fname[256];
	char *cut_config = NULL; unsigned int config_len;

	//parse the unargumente command line in search of filenames
	int in_fcount = 0; //number of input files retrieved from the command line
	for( int i=1; argv[i][0] != '-'; ++i ){
		strcpy( in_fname[i-1], argv[i] );
		++in_fcount;
	}
	
	char iota = 0; int long_iota = 0; //opt character
	bool verbose = false; //verbosity flag
	bool draw = false; //dawing flag
	bool out_to_file = false; //write to file
	FILE *config_pipe;
	
	//create the long options
	struct option longopts[] = {
		{ "config-str", required_argument, NULL, 'C' },
		{ "config-file", required_argument, NULL, 'F' },
		{ 0, 0, 0, 999 }
	};
		
	while( (iota = getopt_long( argc, argv, "i:o:C:F:vd", longopts, &long_iota )) != -1 ){
		switch( iota ){
			case 'i' : //sets ONE input file name
			           //disregards the unopted filenames
				in_fcount = 1;
				strcpy( in_fname[0], optarg );
				break;
			case 'o' : //sets the output file name
				strcpy( out_fname, optarg );
				out_to_file = true;
				break;
			case 'C' : //takes in the cut configuration
				config_len = strlen( optarg );
				cut_config = (char*)malloc( config_len );
				strcpy( cut_config, optarg );
				break;
			case 'F' : //takes in the cut config from file
			           //NOTE: 1024 chars should be enough
			           //      for a config file. If not
			           //      then this can be enlarged
				cut_config = (char*)malloc( 65408 ); //511 lines of
				                                     //128 characters each.
				                                     //huge.
				strcpy( cut_config, "cat " );
				strcat( cut_config, optarg );
				strcat( cut_config, " | tr \"\\\n\\\t\" \" \"" );
				config_pipe = popen( cut_config, "r" );
				fgets( cut_config, 65408, config_pipe );
				pclose( config_pipe );
				break; 
			case 'v' : //sets the verbose flag
				verbose = true;
				break;
			case 'd' :
				draw = true;
				break;
			default : //TODO: output a more informative message
				printf( "Unknown option: %c.\n", iota );
				exit( 1 );
		}
	}
	
	if( verbose ){
		printf( "*** Welcome in the cutting program! ***\n" );
	}
	
	//check the config
	if( cut_config == NULL ){
		fprintf( stderr, "No configuration provided. No default available. Abort.\n" );
		exit( 1 );
	}
	
	//parse the configuration
	//NOTE: so far, just one cut is supported by this progra
	//      the parser, though, can understand more cuts.
	//TODO: add this feature
	XB::cut_cnf *the_cut = XB::config_alloc( 1 );
	XB::parse_config( &cut_config, the_cut );
	
	//load the input
	std::vector< XB::track_info* > data, buf;
	if( in_fcount ){ //there are input files; load them.
		for( int i=0; i < in_fcount; ++i ){
			if( verbose )
				printf( "Loading from file: \"%s\"...\n", in_fname[i] ); 
			XB::load( in_fname[i], buf );
			//NOTE: the ordering of the data is irrelevant
			//      stacking them like this requires
			//      the least typing.
			data.insert( data.begin(), buf.begin(), buf.end() );
			//NOTE: these are vectors of pointers, deallocating
			//      the pointers would mean destroying the data
			buf.clear(); //this does not deallocate the pointers.
		}
	} else { //read from stdin
		if( verbose ) printf( "Loading from STDIN...\n" );
		XB::load( stdin, data );
	}
	
	//wrap the data
	XB::cut_on_zaz wrapped_data( data );
	
	//apply the cut
	//NOTE: the toolkit supports for signed cuts, this program not yet
	//TODO: add support for multple signed cuts.
	if( verbose ) puts( "Performing the cut..." );
	std::vector<bool> flags = XB::do_cut< XB::track_info >( &wrapped_data,
	                                                        the_cut->cut_2D );
	
	//if visualzation is selected, draw the cut.
	//NOTE: the && in_fcount bit excludes this conditional
	//      if we are reading from a pipe, since it would interfere...
	if( draw && in_fcount ){
		XB::draw_cut< XB::track_info >( the_cut->cut_2D,
		                                wrapped_data, flags,
		                                "Z", "A on Z" );
		if( verbose ) printf( "Press enter to continue... " );
		iota = getchar();
	}
	
	if( verbose ) puts( "Applying the cut (removing unflagged data)..." );
	int nb_removed = XB::apply_flagged_cut( data, flags );
	if( verbose ){
		printf( "\tOriginal size: %d\n", data.size() + nb_removed );
		printf( "\tRemoved elements: %d\n", nb_removed );
		printf( "\tFinal size: %d\n", data.size() );
	}
	
	//finally, output the results
	if( verbose ) printf( "Writing to file \"%s\"...\n", out_fname );
	if( out_to_file ) XB::write( out_fname, data );
	else XB::write( stdout, data );
	
	puts( "***Done, goodbye.***" );
	
	return 0;
}
