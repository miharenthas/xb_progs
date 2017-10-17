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
#define DO_OR 0x40

int main( int argc, char **argv ){
	char in_fname[64][256];
	char out_fname[256];
	char tpat_str[256]; strcpy( tpat_str, "all" );
	int flagger = 0;
	int in_fcount = 0;

	//as unzual, interpret the first arguments as files
	for( int i=1; i < argc && i < 64; ++i ){
		if( argv[i][0] != '-' ){
			strncpy( in_fname[in_fcount], argv[i], 256 );
			++in_fcount;
			flagger |= IN_FLAG;
		} else break;
	}

	struct option opts[] = {
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o' },
		{ "tpat", required_argument, NULL, 'T' },
		{ "track", no_argument, &flagger, flagger | TRACK },
		{ "do-or", no_argument, &flagger, flagger | DO_OR },
		{ NULL, 0, NULL, 0 }
	};
	
	char iota = 0; int idx;
	while( (iota = getopt_long( argc, argv, "i:o:T:vtO", opts, &idx )) != -1 ){
		switch( iota ){
			case 'i' :
				strncpy( in_fname[0], optarg, 256 );
				in_fcount = 1;
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
			case 'O' :
				flagger |= DO_OR;
				break;
			default :
				fprintf( stderr, "usage: [-T tpat|-i FILE|-o FILE|-v|-t]\n" );
				exit( 1 );
		}
	}
	
	if( flagger & VERBOSE ) printf( "*** Welcome in the TPAT selector program! ***\n" );
	
	std::vector<XB::data> data, dbuf;
	std::vector<XB::track_info> track, tbuf;
	
	if( flagger & IN_FLAG ){
		for( int i=0; i <  in_fcount; ++i ){
			if( flagger & VERBOSE ) printf( "Reading from %s...\n", in_fname[i] );
			if( flagger & TRACK ){
				XB::load( in_fname[i], tbuf );
				track.insert( track.end(), tbuf.begin(), tbuf.end() );
			} else {
				XB::load( in_fname[i], dbuf );
				data.insert( data.end(), dbuf.begin(), dbuf.end() );
			}
			tbuf.clear();
			dbuf.clear();
		}
	} else {
		if( flagger & VERBOSE ) printf( "Reading from stdin...\n" );
		if( flagger & TRACK ) XB::load( stdin, track );
		else XB::load( stdin, data );
	}
	
	if( flagger & VERBOSE ) printf( "Pruning data...\n" );
	
	int mask = XB::str2tpat( tpat_str );
	int nb_removed, sz_bf = ( flagger & TRACK )? track.size() : data.size();
	if( flagger & TRACK ){
		if( flagger & DO_OR ) nb_removed = select_or_tpat( mask, track );
		else nb_removed = select_and_tpat( mask, track );
	} else {
		if( flagger & DO_OR ) nb_removed = select_or_tpat( mask, data );
		else nb_removed = select_and_tpat( mask, data );
	}
	
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
