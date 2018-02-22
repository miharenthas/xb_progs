//a simple program that runs the selection on the tpat thingie
#include <stdio.h>
#include <vector>
#include <getopt.h>

#include "xb_data.h"
#include "xb_io.h"
#include "xb_tpat.h"

#define IN_FLAG 0x01
#define OUT_FLAG 0x02
#define OUT_STATS 0x04
#define PARSE_HEX 0x08
#define VERBOSE 0x10
#define TRACK 0x20
#define DO_OR 0x40
#define DO_STATS 0x80

void print_help();

int main( int argc, char **argv ){
	char in_fname[64][256];
	char out_fname[256];
	char stats_fname[256];
	char tpat_str[256]; strcpy( tpat_str, "minb" );
	int flagger = 0;
	int in_fcount = 0;
	FILE *stat_stream = stdout;

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
		{ "thex", required_argument, NULL, 'X' },
		{ "save-stats", required_argument, NULL, 'S' },
		{ "help", no_argument, NULL, 'h' },
		{ "track", no_argument, &flagger, flagger | TRACK },
		{ "do-or", no_argument, &flagger, flagger | DO_OR },
		{ "stats", no_argument, &flagger, flagger | DO_STATS },
		{ NULL, 0, NULL, 0 }
	};
	
	char iota = 0; int idx;
	while( (iota = getopt_long( argc, argv, "i:o:T:X:S:vtOsh", opts, &idx )) != -1 ){
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
			case 'X' :
				strncpy( tpat_str, optarg, 256 );
				flagger |= PARSE_HEX;
				break;
			case 'S' :
				strncpy( stats_fname, optarg, 256 );
				flagger |= DO_STATS | OUT_STATS;
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
			case 's' :
				flagger |= DO_STATS;
				//TODO: make the optional argument work
				break;
			case 'h' :
				print_help();
				exit( 0 );
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
	
	gsl_histogram *stats;
	if( flagger & OUT_STATS ){
		stat_stream = fopen( stats_fname, "w" );
		if( !stat_stream ){
			fputs( "error: stat file not open.\n", stderr );
			exit( 1 );
		}
	}
	if( flagger & DO_STATS ){
		stats = XB::tpat_stats_alloc();
		if( flagger & VERBOSE ) printf( "Doing stats...\n" );
		if( flagger & TRACK ) XB::tpat_stats_fill( stats, track );
		else XB::tpat_stats_fill( stats, data );
		
		XB::tpat_stats_printf( stat_stream, stats );
		
		XB::tpat_stats_free( stats );
		if( stat_stream != stdout ) fclose( stat_stream );
	}
	
	if( flagger & VERBOSE ) printf( "Pruning data...\n" );
	
	int mask;
	if( flagger & PARSE_HEX ) mask = XB::hex2tpat( tpat_str );
	else mask = XB::str2tpat( tpat_str );
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
	
	if( strstr( "/dev/null", out_fname ) ) goto __END__;
	if( flagger & OUT_FLAG ){
		if( flagger & TRACK ) XB::write( out_fname, track );
		else XB::write( out_fname, data );
	} else {
		if( flagger & TRACK ) XB::write( stdout, track );
		else XB::write( stdout, data );
	}
	
	__END__:
	if( flagger & VERBOSE ) puts( "Done. Goodbye." );
	
	return 0;
}

//------------------------------------------------------------------------------------
//let's not have to read the source code for the flags name every time
void print_help(){
	puts( "xb_trigger_siv: a program to siv on the trigger (Tpat)" );
	puts( "options:" );
	puts( "\t-v | --verbose" );
	puts( "\t-i | --input [FILE]" );
	puts( "\t-o | --output [FILE]" );
	puts( "\t-T | --tpat [tpat description, see below]" );
	puts( "\t-X | --thex [some hexes for the mask]" );
	puts( "\t-S | --save-stats [FILE]" );
	puts( "\t-s | --stats" );
	puts( "\t-t | --track" );
	puts( "\t-O | --do-or" );
	puts( "\t-h | --help" );
	puts( "\ntpat flag names:" );
	puts( "\tminb     -- minimum bias" );
	puts( "\tfrag     -- NTF fired" );
	puts( "\tntr      -- LAND trigger" );
	puts( "\tcbsumf   -- XB forward" );
	puts( "\tcbsum    -- Crystal Ball" );
	puts( "\tcbor     -- Either half of XB" );
	puts( "\tpix      -- pixel detector" );
	puts( "\tlandc    -- offspill, LAND cosmic" );
	puts( "\ttfwc     -- offspill, TFW cosmic" );
	puts( "\tntfc     -- offspill, NTF cosmic" );
	puts( "\tcbc      -- offspill, XB cosmic" );
	puts( "\tcboffsp  -- offspill, XB sum" );
	puts( "\tpixoffsp -- offspill, pixel" );
}
	
