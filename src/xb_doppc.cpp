//this program doppler corrects the XB::data

#include <stdio.h>
#include <unistd.h>
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
                           bool verbose );

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
	bool verbose = false; //if true, verbose output
	bool in_flag = false; //if true, read from a file and not stdin
	bool track_flag = false; //if true, read the track info from file and not stdin
	bool cluster_flag = false; //if true, correct clusters instead of just events.
	bool out_flag = false; //if true, write to a file and not to stdout
	bool reader_flag = false; //if true, use xb_data_translator to read the TRACK file
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
	if( track_f_count != 0 ) track_flag = true;
		
	char iota = 0;
	while( (iota = getopt( argc, argv, "d:o:RfFskvb:" )) != -1 ){
		switch( iota ){
			case 'd' :
				if( strlen( optarg ) < 256 ){
					strcpy( in_f_name, optarg );
					in_flag = true;
				} else {
					printf( "Input file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'o' :
				if( strlen( optarg ) < 256 ){
					strcpy( out_f_name, optarg );
					out_flag = true;
				} else {
					printf( "Output file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'R' :
				reader_flag = true;
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
				cluster_flag = true;
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
	if( !track_flag && !in_flag ){
		printf( "Sorry, at the moment I can't read everything from STDIN...\n" );
		exit( 1 );
	}
	
	if( verbose ) printf( "*** Welcome in the doppler corrector ***\n" );
	
	//------------------------------------
	//first of all, read the track data
	std::vector<XB::track_info> xb_track_book, tb_buf;
	
	if( reader_flag && track_flag ){ //if the file(s) haven't been translated yet
	                                 //use the translator
		if( verbose ) printf( "Reading through xb_data_translator...\n" );
		translate_track_info( xb_track_book, track_f_count, track_f_name, verbose );
	} else if( !reader_flag && track_flag ) { //or loop on them with XB::load
		for( int i=0; i < track_f_count; ++i ){
			if( verbose ) printf( "Reading track from %s...\n", track_f_name[i] );
			XB::load( track_f_name[i], tb_buf ); //load the track data
			
			//cat them at the end of the array
			xb_track_book.insert( xb_track_book.end(), tb_buf.begin(), tb_buf.end() );
			
			//cleanup
			tb_buf.clear();
		}
	} else if( !track_flag ) { //or load from STDIN
		if( verbose ) printf( "Reading track from STDIN...\n" );
		XB::load( stdin, xb_track_book );
	}
	
	//then, load the actual data to correct
	std::vector<XB::data> xb_book;
	std::vector<XB::clusterZ> klz;
	if( in_flag ){
		if( verbose ) printf( "Reading data from %s...\n", in_f_name );
		if( cluster_flag ) XB::load( in_f_name, klz );
		else XB::load( in_f_name, xb_book );
	} else {
		if( verbose ) printf( "Reading data from STDIN...\n" );
		if( cluster_flag ) XB::load( stdin, klz );
		else XB::load( stdin, xb_book );
	}
	
	//------------------------------------
	//do the correction
	if( verbose ) printf( "Processing...\n" );
	if( cluster_flag ) apply_doppler_correction( klz, xb_track_book,
	                                             default_beam_out,
	                                             mode, verbose );
	else apply_doppler_correction( xb_book, xb_track_book,
	                               default_beam_out,
	                               mode, verbose );

	//------------------------------------	
	//now, save
	if( out_flag ){
		if( verbose ) printf( "Writing to %s...\n", out_f_name );
		if( cluster_flag ) XB::write( out_f_name, klz );
		else XB::write( out_f_name, xb_book );
	} else {
		if( cluster_flag ) XB::write( stdout, klz );
		else XB::write( stdout, xb_book );
	}
	
	//happy thoughts
	if( verbose ) printf( "*** Done, goodbye. ***\n" );
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
	strcpy( command, "./xb_data_translator " );

	//cat the filenames
	if( verbose ) printf( "Translator will read from file(s):\n" );
	for( int f=0; f < track_f_count; ++f ){
		if( verbose ) printf( "\t%s\n", track_f_name[f] );
		strcat( command, track_f_name[f] );
		strcat( command, " " );
	}

	//add the option to get tracking data
	strcat( command, "-t" );
	
	if( verbose ) printf( "Calling:\n\n%s\n\n", command );
	
	//open the read pipe
	FILE *p_xb_data_translator = popen( command, "r" );

	//receive the data
	XB::load( p_xb_data_translator, xb_track_book );

	//close the pipe
	pclose( p_xb_data_translator );
}
