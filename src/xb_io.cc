//XB I/O intergface implemetation
#include "xb_io.h"

//-------------------------------------------------------------------------------
//write an header to a file
void XB::write_header( FILE *f_out, const XB::io_header &hdr ){
	//writes three times the same header to the beginning of a file
	//this should make it robust against corruption (?)
	XB::io_header hhh[] = { hdr, hdr, hdr };
	fwrite( hhh, 3*sizeof(XB::io_header), 1, f_out );
}

//load the header of a file
void XB::load_header( FILE *f_in, XB::io_header &hdr ){
	XB::io_header hhh[3];
	fread( hhh, 3*sizeof(XB::io_header), 1, f_in );

	//copy the safest one
	if( !hhh[0]._n ){ hdr = hhh[0]; return; }
	if( !hhh[1]._n ){ hdr = hhh[1]; return; }
	if( !hhh[2]._n ){ hdr = hhh[2]; return; }
	throw XB::error( "bad header!", "XB::load_header" );
}

//compare two headers
bool XB::operator==( const io_header &left, const io_header &right ){
	if( left.f_version != right.f_version ) return false;
	char dsc_l[11]; strcpy( dsc_l, &left.d );
	char dsc_r[11]; strcpy( dsc_r, &left.d );
	if( strcmp( dsc_l, dsc_r ) ) return false;
	
	return true;
}

//allocate the header
XB::io_header *XB::alloc_header( const unsigned int f_version, const char desc[11] ){
	XB::io_header *hdr = (XB::io_header*)malloc( sizeof( XB::io_header ) );
	hdr->f_version = f_version;
	memcpy( &hdr->d, desc, 11*sizeof(char) );
	
	if( hdr->_n ){
		free( hdr );
		throw XB::error( "bad header!", "XB::alloc_header" );
	}
	
	return hdr;
}

//free the header
void XB::free_header( XB::io_header *hdr ){
	free( hdr );
}

//-------------------------------------------------------------------------------
//The writer bit implementation
void XB::write( FILE* f_out, std::vector<XB::data> &xb_book ){
	//write the the header
	XB::io_header *hdr = alloc_header( 0, XB_FILE_DESCRIPTOR_DATA );
	XB::write_header( f_out, *hdr );
	XB::free_header( hdr );
	
	//begin writing:
	//the format is:
	//header:
	//2*unint + 5*bool + 2*float
	//body:
	//4*n*sizeof(float) + n*sizeof(unsigned int)
	
	//a handy buffer for the single data)
	void *buf = malloc( 5*sizeof(bool) + 2*sizeof(unsigned int) + 2*sizeof(float) );
	bool *flag_buf; 
	unsigned int *u_buf, n;
	float *f_buf;
	//link the pointers
	u_buf = (unsigned int*)buf;
	flag_buf = (bool*)(u_buf + 2);
	f_buf = (float*)(flag_buf + 5);
	for( int i=0; i < xb_book.size(); ++i ){
		//copy the number of elements and event id
		u_buf[0] = xb_book[i].n;
		u_buf[1] = xb_book[i].evnt;
		
		//copy the flags in an orderdly manner
		flag_buf[0] = xb_book[i].empty_t;
		flag_buf[1] = xb_book[i].empty_pt;
		flag_buf[2] = xb_book[i].empty_e;
		flag_buf[3] = xb_book[i].empty_he;
		flag_buf[4] = xb_book[i].empty_sum_e;
		
		//copy the two floats
		f_buf[0] = xb_book[i].sum_e;
		f_buf[1] = xb_book[i].in_beta;
		
		//write it all down
		fwrite( buf, 5*sizeof(bool) + 2*sizeof(unsigned int) + 2*sizeof(float), 1, f_out );
		
		//copy the actual data buffer
		//the array "t" is the beginning of the class' data buffer
		//this is a bit of a hack, but it saves time
		n = xb_book[i].n;
		fwrite( xb_book[i].t, 4*n*sizeof(float) + n*sizeof(unsigned int), 1, f_out );
	}
}

//char* interface
void XB::write( char* f_name, std::vector<XB::data> &xb_book ){
	//build the command for the pipe
	char command[310];
	strcpy( command, "bzip2 -z > " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_out = popen( command, "w" );
	if( f_out == NULL ) throw( XB::error( "I/O Error!", "XB::Write" ) );

	//write
	XB::write( f_out, xb_book );
	
	//close
	pclose( f_out );
}

//std::string interface
void XB::write( std::string f_name, std::vector<XB::data> &xb_book ){
	XB::write( f_name.c_str(), xb_book );
}

//--------------------------------------------------------------------------------
//loader
void XB::load( FILE* f_in, std::vector<XB::data> &xb_book ){
	//check if the vector is empty. If it's not, raise an exception.
	if( !xb_book.empty() ) throw XB::error( "Vector not empty!", "XB::load" );

	//get and check the header
	XB::io_header hdr;
	XB::load_header( f_in, hdr );
	if( !strstr( &hdr.d, "DATA" ) ) throw XB::error( "Wrong data file!", "XB::load" );

	//prepare the buffers
	void *buf = malloc( 5*sizeof(bool) + 2*sizeof(unsigned int) + 2*sizeof(float) ), *data_buf;
	bool *flag_buf; 
	unsigned int *u_buf, n;
	float *f_buf;
	//link the pointers
	u_buf = (unsigned int*)buf;
	flag_buf = (bool*)(u_buf + 2);
	f_buf = (float*)(flag_buf + 5);	
	//read
	while( !feof( f_in ) ){
		//get the number of elemets in the current event
		fread( buf, 5*sizeof(bool) + 2*sizeof(unsigned int) + 2*sizeof(float), 1, f_in );
		
		//construct the class in the back of the vector
		xb_book.push_back( XB::data( u_buf[0], u_buf[1] ) );
		
		//copy the bools
		xb_book.back().empty_t = flag_buf[0];
		xb_book.back().empty_pt = flag_buf[1];
		xb_book.back().empty_e = flag_buf[2];
		xb_book.back().empty_he = flag_buf[3];
		xb_book.back().empty_sum_e = flag_buf[4];
		
		//copy the floats
		xb_book.back().sum_e = f_buf[0];
		xb_book.back().in_beta = f_buf[1];
		
		//get the data
		n = u_buf[0];
		data_buf = (void*)malloc( 4*n*sizeof(float) + n*sizeof(unsigned int) );
		fread( data_buf, 4*n*sizeof(float) + n*sizeof(unsigned int), 1, f_in );
		
		//copy the data in (using the t PoE)
		memcpy( xb_book.back().t, data_buf, 4*n*sizeof(float) + n*sizeof(unsigned int) );
		
		//cleanup
		free( data_buf );
	}
}

//char* interface for the loader
void XB::load( char* f_name, std::vector<XB::data> &xb_book ){
	//build the command for the pipe
	char command[310];
	
	//check if the file exists.
	//NOTE: this bit of code works with glibc on GNU/Linux
	//      no guarantee is provided for other operating systems.
	strcpy( command, "test -f " );
	strcat( command, f_name );
	if( system( command ) ) throw( XB::error( "File doesn't exist!", "XB::load" ) );
	
	strcpy( command, "bunzip2 -c " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_in = popen( command, "r" );
	if( f_in == NULL ) throw( XB::error( "I/O Error!", "XB::load" ) );

	//write
	XB::load( f_in, xb_book );
	
	//close
	pclose( f_in );
}

//std::string interface for the loader.
void XB::load( std::string f_name, std::vector<XB::data> &xb_book ){
	XB::load( f_name.c_str(), xb_book );
}

//-------------------------------------------------------------------------------
//The writer bit implementation for the tracker info
void XB::write( FILE* f_out, std::vector<XB::track_info> &xb_book ){
	//write the the header
	XB::io_header *hdr = alloc_header( 0, XB_FILE_DESCRIPTOR_TRACK );
	XB::write_header( f_out, *hdr );
	XB::free_header( hdr );

	//begin writing:
	//the format is:
	//header:
	//2*unsigned int
	//body:
	//2*float
	//3*n*sizeof(float) + 2*n*sizeof(versor)
	
	//a handy buffer for the various data
	unsigned int *u_buf, n;
	float *f_buf;
	void *buf = malloc( 2*sizeof(unsigned int) + 4*sizeof(float) ); //allocate the buffer
	u_buf = (unsigned int*)buf; //link the uint pointer
	f_buf = (float*)(u_buf + 2); //link the float pointer
	for( int i=0; i < xb_book.size(); ++i ){
		//copy the number of elements and event id
		u_buf[0] = xb_book[i].n;
		u_buf[1] = xb_book[i].evnt;
		f_buf[0] = xb_book[i].in_beta;
		f_buf[1] = xb_book[i].beta_0;
		f_buf[2] = xb_book[i].in_Z;
		f_buf[3] = xb_book[i].in_A_on_Z;
		fwrite( buf, 2*sizeof(unsigned int) + 4*sizeof(float), 1, f_out );
		
		//copy the actual data buffer
		//the array "fragment_A" is the beginning of the class' data buffer
		//this is a bit of a hack, but it saves time
		n = xb_book[i].n;
		fwrite( xb_book[i].fragment_A, 3*n*sizeof(float) + 2*n*sizeof(XB::versor), 1, f_out );
	}
}

//char* interface
void XB::write( char* f_name, std::vector<XB::track_info> &xb_book ){
	//build the command for the pipe
	char command[310];
	strcpy( command, "bzip2 -z > " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_out = popen( command, "w" );
	if( f_out == NULL ) throw( XB::error( "I/O Error!", "XB::Write" ) );

	//write
	XB::write( f_out, xb_book );
	
	//close
	pclose( f_out );
}

//std::string interface
void XB::write( std::string f_name, std::vector<XB::track_info> &xb_book ){
	return XB::write( f_name.c_str(), xb_book );
}

//--------------------------------------------------------------------------------
//loader
void XB::load( FILE* f_in, std::vector<XB::track_info> &xb_book ){
	//check if the vector is empty. If it's not, raise an exception.
	if( !xb_book.empty() ) throw XB::error( "Vector not empty!", "XB::load" );

	//get and check the header
	XB::io_header hdr;
	XB::load_header( f_in, hdr );
	if( !strstr( &hdr.d, "TRACK" ) ) throw XB::error( "Wrong track file!", "XB::load" );

	//prepare the buffers
	void *data_buf; //allocated time by time
	unsigned int *u_buf, n;
	float *f_buf;
	void *buf = malloc( 2*sizeof(unsigned int) + 4*sizeof(float) ); //header buffer
	
	//link the pointers
	u_buf = (unsigned int*)buf;
	f_buf = (float*)(u_buf + 2);
	
	//read
	int count = 0;
	while( !feof( f_in ) ){
		//get the number of elemets in the current event
		fread( buf, 2*sizeof(unsigned int) + 4*sizeof(float), 1, f_in );

		//construct the class in the back of the vector
		xb_book.push_back( XB::track_info( u_buf[0], u_buf[1] ) );

		//set the in_beta and beta_0
		xb_book.back().in_beta = f_buf[0];
		xb_book.back().beta_0 = f_buf[1];
		xb_book.back().in_Z = f_buf[2];
		xb_book.back().in_A_on_Z = f_buf[3];

		//get the data
		n = u_buf[0];
		data_buf = (void*)malloc( 3*n*sizeof(float) + 2*n*sizeof(XB::versor) );
		fread( data_buf, 3*n*sizeof(float) + 2*n*sizeof(XB::versor), 1, f_in );

		//copy the data in (using the fragment_A PoE)
		memcpy( xb_book.back().fragment_A, data_buf,
		        3*n*sizeof(float) + 2*n*sizeof(XB::versor) );

		//cleanup
		free( data_buf );
		++count;
	}
}

//char* interface for the loader
void XB::load( char* f_name, std::vector<XB::track_info> &xb_book ){
	//build the command for the pipe
	char command[310];
	
	//check if the file exists.
	//NOTE: this bit of code works with glibc on GNU/Linux
	//      no guarantee is provided for other operating systems.
	strcpy( command, "test -f " );
	strcat( command, f_name );
	if( system( command ) ) throw( XB::error( "File doesn't exist!", "XB::load" ) );

	strcpy( command, "bunzip2 -c " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_in = popen( command, "r" );
	if( f_in == NULL ) throw( XB::error( "I/O Error!", "XB::load" ) );

	//write
	XB::load( f_in, xb_book );
	
	//close
	pclose( f_in );
}

//std::string interface for the loader.
void XB::load( std::string f_name, std::vector<XB::track_info> &xb_book ){
	XB::load( f_name.c_str(), xb_book );
}

//--------------------------------------------------------------------------------
//the wirter implementation for XB::clusterZ
//NOTE: in what follows, for now, cluster.dists is NOT wirtten
//      because it's not filled. The code is there, though:
//      just uncomment it when ready.
void XB::write( FILE* f_out, std::vector<XB::clusterZ> &event_klZ ){
	//write the the header
	XB::io_header *hdr = alloc_header( 0, XB_FILE_DESCRIPTOR_CLUSTERS );
	XB::write_header( f_out, *hdr );
	XB::free_header( hdr );

	//the format is:
	//header:
	//1*unsigned int - the.n
	//body:
	//n*clusters:
	//  2*unsigned int
	//  3*float
	//  n*float
	//  n*unsigned int
	
	//allocate some buffer
	//this will make the life of the writer a bit better
	void *uf_buf = malloc( 2*sizeof(unsigned int) + 3*sizeof(float) );
	unsigned int *u_buf = (unsigned int*)uf_buf;
	float *f_buf = (float*)((unsigned int*)uf_buf + 2);
	
	//loop on the events
	for( int i=0; i < event_klZ.size(); ++i ){
		//write the header( 2 uints and 1 float )
		fwrite( &event_klZ[i].n, 2*sizeof(unsigned int) + sizeof(float), 1, f_out );
		
		//loop on the clusters
		for( int k=0; k < event_klZ[i].n; ++k ){
			//fill the buffer
			u_buf[0] = event_klZ[i].clusters[k].n;
			u_buf[1] = event_klZ[i].clusters[k].centroid_id;
			f_buf[0] = event_klZ[i].clusters[k].c_altitude;
			f_buf[1] = event_klZ[i].clusters[k].c_azimuth;
			f_buf[2] = event_klZ[i].clusters[k].sum_e;
			//write
			fwrite( uf_buf, 2*sizeof(unsigned int) + 3*sizeof(float), 1, f_out );
			fwrite( &event_klZ[i].clusters[k].crys_e[0],
			        event_klZ[i].clusters[k].n*sizeof(float),
			        1, f_out );
			fwrite( &event_klZ[i].clusters[k].crys[0],
			        event_klZ[i].clusters[k].n*sizeof(unsigned int),
			        1, f_out );
		}
	}
	
	//cleanup
	free( uf_buf );
}
			
//char* interface
void XB::write( char* f_name, std::vector<XB::clusterZ> &event_klZ ){
	//build the command for the pipe
	char command[310];
	strcpy( command, "bzip2 -z > " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_out = popen( command, "w" );
	if( f_out == NULL ) throw( XB::error( "I/O Error!", "XB::Write" ) );

	//write
	XB::write( f_out, event_klZ );
	
	//close
	pclose( f_out );
}

//std::string interface
void XB::write( std::string f_name, std::vector<XB::clusterZ> &event_klZ ){
	return XB::write( f_name.c_str(), event_klZ );
}

//-------------------------------------------------------------------------------------------
//the loader for the clusters
void XB::load( FILE* f_in, std::vector<XB::clusterZ> &event_klZ ){
	if( !event_klZ.empty() )
		throw XB::error( "Vector not empty!", "XB::load" );
	
	//get and check the header
	XB::io_header hdr;
	XB::load_header( f_in, hdr );
	if( !strstr( &hdr.d, "KLZ" ) ) throw XB::error( "Wrong cluster file!", "XB::load" );
	
	void *uf_buf = malloc( 2*sizeof(unsigned int) + 3*sizeof(float) );
	unsigned int *u_buf = (unsigned int*)uf_buf;
	float *f_buf = (float*)((unsigned int*)uf_buf + 2);
	clusterZ klZ;
	void *klZ_begin = &klZ.n; //point of entry for the structures
	
	while( !feof( f_in ) ){
		fread( klZ_begin, 2*sizeof(unsigned int)+sizeof(float), 1, f_in );
		
		klZ.clusters = std::vector<cluster>( klZ.n );
		for( int k=0; k < klZ.n; ++k ){
			fread( uf_buf, 2*sizeof(unsigned int) + 3*sizeof(float), 1, f_in );
			
			klZ.clusters[k].n = u_buf[0];
			klZ.clusters[k].centroid_id = u_buf[1];
			klZ.clusters[k].c_altitude = f_buf[0];
			klZ.clusters[k].c_azimuth = f_buf[1];
			klZ.clusters[k].sum_e = f_buf[2];
		
			klZ.clusters[k].crys_e.resize( u_buf[0] );
			klZ.clusters[k].crys.resize( u_buf[0] );
			
			fread( &klZ.clusters[k].crys_e[0],
			       u_buf[0]*sizeof(float),
			       1, f_in );
			fread( &klZ.clusters[k].crys[0],
			       u_buf[0]*sizeof(unsigned int),
			       1, f_in );
		}
		
		event_klZ.push_back( klZ );
	}
	
	free( uf_buf );
}
	
//char* interface for the loader
void XB::load( char* f_name, std::vector<XB::clusterZ> &event_klZ ){
	//build the command for the pipe
	char command[310];
	
	//check if the file exists.
	//NOTE: this bit of code works with glibc on GNU/Linux
	//      no guarantee is provided for other operating systems.
	strcpy( command, "test -f " );
	strcat( command, f_name );
	if( system( command ) ) throw( XB::error( "File doesn't exist!", "XB::load" ) );
	
	strcpy( command, "bunzip2 -c " );
	strcat( command, f_name );
	
	//open the pipe and test
	FILE* f_in = popen( command, "r" );
	if( f_in == NULL ) throw( XB::error( "I/O Error!", "XB::load" ) );

	//write
	XB::load( f_in, event_klZ );
	
	//close
	pclose( f_in );
}

//std::string interface for the loader.
void XB::load( std::string f_name, std::vector<XB::clusterZ> &event_klZ ){
	XB::load( f_name.c_str(), event_klZ );
}
