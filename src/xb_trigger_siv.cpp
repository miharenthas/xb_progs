//a simple program that runs the selection on the tpat thingie
#include <stdio.h>
#include <vector>
#include <getopt.h>

#include "xb_data.h"
#include "xb_io.h"
#include "xb_tpat.h"

#define VERBOSE 0x10
#define IN_FLAG 0x01
#define OUT_FLAG 0x02
#define TRACK 0x20

int main( int argc, char **argv ){
	char in_fname[256];
	char out_fname[256];
	char tpat_str[256]; strcpy( tpat_str, "all" );
	int flagger = 0;

	struct option opts[] = {
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o' },
		{ "tpat", required_argument, NULL, 'T' },
		{ "track", no_argument, &flagger, flagger | TRACK },
		{ NULL, 0, NULL, 0 }
	};
	
	char iota = 0; int idx;
	while( (iota = getopt_long( argc, argv, "i:o:T:vt", opts, &idx )) != -1 ){
		switch( iota ){
			case 'i' :
				strncpy( in_fname, optarg, 256 );
				flagger |= IN_FLAG;
				break;
			case 'o' :
				strncpy( out_fname, optarg, 256 );
				flagger |= OUT_FLAG;
				break;
			case 'T' :
				strncpy( tpat_str, optarg, 256 );
				break;
			case 'v' :
				flagger |= VERBOSE;
				break;
			case 't' :
				flagger |= TRACK;
				break;
			default :
				fprintf( stderr, "usage: [-T tpat|-i FILE|-o FILE|-v|-t]\n" );
				exit( 1 );
		}
	}
	
	if( flagger & VERBOSE ) printf( "*** Welcome in the TPAT selector program! ***\n" );
	
	std::vector<XB::data> data;
	std::vector<XB::track_info> track;
	
	if( flagger & VERBOSE ) printf( "Reading from %s...\n",
	                                ( flagger & IN_FLAG )? in_fname : "STDIN" );
	
	if( flagger & IN_FLAG ){
		if( flagger & TRACK ) XB::load( in_fname, track );
		else XB::load( in_fname, data );
	} else {
		if( flagger & TRACK ) XB::load( stdin, track );
		else XB::load( stdin, data );
	}
	
	if( flagger & VERBOSE ) printf( "Pruning data...\n" );
	
	int mask = XB::str2tpat( tpat_str );
	int nb_removed, sz_bf = ( flagger & TRACK )? track.size() : data.size();
	if( flagger & TRACK ) nb_removed = select_on_tpat( mask, track );
	else nb_removed = select_on_tpat( mask, data );
	
	if( flagger & VERBOSE ) printf( "Events before: %d\nEvents now: %d\nRemoved: %d\n",
	                                sz_bf, ( flagger & TRACK )? track.size() : data.size(),
	                                nb_removed );
	
	if( flagger & OUT_FLAG ){
		if( flagger & TRACK ) XB::write( out_fname, track );
		else XB::write( out_fname, data );
	} else {
		if( flagger & TRACK ) XB::write( stdout, track );
		else XB::write( stdout, data );
	}
	
	if( flagger & VERBOSE ) puts( "Done. Goodbye." );
	
	return 0;
}	
