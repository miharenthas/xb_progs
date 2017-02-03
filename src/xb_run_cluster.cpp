//this program runs xb_cluster on a XB::data dataset (also, on one)
//it can visualize the events with gnuplot

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <omp.h>

#include <vector>

#include "xb_io.h"
#include "xb_error.h"
#include "xb_cluster.h"
#include "xb_draw_cluster_ball.h"


//--------------------------------------------------------------------------------
int main( int argc, char **argv ){
	//input variables
	char in_fname[256], out_fname[256];
	bool in_flag = false, out_flag = false, verbose = false,
	     draw = false, check_flag = false;
	int neigh_order = 1, wait_for = 10; //order of neighbourhood for the NN algorithm
	gnuplot_ctrl *gp_h; //the handle to the gnuplot session, in case we draw
	
	//get ONE filename from the command line.
	if( argv[1][0] != '-' && strlen( argv[1] ) < 256 ){
		strcpy( in_fname, argv[1] );
		in_flag = true;
	}

	//input parsing
	char iota = 0;
	while( (iota = getopt( argc, argv, "i:o:vdn::w:c") ) != -1 )
		switch( iota ){
			case 'i' : //set an input file
				if( strlen( optarg ) < 256 ){
					strcpy( in_fname, optarg );
					in_flag = true;
				}
				break;
			case 'o' : //set an output file
				if( strlen( optarg ) < 256 ){
					strcpy( out_fname, optarg );
					out_flag = true;
				}
				break;
			case 'v' : //be verbose
				verbose = true;
				break;
			case 'd' : //draw events
				draw = true;
				break;
			case 'n' : //use nearest-neighbour
				if( optarg != NULL ) neigh_order = atoi( optarg );
				break;
			case 'w' : //set the waiting period for display
				wait_for = atoi( optarg );
				break;
			case 'c' : //check the output file
				check_flag = true;
				break;
			default:
				printf( "-%c is not a valid option.\n", optopt );
				printf( "usage: xb_run_cluster [-i[FILE]|-o[FILE]|-v|-d|-n[NEIGHBOUR ORDER]|-w[SECONDS]|-c]\n" );
				printf( "if -i or -o aren't specified, stdin or stdout are used, respectively.\n" );
				exit( 1 );
		}
	//standard greetings
	if( verbose ) printf( "*** Welcome in the clustering program for s412! ***\n" );
	
	//if no i/o file is specified, use the standard streams.
	//load the input, whichever it is
	if( verbose && in_flag ) printf( "Reading from: %s...\n", in_fname );
	else if( verbose && !in_flag ) printf( "Reading from STDIN...\n" );

	std::vector<XB::data> xb_book;
	if( in_flag ) XB::load( in_fname, xb_book );
	else XB::load( stdin, xb_book );
	
	//loop run the chosen algorithm:
	std::vector<XB::clusterZ> event_klZ( xb_book.size() ); //cluster buffer
	
	#pragma omp parallel shared( event_klZ, xb_book )
	{
	if( verbose && !omp_get_thread_num() ){
		printf( "Running with %d threads...\n", omp_get_num_threads() );
		printf( "Processing event: 0000000000" );
	}
	#pragma omp for schedule( dynamic )
	for( int i=0; i < xb_book.size(); ++i ){
		if( verbose && !omp_get_thread_num() )
			printf( "\b\b\b\b\b\b\b\b\b\b" );
		
		event_klZ[i] = XB::make_clusters_NN( xb_book[i], neigh_order );
		
		if( verbose && !omp_get_thread_num() )
			printf( "%010d", i );
	}
	if( verbose && !omp_get_thread_num() ) printf( "\nDone.\n" );
	} //parallel pragma ends here
	
	//draw events,
	if( draw ) for( int i=0; i < event_klZ.size(); ++i ){
		if( verbose ){
			printf( "Drawing event %d:\n", i );
			printf( ".n: %u.\n\n", event_klZ[i].n );
		}
		
		gp_h = XB::draw_cluster_ball( event_klZ[i] );
				
		sleep( wait_for );
		gnuplot_close( gp_h );
	}
	
	//save/spit it out
	if( verbose ) printf( "Saving in: %s\n", out_fname );
	if( out_flag ) XB::write( out_fname, event_klZ );
	else XB::write( stdout, event_klZ );
	
	//check the output file
	std::vector<XB::clusterZ> event_klZ_check;
	if( check_flag && out_flag ){
		if( verbose ) printf( "Veryfing...\n" );
		try{
			XB::load( out_fname, event_klZ_check );
		}catch( XB::error e ){
			printf( "There has been an error: %s\n", e.what );
			exit( 1 );
		}
		
		if( verbose ) printf( "Entry #: " );
		for( int i=0; i < event_klZ.size(); ++i ){
			if( verbose && i ) printf( "\b\b\b\b\b\b\b\b\b\b" );
			if( verbose ) printf( "%010d", i );
			
			if( event_klZ[i].n != event_klZ_check.at(i).n ){
				printf( "Ooops: screwed up.\n" );
				break;
			}
		}
		if( verbose ) putchar( '\n' );
	}
	
	//happy thoughts
	if( verbose ) printf( "*** Done. Goodbye. ***\n" );
	return 0;
}
	
