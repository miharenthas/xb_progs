//this tool extracts a (simple --> single) detector data loaded into the
//arbitrary structure
//it's not an ability of xb_data_translator largely because of lazy

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <vector>

#include "xb_reader.h"
#include "xb_io.h"
#include "xb_arbitrary_data.h"

//------------------------------------------------------------------------------------
//sm' flags
#define VERBOSE 0x0001
#define OUT_FLAG 0x0002

//------------------------------------------------------------------------------------
//a parser for the data structure spec
//remember: farr is null-ish terminated
int xb_getarb__parse( XB::adata_field **farr, const char *spec );

//------------------------------------------------------------------------------------
//the main (no globclass here, yet)
int main( int argc, char **argv ){
	char in_fname[64][256];
	char out_fname[256];
	char spec[512]; strcpy( spec, "Xbn[Xbi:Xbpt:Xbt:Xbe:Xbhe]4" );
	short int flagger = 0; //c'mon, 16 bits are enough
	
	//usual retrieval of filenames
	int in_fcount = 0;
	for( int i=1; i < argc && i < 64; ++i ){
		if( argv[i][0] == '-' ) break;
		strncpy( in_fname[in_fcount], argv[i], 256 );
		++in_fcount;
	}
	
	struct option opts[] = {
		{ "verbose", no_argument, &flagger, flagger | VERBOSE },
		{ "read-from", required_argument, NULL, 'i' },
		{ "write-to", required_argument, NULL, 'o' },
		{ "fields", required_argument, NULL, 'F' },
		{ NULL, 0, NULL, 0 }
	};
	
	char iota=0;
	int idx;
	while( (iota = getopt_long( argc, argv, "i:o:F:v", opts, &idx )) != -1 ){
		switch( iota ){
			case 'v' :
				flagger |= VERBOSE;
				break;
			case 'i' :
				strncpy( in_fname[0], optarg, 256 );
				in_fcount = 1;
				break;
			case 'o' :
				strncpy( out_fname, optarg, 256 );
				flagger |= OUT_FLAG;
				break;
			case 'F' :
				strncpy( spec, optarg, 512 );
				break;
			default :
				fprintf( stderr, "YOU made a mistake.\n" );
				exit( 1 );
		}
	}
	
	if( flagger & VERBOSE ) puts( "*** Welcome in the XB arbitrary data loader ***" );
	
	//consistency check
	if( !in_fcount ){
		fprintf( stderr, "Sorry: cannot read ROOT stuff from a pipe.\n" );
		exit( 2 );
	}
	if( flagger & VERBOSE && !( flagger & OUT_FLAG ) ){
		fprintf( stderr, "Verbose and putting on pipe: are YOU sure?\n" );
		exit( 3 );
	} 
	
	//parse the spec
	if( flagger & VERBOSE ) puts( "Parsing spec..." );
	XB::adata_field *farr;
	int nf = xb_getarb_parse( &farr, spec );
	
	//load all the files
	if( flagger & VERBOSE ) puts( "Loading files..." );
	std::vector<XB::adata> book, buf;
	for( int f=0; f < in_fcount; ++f ){
		try{
			if( flagger & VERBOSE ) printf( "\tLoading %s...\n", in_fname[f] );
			XB::arb_reader( buf, in_fname[f], farr );
			data.insert( data.end(), buf.begin(), buf.end() );
		} catch( XB::error e ) {
			fprintf( stderr, "File %s ended badly with %s\n",
			         in_fname[f], e.what() );
		}
		buf.clear();
	}
	
	//and write all the data
	if( flagger & VERBOSE ) puts( "Putting data..." )
	if( flagger & OUT_FLAG ) XB::write( out_fname, data );
	else XB::write( stdout, data );
	
	if( fagger & VERBOSE ) puts( "Done. Goodbye." );
	return 0;
}

//------------------------------------------------------------------------------------
//the parser
int xb_getarb_parse( XB::adata_field **farr, const char *str ){
	char lead[16], fields[495];
	
	//get the size of the fields (in bytes*lead_field)
	int fsize;
	sscanf( "%s[%s]%d", lead, fields, &fsize ); 
	
	//count the fields
	char *head = fields;
	int nf = 1;
	while( *head != '\0' ) if( *head == ':' ) ++nf;
	
	*farr = (XB::adata_field*)calloc( nf*sizeof(XB::adata_field)+1, 1 );
	
	//read the fields
	head = strtok( spec, ":" );
	int i=0;
	while( head ){
		strcpy( farr[i].name, head );
		farr[i].size = fsize;
		strtok( NULL, ":" );
	} //and the last one is already nullified
	
	return nf;
}
