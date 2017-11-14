//this program doppler corrects the XB::data

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <algorithm>
#include <vector>

#include "xb_doppler_corr.h"
#include "xb_cluster.h"
#include "xb_data.h"
#include "xb_io.h"
#include "xb_error.h"

#include "xb_doppc.h"

//------------------------------------------------------------------------------------
//the (piped) interface to read a root file with xb_data_translator
void translate_track_info( std::vector<XB::track_info> &xb_track_book,
                           unsigned int track_f_count, char track_f_name[][256],
                           int flagger );

//------------------------------------------------------------------------------------
//the main
int main( int argc, char **argv ){
	//input variables
	char in_f_name[256]; //input DATA file
	char track_f_name[64][256]; //input TRACK file
	char out_f_name[256]; //output file
	unsigned int track_f_count = 0; //# of track files
	unsigned int default_beam_out = 81; //the default beam in

	//flags
	int flagger = 0;
	correct_mode mode = RELAXED; //the default is: correct everything
	                             //at the best of your capabilities.
	
	//------------------------------------
	//input parsing
	//first look at the first unminused entry, which is supposed to be
	//the file containing the tracker information.
	for( int i=1; i < argc && i < 64; ++i ){
		if( argv[i][0] != '-' && strlen( argv[i] ) < 256 ){
			strcpy( track_f_name[i-1], argv[i] );
			++track_f_count;
		} else if( argv[i][0] == '-' ) break; //exit the loop at the first option
		                                      //and silently discard file names that
		                                      //are too long.
	}
	
	//set the flag for stdin reading
	if( track_f_count != 0 ) track_from_file_flag = true;
	
	struct options opts[] = {
		{ "data", required_argument, NULL, 'd' },
		{ "output", required_argument, NULL, 'o' },
		{ "read-rootfile", no_argument, &flagger, flagger | USE_TRANSLATOR },
		{ "fastidious", no_argument, NULL, 'f' },
		{ "very-fastidious", no_argument, NULL, 'F' },
		{ "simulation", no_argument, NULL, 's' },
		{ "cluster", no_argument,  &flagger, flagger | CLUSTER_FLAG },
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "beam-out", required_argument, NULL, 'b' },
		{ "no-track", no_argument, &flagger, flagger | NO_TRACK }
	};
	
	char iota = 0; int idx;
	while( (iota = getopt_long( argc, argv, "d:o:RfFskvb:", opts, &idx )) != -1 ){
		switch( iota ){
			case 'd' :
				if( strlen( optarg ) < 256 ){
					strcpy( in_f_name, optarg );
					flagger |= IN_FROM_FILE;
				} else {
					printf( "Input file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'o' :
				if( strlen( optarg ) < 256 ){
					strcpy( out_f_name, optarg );
					flagger |= OUT_TO_FILE;
				} else {
					printf( "Output file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'R' :
				flagger |= USE_TRANSLATOR;
				break;
			case 'f' :
				if( mode != VERY_FASTIDIOUS ) mode = FASTIDIOUS;
				break;
			case 'F' :
				mode = VERY_FASTIDIOUS;
				break;
			case 's' :
				mode = SIMULATION;
				break;
			case 'k' :
				flagger |= CLUSTER_FLAG;
				break;
			case 'v' :
				verbose = true;
				break;
			case 'b' :
				default_beam_out = atoi( optarg );
				if( default_beam_out < 1 || default_beam_out > 162 ){
					printf( "Valid indices: 1..162.\n" );
					exit( 1 );
				}
				break;
			default :
				printf( "\"%s\" is not a valid option.\n", optopt );
				printf( "Valid options are: -i <file_name>\n-o <file_name>\n" );
				printf( "-R\n-f\n-F\n-s\n-v\n-b [1..162]\n" );
				exit( 0 );
		}
	}
	
	//consistency check
	if( !( flagger & (IN_FROM_FILE | TRACK_FROM_FILE | NO_TRACK) ) ){
		printf( "Sorry, at the moment I can't read everything from STDIN...\n" );
		exit( 1 );
	}
	
	if( flagger & VERBOSE ) printf( "*** Welcome in the doppler corrector ***\n" );
	
	//------------------------------------
	//first of all, read the track data
	std::vector<XB::track_info> xb_track_book, tb_buf;
	if( flagger & NO_TRACK );
		//we don't have a track, move on.
	else if( flagger & (USE_TRANSLATOR | TRACK_FROM_FILE) ){
		//if the file(s) haven't been translated yet
	        //use the translator
		if( flagger & VERBOSE ) printf( "Reading through xb_data_translator...\n" );
		translate_track_info( xb_track_book, track_f_count, track_f_name, verbose );
	} else if( flagger & TRACK_FROM_FILE && !( flagger & USE_TRANSLATOR ) ) {
		//or loop on them with XB::load
		for( int i=0; i < track_f_count; ++i ){
			if( flagger & VERBOSE ) printf( "Reading track from %s...\n", track_f_name[i] );
			XB::load( track_f_name[i], tb_buf ); //load the track data
			
			//cat them at the end of the array
			xb_track_book.insert( xb_track_book.end(), tb_buf.begin(), tb_buf.end() );
			
			//cleanup
			tb_buf.clear();
		}
	} else if( !( flagger & TRACK_FROM_FILE ) ) { //or load from STDIN
		if( flagger & VERBOSE ) printf( "Reading track from STDIN...\n" );
		XB::load( stdin, xb_track_book );
	}
	
	//then, load the actual data to correct
	std::vector<XB::data> xb_book;
	std::vector<XB::clusterZ> klz;
	if( flagger & IN_FROM_FILE ){
		if( flagger & VERBOSE ) printf( "Reading data from %s...\n", in_f_name );
		if( flagger & CLUSTER_FLAG ) XB::load( in_f_name, klz );
		else XB::load( in_f_name, xb_book );
	} else {
		if( flagger & VERBOSE ) printf( "Reading data from STDIN...\n" );
		if( flagger & CLUSTER_FLAG ) XB::load( stdin, klz );
		else XB::load( stdin, xb_book );
	}
	
	//------------------------------------
	//do the correction
	if( flagger & VERBOSE ) printf( "Processing...\n" );
	if( flagger & CLUSTER_FLAG ) apply_doppler_correction( klz, xb_track_book,
	                                                       default_beam_out,
	                                                       mode, flagger );
	else apply_doppler_correction( xb_book, xb_track_book,
	                               default_beam_out,
	                               mode, flagger );

	//------------------------------------	
	//now, save
	if( flagger & OUT_TO_FILE ){
		if( flagger & VERBOSE ) printf( "Writing to %s...\n", out_f_name );
		if( flagger & CLUSTER_FLAG ) XB::write( out_f_name, klz );
		else XB::write( out_f_name, xb_book );
	} else {
		if( flagger & CLUSTER_FLAG ) XB::write( stdout, klz );
		else XB::write( stdout, xb_book );
	}
	
	//happy thoughts
	if( flagger & VERBOSE ) printf( "*** Done, goodbye. ***\n" );
	return 0;
}


//------------------------------------------------------------------------------------
//implementation of the (piped) interface to xb_data_translator
void translate_track_info( std::vector<XB::track_info> &xb_track_book,
                           unsigned int track_f_count, char track_f_name[][256],
                           bool verbose ){

	//prepare the command
	char command[64*256 + 512]; //potentially, this is huge

	//build the command:
	//first check if it's in the system path, if so use it
	//if not, try it in the local folder
	//else, kill yourself.
	if( !system( "which xb_data_translator 2>1 1>/dev/null" ) )
		strcpy( command, "xb_data_translator " );
	else if( !system( "which ./xb_data_translator 2>1 1>/dev/null" ) )
		strcpy( command, "./xb_data_translator " );
	else{
		fprintf( stderr, "Error: couldn't find the data translator.\n" );
		exit( 1 );
	}

	//cat the filenames
	if( flagger & VERBOSE ) printf( "Translator will read from file(s):\n" );
	for( int f=0; f < track_f_count; ++f ){
		if( flagger & VERBOSE ) printf( "\t%s\n", track_f_name[f] );
		strcat( command, track_f_name[f] );
		strcat( command, " " );
	}

	//add the option to get tracking data
	strcat( command, "-t" );
	
	if( flagger & VERBOSE ) printf( "Calling:\n\n%s\n\n", command );
	
	//open the read pipe
	FILE *p_xb_data_translator = popen( command, "r" );

	//receive the data
	XB::load( p_xb_data_translator, xb_track_book );

	//close the pipe
	pclose( p_xb_data_translator );
}
