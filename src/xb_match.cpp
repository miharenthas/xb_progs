//this little program matches two event_holder sets on the event ID
//and returns all series with only the events common to all series.
//if it's signaled to put on stdout, put the first of the bunch.
/*
EXAMPLE:
data:  1 2 3 4 5 6 7 . .
adata: 1 . 3 . . . 7 . .
returns
data:   3 7 --> stdin & save
adata:  3 7 --> save
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <algorithm>
#include <vector>

#include "xb_io.h"
#include "xb_data.h"
#include "xb_arbitrary_data.h"
#include "xb_cluster.h"
#include "xb_doppc.h" //for the is_event_id functional
                      //and the event_id_comparison
                      //and inspiration

#define MAX_FILES 2

#define VERBOSE 0x0001
#define STDIN_FLAG 0x0002
#define STDOUT_FLAG 0x0004
#define HAVE_DATA 0x0010
#define HAVE_ADATA 0x0020
#define HAVE_TRACK 0x0040
#define HAVE_KLZ 0x0080

enum type_type{
	XB_DATA,
	XB_ADATA,
	XB_TRACK,
	XB_KLZ
};

int main( int argc, char **argv ){
	char in_fnames[MAX_FILES][256];
	char out_fnames[MAX_FILES][256];
	int flagger=0;
	
	int in_fcount=0, out_fcount=0;
	for( int i=1; i < argc && i < MAX_FILES; ++i ){
		if( argv[i][0] == '-' ) break;
		strncpy( in_fnames[fcount], argv[i], 256 );
		++fcount;
	}
	
	char iota=0; int idx;
	struct option opts[] = {
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "put-on-stdout", no_argumen, &flagger, flagger | STDOUT_FLAG },
		{ "get-from-stidn", no_argumen, &flagger, flagger | STDIN_FLAG },
		{ "type", required_argument, NULL, 't' },
		{ "put-to", required_argument, NULL, 'o' },
		{ NULL, 0, NULL, 0 }
	};
	
	type_type tt = XB_DATA; //if no type is specified, assume data.
	while( (iota = getopt_long( argc, argv, "vct:-", opts, &idx )) != -1 ){
		switch( iota ){
			case 'v' :
				flagger |= VERBOSE;
				break;
			case '-' :
				flagger |= STDOUT_FLAG;
				break;
			case 'c' :
				flagger |= STDIN_FLAG;
				break;
			case 't' :
				if( !strcmp( optarg, "data" ) ) tt = XB_DATA;
				else if( !strcmp( optarg, "adata" ) ) tt = XB_ADATA;
				else if( !strcmp( optarg, "track" ) ) tt = XB_TRACK;
				else if( !strcmp( optarg, "klz" ) ) tt = XB_KLZ;
				break;
			case 'o' :
				if( fcount >= MAX_FILES ) break;
				strncpy( out_fnames[out_fcount], optarg );
				++out_fcount;
				break;
			default : exit( 1 );
		}
	}
	
	if( fcount > MAX_FILES ){
		fprintf( stderr, "Can't handle more than 2 sequences.\n" );
		exit( 3 );
	}
	
	//declare all the necessary vectors
	std::vector<XB::data> data[64];
	std::vector<XB::adata> adata[64];
	std::vector<XB::track_info> track[64];
	std::vector<XB::clusterZ> klz[64];
	
	if( flagger & STDIN_FLAG ){
		//shift the filenames
		switch( tt ){
			case XB_DATA :
				XB::load( stdin, data[0] );
				std::sort( data[0].begin(), data[0].end(), event_id_comparison );
				flagger |= HAVE_DATA;
				break;
			case XB_ADATA :
				XB::load( stdin, adata[0] );
				std::sort( adata[0].begin(), adata[0].end(), event_id_comparison );
				flagger |= HAVE_ADATA;
				break;
			case XB_TRACK :
				XB::load( stdin, track[0] );
				std::sort( track[0].begin(), track[0].end(), event_id_comparison );
				flagger |= HAVE_TRACK;
				break;
			case XB_KLZ :
				XB::load( stdin, klz[0] );
				std::sort( klz[0].begin(), klz[0].end(), event_id_comparison );
				flagger |= HAVE_KLZ;
				break;
			default :
				exit( 4 );
		}
	}
	
	//load and sort all the files.
	io_header hdr; int off = ( flagger & STDIN_FLAG ) 1 : 0;
	while( int f=0 ; f < in_fcount; ++f ){
		//getmyheader
		XB::load_header( in_fnames[f], hdr );
		
		//switch-ish on the type
		if( strstr( &hdr.d, "DATA" ) ){
			XB::load( in_fnames[f], data[f+off] );
			std::sort( data[f+off].begin(), data[f+off].end(), event_id_comparison );
			flagger |= HAVE_DATA;
		} else if( strstr( &hdr.d, "ADATA" ) ){
			XB::load( in_fnames[f], adata[f+off] );
			std::sort( adata[f+off].begin(), adata[f+off].end(), event_id_comparison );
			flagger |= HAVE_ADATA;
		} else if( strstr( &hdr.d, "TRACK" ) ){
			XB::load( in_fnames[f], track[f+off] );
			std::sort( track[f+off].begin(), track[f+off].end(), event_id_comparison );
			flagger |= HAVE_TRACK;
		} else if( strstr( &hdr.d, "KLZ" ) )
			XB::load( in_fnames[f], klz[f+off] );
			std::sort( klz[f+off].begin(), klz[f+off].end(), event_id_comparison );
			flagger |= HAVE_KLZ;
		} else {
			fprintf( stderr, "File '%s' isn't valid.\n", in_fnames[f] );
			exit( 2 );
		}
	}
	
	is_event_id is_evnt;
	XB::data *d_iter;
	XB::adata *a_iter;
	XB::track_info *t_iter;
	XB::clusterZ *k_iter;
	
	//now we can assume that we have two files.
	//so we have 10 cases
	int ftyp = flagger & 0xFF00;
	switch( ftyp ){
		case HAVE_DATA : //we just have data
			compare<XB::data,XB::data>( data[0], data[1] );
			break;
		case HAVE_DATA | HAVE_ADATA :
			int d = ( data[0].size() )? 0 : 1;
			compare<XB::data,XB::adata>( data[d], adata[++d%MAX_FILES] );
			break;
		case HAVE_DATA | HAVE_TRACK :
			int d = ( data[0].size() )? 0 : 1;
			compare<XB::data,XB::track_inof>( data[d], track[++d%MAX_FILES] );
			break;
		case HAVE_DATA | HAVE_KLZ :
			int d = ( data[0].size() )? 0 : 1;
			compare<XB::data,XB::clusterZ>( data[d], klz[++d%MAX_FILES] );
			break;
		case HAVE_KLZ : //we just have clusters
			compare<XB::clusterZ,XB::clusterZ>( klz[0], klz[1] );
			break;
		case HAVE_KLZ | HAVE_ADATA :
			int k = ( klz[0].size() )? 0 : 1;
			compare<XB::clusterZ,XB::adata>( klz[k], adata[++k%MAX_FILES] );
			break;
		case HAVE_KLZ | HAVE_TRACK :
			int k = ( klz[0].size() )? 0 : 1;
			compare<XB::clusterZ,XB::track_info>( klz[k], track[++k%MAX_FILES );
			break;
		case HAVE_TRACK :
			compare<XB::track_info,XB::track_info>( track[0], track[1] );
			break;
		case HAVE_TRACK | HAVE_ADATA :
			int t = ( track[0].size() )? 0 : 1;
			compare<XB::track_info,XB::adata>( track[t], adata[++t%MAX_FILES] );
			break;
		case HAVE_ADATA :
			compare<XB::adata,XB::adata>( adata[0], adata[1] );
			break;
	}
	
	if( flagger & STDOUT_FLAG ){
		if( data[0].size() ) XB::write( stdout, data[0] );
		else if( adata[0].size() ) XB::write( stdout, adata[0] );
		else if( track[0].size() ) XB::write( stdout, track[0] );
		else if( klz[0].size() XB::write( stdout, klz[0] );
	}
	
	off = ( flagger & STDOUT_FLAG )? 1 : 0;
	for( int f=0; f < out_fcount; ++f ){
		if( data[f].size() ) XB::write( out_fname[f], data[f+off] );
		else if( adata[f+off].size() ) XB::write( out_fname[f], adata[f+off] );
		else if( track[f+off].size() ) XB::write( out_fname[f], track[f+off] );
		else if( klz[f+off].size() XB::write( out_fname[f], klz[f+off] ); );
	}
	
	return 0;
}

template< class T_one, class T_two >
compare( std::vector<T_one> &one, std::vector<T_two> &two ){
	//...
}
			
	
	
	
	
	
	
