//little test program for the adata structure.

#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "xb_arbitrary_data.h"
#include "xb_io.h"
#include "xb_reader.h"

void tester( std::vector<XB::adata> &str, int ns, int fcount );
char fields[256][64];

int main( int argc, char **argv ){

	int fcount = 0;
	for( int i=1; i < argc; ++i ){
		if( argv[i][0] == '-' ) break;
		strncpy( fields[fcount], argv[i], 64 );
		++fcount;
	}
	
	char iota=0;
	int ns=10; //number of structures to allocate
	while( (iota = getopt( argc, argv, "n:" )) != -1 ){
		switch( iota ){
			case 'n':
				ns = atoi( optarg );
				break;
			default :
				fprintf( stderr, "YOU made a mistake.\n" );
				exit( 1 );
		}
	}
	
	puts( "*** Weclome in XB::adata test program ***" );
	
	//should try the standard constructor
	std::vector<XB::adata> str( ns ), vec;
	puts( "A vector of adata's allocated." );
	
	tester( str, ns, fcount );
	
	XB::write( "astr3.axb", str );
	puts( "Written a file" );
	XB::load( "astr3.axb", vec );
	puts( "Read the file" );
	
	if( str.size() != vec.size() ){ puts( "Wrong answer!" ); exit( 3 ); }

	for( int i=0; i < str.size(); ++i  ) if( str[i] != vec[i] ) puts( "Broken copy!" );
	
	vec.clear();
	XB::adata_field farr[] = {
		{ "Xbn", 4 },
		{ "Xbpt", 4 },
		{ "Xbi", 4 },
		{ "", 0 }
	};
	
	XB::arb_reader( vec, "try.root", farr );
	printf( "Read BURP file, %d entries\n", vec.size() );
	
	std::vector<XB::adata_field> fld;
	for( int i=0; i < 10 && i < vec.size(); ++i ){
		fld = vec[i].lsfields();
		printf( "\t%s\n", fld[0].name );
		printf( "\t%s\n", fld[1].name );
	}
	
	return 0;
}

void tester( std::vector<XB::adata> &str, int ns, int fcount ){
	XB::adata_field *farr = (XB::adata_field*)malloc( fcount*sizeof(XB::adata_field) );
	for( int f=0; f < fcount; ++f ){
		strncpy( farr[f].name, fields[f], 16 );
		farr[f].size = 4*sizeof(float);
	}
	puts( "Created a field list." );
	
	//try the parametric constructo
	XB::adata astr( farr, fcount );
	puts( "Created a structure from a field list." );
	
	//try lsfields
	std::vector<XB::adata_field> flist = astr.lsfields(); //list the fields
	puts( "Listed the fields:" );
	float data[] = {1.5, 2.5, 3.5, 4.5};
	for( int i=0; i < fcount; ++i ){
		astr.dofield( fields[i], 4*sizeof(float), data ); //dofield in copy
		printf( "\tCopied field %s\n", fields[i] );
	}
	
	for( int i=0; i < ns; ++i ) str[i] = astr; //assignmet op and copy ctor (the same)
	puts( "Copied the structures in the vector." );
	
	for( int i=0; i < ns; ++i ) if( str[i] != astr ) puts( "Broken copy!" );
	puts( "Verified the copies in the vector." );
	
	str.clear();
	puts( "Vector cleared." );
	for( int i=0; i < ns; ++i ) str.push_back( astr ); //assignmet op and copy ctor (the same)
	puts( "Copy-construted the structures in the vector." );
	
	//this actually works?
	for( int i=0; i < ns-1; ++i ) if( str[i] != str[i+1] ) puts( "Broken copy!" );
	puts( "Verified the copies in the vector." );
	
	astr.dofield( "-prak", 4*sizeof(float), NULL ); //dofield, just alloc
	puts( "Allocated empty field." );
	astr.dofield( "-prakkino", 4*sizeof(float), data ); //dofield alloc and copy
	astr.dofield( "-prakkuccio", 2*sizeof(float), data );
	astr.dofield( "-prakkello", 3*sizeof(float), data );
	puts( "Allocated full fields." );
	astr.rmfield( fields[0] );
	astr.rmfield( "-prakkuccio" );
	puts( "Removed fields." );
	
	float *redata = (float*)astr( "-prakkino" );
	puts( "Copied data from a field:" );
	printf( "\tredata = { %f %f %f %f }\n", redata[0], redata[1], redata[2], redata[3] );
	printf( "\tredata[0] = %f\n", astr.tip<float>( "-prakkino" ) );
	
	free( redata ); //we'll play with it later again.
	redata = NULL;
	
	XB::adata bstr( astr );
	if( astr == bstr ) puts( "Copy-constructed structure:" );
	else{ puts( "Copy construction failed" ); exit( 2 ); }
	redata = (float*)bstr( "-prakkino" );
	printf( "\tredata = { %f %f %f %f }\n", redata[0], redata[1], redata[2], redata[3] );
	printf( "\tredata[0] = %f\n", bstr.tip<float>( "-prakkino" ) );
	bstr.clear();
	puts( "Cleared bstr." );
	
	void *linbuf;
	int lb_size = XB::adata_getlbuf( &linbuf, astr );
	printf( "Linearized a structure, it's %d long.\n", lb_size );
	astr.clear();
	puts( "Astr cleared." );
	XB::adata_fromlbuf( bstr, linbuf );
	puts( "Got a struct from a linear buffer:" );
	redata = (float*)bstr( "-prak" );
	printf( "\tredata = { %f %f %f %f }\n", redata[0], redata[1], redata[2], redata[3] );
	printf( "\ttip = %f\n", bstr.tip<float>( "-prakkello" ) );

	free( farr );
	free( redata );
	puts( "Objects destroyed.\n" );
}
	
	
	
	
	
	
