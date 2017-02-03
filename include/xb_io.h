//this is the I/O interface for the class XB::data header file
#ifndef XB_IO__H
#define XB_IO__H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "xb_data.h"
#include "xb_cluster.h"
#include "xb_error.h"

#define XB_FILE_DESCRIPTOR_DATA "DATADATA@@"
#define XB_FILE_DESCRIPTOR_TRACK "TRACKTRACK"
#define XB_FILE_DESCRIPTOR_CLUSTERS "KLZKLZKLZ@"

namespace XB{
	//-----------------------------------------------------------------------
	//A data structure containung header information
	typedef struct _xb_io_header_information {
		unsigned int f_version;
		char d, e, s, c, r, i, p, t, o, R, _n;
	} io_header;
	
	//-----------------------------------------------------------------------
	//header operations:
	//this should provide some (little) protection against wrong file reading
	//and provide versioning information (if the file layout changes)
	void write_header( FILE *f_out, const io_header &hdr );
	void load_header( FILE *f_in, io_header &hdr );
	bool operator==( const io_header &left, const io_header &right );
	io_header *alloc_header( const unsigned int f_version, const char desc[11] );
	void free_header( io_header *hdr );
	
	//-----------------------------------------------------------------------
	//write the data
	//to save space, this uses a pipe to bzip2 and saves compressed.
	
	//this block of functions work with XB::data kind of data
	void write( FILE* f_out, std::vector<XB::data> &xb_book );
	void write( std::string f_name, std::vector<XB::data> &xb_book );
	void write( char* f_name, std::vector<XB::data> &xb_book );
	
	//this block of functions work with XB::data kind of data
	void write( FILE* f_out, std::vector<XB::track_info> &xb_book );
	void write( std::string f_name, std::vector<XB::track_info> &xb_book );
	void write( char* f_name, std::vector<XB::track_info> &xb_book );

	//this block of functions work wiht XB::clusterZ kind of data
	void write( FILE* f_out, std::vector<XB::clusterZ> &event_klZ );
	void write( std::string f_name, std::vector<XB::clusterZ> &event_klZ );
	void write( char* f_name, std::vector<XB::clusterZ> &event_klZ );
	
	//-----------------------------------------------------------------------
	//load from file
	//assumes that the file has been written with "write", so it pipes
	//though bunzip2
	
	//this block of functions work with XB::data kind of data
	void load( FILE* f_in, std::vector<XB::data> &xb_book );
	void load( std::string f_name, std::vector<XB::data> &xb_book );
	void load( char* f_name, std::vector<XB::data> &xb_book );

	//this block of functions work with XB::data kind of data
	void load( FILE* f_in, std::vector<XB::track_info> &xb_book );
	void load( std::string f_name, std::vector<XB::track_info> &xb_book );
	void load( char* f_name, std::vector<XB::track_info> &xb_book );
	
	//this block of functions work with XB:clusterZ kind of data
	void load( FILE* f_in, std::vector<XB::clusterZ> &event_klZ );
	void load( std::string f_name, std::vector<XB::clusterZ> &event_klZ );
	void load( char* f_name, std::vector<XB::clusterZ> &event_klZ );
	
}

#endif
