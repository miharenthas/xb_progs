//this program translates the data from the TTree to a humane format
//it can also be piped into another program(!!!)

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "xb_io.h"
#include "xb_reader.h"

using namespace std;

//------------------------------------------------------------------------------------
//a handy data structure to hold the program's settings
struct translator_settings{
	bool in_flag;
	bool out_flag;
	bool check_flag;
	bool track_flag;
	bool sim_flag;
	bool verbose;
	char in_f_name[64][256];
	char out_f_name[256];
	int in_f_count;
};

//------------------------------------------------------------------------------------
//load, write and check wrapper class
template< class xb_data_type >
class xb_data_translator{
	public:
		//ctor, dtor
		xb_data_translator( struct translator_settings &given_settings );
		~xb_data_translator();
		
		//methods
		void data_loader(); //read the data, according to the template
		void data_putter(); //puts the data, where and how according to the template
		void check();	//checks the data
	private:
		xb_data_translator(); //this class cannot be dfault constructed
		                      //you need the options
		vector<xb_data_type*> xb_book; //the read data
		struct translator_settings settings; //the settings
};

//------------------------------------------------------------------------------------
//main
int main( int argc, char** argv ){
	//input variables
	//are now all held into the settings struct
	struct translator_settings settings;
	
	//parse (and count) eventual free file names
	//at the beginning of the input
	settings.in_f_count = 0;
	for( int i=1; i < argc && settings.in_f_count < 64; ++i ){
		if( argv[i][0] != '-' && strlen( argv[i] ) < 256 ){
			strcpy( settings.in_f_name[settings.in_f_count], argv[i] );
			++settings.in_f_count;
			settings.in_flag = true;
		} else if( argv[i][0] == '-' ) break;
	}
	
	//long options
	struct option opts[] = {
		{ "output-file", required_argument, NULL, 'o' },
		{ "check-output", no_argument, NULL, 'c' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "tracking-data", no_argument, NULL, 't' },
		{ "simulation-data", no_argument, NULL, 's' },
		{ "help", no_argument, NULL, 'h' },
		{ 0, 0, 0, 999 }
	};

	//further input parsing
	char iota = 0; int opt_idx = 0;
	while( (iota = getopt_long( argc, argv, "i:o:cvts", opts, &opt_idx )) != -1 ){
		switch( iota ){
			case 'i':
				if( strlen( optarg ) > 256 ){
					printf( "Input file name is too long!\n" );
					exit( 1 );
				}
				strcpy( settings.in_f_name[0], optarg );
				settings.in_f_count = 1;
				settings.in_flag = true;
				break;
			case 'o':
				if( strlen( optarg ) > 256 ){
					printf( "Output file name is too long!\n" );
					exit( 1 );
				}
				strcpy( settings.out_f_name, optarg );
				settings.out_flag = true;
				break;
			case 'c':
				settings.check_flag = true;
				break;
			case 'v':
				settings.verbose = true;
				break;
			case 't':
				settings.track_flag = true;
				break;
			case 's':
				settings.sim_flag = true;
				break;
			case 'h':
				system( "cat doc/xb_data_translator" );
				exit( 0 );
			default :
				fprintf( stderr,
				         "%c is not a valid command. Type --help for more.\n", iota );
				return 0;
		}
	}
	
	//std greetings
	if( settings.verbose ) printf( "*** Welcome to the TTree to XB::data translator! ***\n" );
	
	//do we have an input file?
	if( !settings.in_flag ){
		cerr << "No input file specified: ROOT doesn't support it." << endl;
		exit( 1 );
	}
	
	//create the program's objects
	xb_data_translator<XB::data> data_engine( settings ); //works also for simulations
	xb_data_translator<XB::track_info> track_engine( settings );
	
	//load the data
	try{
		if( settings.track_flag ) track_engine.data_loader();
		else data_engine.data_loader();
	} catch( XB::error e ) {
		cerr << "There has been an error: " << e.what << endl;
		exit( 1 );
	}
	
	//output the data
	try{
		if( settings.track_flag ) track_engine.data_putter();
		else data_engine.data_putter();
	} catch( XB::error e ) {
		cerr << "There has been an error: " << e.what << endl;
		exit( 1 );
	}
	
	//check the data
	try{
		if( settings.track_flag && settings.check_flag ) track_engine.check();
		else if( !settings.track_flag && settings.check_flag ) data_engine.check();
	} catch( XB::error e ) {
		cerr << "There has been an error: " << e.what << endl;
		exit( 1 );
	}
	
	if( settings.verbose ) printf( "*** Done, goodbye. ***\n" );
	return 0;
}

//------------------------------------------------------------------------------------
//implementation of the class

//constructor
template< class xb_data_type >
xb_data_translator<xb_data_type>::xb_data_translator( struct translator_settings &given_s ):
	xb_book( 0 ) //just to say that we initialized it...
{
	//copy the settings...
	settings.in_flag = given_s.in_flag;
	settings.out_flag = given_s.out_flag;
	settings.check_flag = given_s.check_flag;
	settings.track_flag = given_s.track_flag;
	settings.verbose = given_s.verbose;
	settings.sim_flag = given_s.sim_flag;
	
	strcpy( settings.out_f_name, given_s.out_f_name );
	
	settings.in_f_count = given_s.in_f_count;
	for( int i=0; i < settings.in_f_count; ++i )
		strcpy( settings.in_f_name[i], given_s.in_f_name[i] );
}

//destructor
template< class xb_data_type >
xb_data_translator<xb_data_type>::~xb_data_translator(){} //nothing to do...

//------------------------------------------------------------------------------------
//data loader
template< class xb_data_type >
void xb_data_translator<xb_data_type>::data_loader(){
	//load the TTree into the vector (loop on the files)
	vector<xb_data_type*> xb_book_buf;
	for( int i=0; i < settings.in_f_count; ++i ){
		if( settings.verbose ) cout << "Reading from " << settings.in_f_name[i] << "..." << endl;

		//do the reading:
		//first try to open the file, if it makes it store it at the end of xb_bok
		//if not, check the error type: continue if a file is not found and freak out
		//in all other cases
		try{
			if( settings.sim_flag ) XB::sim_reader( xb_book_buf, settings.in_f_name[i] );
			else XB::reader( xb_book_buf, settings.in_f_name[i] );
			xb_book.insert( xb_book.end(), xb_book_buf.begin(), xb_book_buf.end() ); //cat it at the end
		}catch( XB::error e ){
			if( !strcmp( e.what, "File error!XB::reader" ) ){
				if( settings.verbose ) printf( "File \"%s\" not found.\n", settings.in_f_name[i] );
				continue;
			} else {
				throw e;
			}
		}
		
		//cleanup
		xb_book_buf.clear();
	}
	
	//check that there has been some input
	if( xb_book.empty() ){
		throw XB::error( "No data!", "xb_data_translator" );
	}
}

//------------------------------------------------------------------------------------
//data putter
template< class xb_data_type >
void xb_data_translator<xb_data_type>::data_putter(){
	//save it into our cute format
	if( settings.verbose && settings.out_flag )
		cout << "Writing to " << settings.out_f_name << "..." << endl;
	try{
		if( settings.out_flag ) XB::write( settings.out_f_name, xb_book );
		else XB:write( stdout, xb_book );
	}catch( XB::error e ){
		throw e;
	}
}

//------------------------------------------------------------------------------------
//performs the check
template< class xb_data_type >
void xb_data_translator<xb_data_type>::check(){
	vector<xb_data_type*> xb_book_check;
	//verify-ish
	if( settings.out_flag ){
		if( settings.verbose ) cout << "Veryfiyng..." << endl;
		try{
			XB::load( settings.out_f_name, xb_book_check );
		}catch( XB::error e ){
			throw e;
		}
		
		if( settings.verbose ) printf( "Entry #: " );
		for( int i=0; i < xb_book.size(); ++i ){
			if( settings.verbose && i ) printf( "\b\b\b\b\b\b\b\b\b\b" );
			if( settings.verbose ) printf( "%010d", i );
			
			if( *xb_book[i] != *xb_book_check.at(i) ){
				cerr << endl << "Ooops, screwed up.";
				break;
			}
		}
		if( settings.verbose ) putchar( '\n' );
	}
}
