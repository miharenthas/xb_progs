//this program doppler corrects the XB::data

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

#include <algorithm>
#include <functional>
#include <vector>

#include "xb_doppler_corr.h"
#include "xb_data.h"
#include "xb_io.h"

//------------------------------------------------------------------------------------
//a handy structure to control the operation mode
enum correct_mode{
	RELAXED = 0, //correct everything; when a beta_0 isn't available
	             //interpolate (fit) the available data to get an estimate
	FASTIDIOUS, //correct and propagate only
	            //those events for which track data are available
	VERY_FASTIDIOUS, //correct and propagate only those events
	                 //for which track data are available AND
	                 //only one fragment is detected. Also, this mode
	                 //uses the outgoing direction from the tracker.
	SIMULATION //apply the doppler correction to a simulation
	           //which of course doesn't have any track information
	           //so the track info for a run is borrowed and acessed
	           //randomly.
};

//------------------------------------------------------------------------------------
//a little functional to look for elements by event id
class is_event_id : public std::unary_function< XB::event_holder*, bool > {
	public:
		//constructors
		is_event_id(): evnt( 0 ) {}; //default
		is_event_id( unsigned int the_evnt ): evnt( the_evnt ) {}; //from uint
		is_event_id( XB::event_holder &given  ): evnt( given.evnt ) {}; //from XB::event_holder
		is_event_id( is_event_id &given ): evnt( given.evnt ) {}; //copy
		
		//the main thing
		bool operator()( XB::event_holder *given ){ return given->evnt == evnt; };
		
		//various type of assignement
		is_event_id &operator=( const unsigned int given ){ evnt = given; return *this; };
		is_event_id &operator=( const is_event_id &given ){ evnt = given.evnt; return *this; };
		is_event_id &operator=( const XB::event_holder &given ){ evnt = given.evnt; return *this; };
	
	private:
		//the datum for comparison
		unsigned int evnt;
};

//------------------------------------------------------------------------------------
//the wrapper function that applies the doppler correction
void apply_doppler_correction( std::vector<XB::data*> &xb_book,
                               std::vector<XB::track_info*> &xb_track_book,
                               unsigned int default_beam_out, correct_mode mode,
                               bool verbose );

//the (piped) interface to read a root file with xb_data_translator
void translate_track_info( std::vector<XB::track_info*> &xb_track_book,
                           unsigned int track_f_count, char track_f_name[][256],
                           bool verbose );
//an utility for sorting
bool evnt_id_comparison( const XB::event_holder *one, const XB::event_holder *two );

//------------------------------------------------------------------------------------
//the main
int main( int argc, char **argv ){
	//input variables
	char in_f_name[256]; //input DATA file
	char track_f_name[64][256]; //input TRACK file
	char out_f_name[256]; //output file
	unsigned int track_f_count = 0; //# of track files
	unsigned int default_beam_out = 81; //the default beam in

	//flags
	bool verbose = false; //if true, verbose output
	bool in_flag = false; //if true, read from a file and not stdin
	bool track_flag = false; //if true, read the track info from file and not stdin
	bool out_flag = false; //if true, write to a file and not to stdout
	bool reader_flag = false; //if true, use xb_data_translator to read the TRACK file
	correct_mode mode = RELAXED; //the default is: correct everything
	                             //at the best of your capabilities.
	
	//------------------------------------
	//input parsing
	//first look at the first unminused entry, which is supposed to be
	//the file containing the tracker information.
	for( int i=1; i < argc && i < 64; ++i ){
		if( argv[i][0] != '-' && strlen( argv[i] ) < 256 ){
			strcpy( track_f_name[i-1], argv[i] );
			++track_f_count;
		} else if( argv[i][0] == '-' ) break; //exit the loop at the first option
		                                      //and silently discard file names that
		                                      //are too long.
	}
	
	//set the flag for stdin reading
	if( track_f_count != 0 ) track_flag = true;
		
	char iota = 0;
	while( (iota = getopt( argc, argv, "d:o:RfFsvb:" )) != -1 ){
		switch( iota ){
			case 'd' :
				if( strlen( optarg ) < 256 ){
					strcpy( in_f_name, optarg );
					in_flag = true;
				} else {
					printf( "Input file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'o' :
				if( strlen( optarg ) < 256 ){
					strcpy( out_f_name, optarg );
					out_flag = true;
				} else {
					printf( "Output file name too long.\n" );
					exit( 1 );
				}
				break;
			case 'R' :
				reader_flag = true;
				break;
			case 'f' :
				if( mode != VERY_FASTIDIOUS ) mode = FASTIDIOUS;
				break;
			case 'F' :
				mode = VERY_FASTIDIOUS;
				break;
			case 's' :
				mode = SIMULATION;
				break;
			case 'v' :
				verbose = true;
				break;
			case 'b' :
				default_beam_out = atoi( optarg );
				if( default_beam_out < 1 || default_beam_out > 162 ){
					printf( "Valid indices: 1..162.\n" );
					exit( 1 );
				}
				break;
			default :
				printf( "\"%s\" is not a valid option.\n", optopt );
				printf( "Valid options are: -i <file_name>\n-o <file_name>\n" );
				printf( "-R\n-f\n-F\n-s\n-v\n-b [1..162]\n" );
				exit( 0 );
		}
	}
	
	//consistency check
	if( !track_flag && !in_flag ){
		printf( "Sorry, at the moment I can't read everything from STDIN...\n" );
		exit( 1 );
	}
	
	if( verbose ) printf( "*** Welcome in the doppler corrector ***\n" );
	
	//------------------------------------
	//first of all, read the track data
	std::vector<XB::track_info*> xb_track_book, tb_buf;
	
	if( reader_flag && track_flag ){ //if the file(s) haven't been translated yet
	                                 //use the translator
		if( verbose ) printf( "Reading through xb_data_translator...\n" );
		translate_track_info( xb_track_book, track_f_count, track_f_name, verbose );
	} else if( !reader_flag && track_flag ) { //or loop on them with XB::load
		for( int i=0; i < track_f_count; ++i ){
			if( verbose ) printf( "Reading track from %s...\n", track_f_name[i] );
			XB::load( track_f_name[i], tb_buf ); //load the track data
			
			//cat them at the end of the array
			xb_track_book.insert( xb_track_book.end(), tb_buf.begin(), tb_buf.end() );
			
			//cleanup
			tb_buf.clear();
		}
	} else if( !track_flag ) { //or load from STDIN
		if( verbose ) printf( "Reading track from STDIN...\n" );
		XB::load( stdin, xb_track_book );
	}
	
	//then, load the actual data to correct
	std::vector<XB::data*> xb_book;
	if( in_flag ){
		if( verbose ) printf( "Reading data from %s...\n", in_f_name );
		XB::load( in_f_name, xb_book );
	} else {
		if( verbose ) printf( "Reading data from STDIN...\n" );
		XB::load( stdin, xb_book );
	}
	
	//------------------------------------
	//do the correction
	if( verbose ) printf( "Processing...\n" );
	apply_doppler_correction( xb_book, xb_track_book, default_beam_out, mode, verbose );

	//------------------------------------	
	//now, save
	if( out_flag ){
		if( verbose ) printf( "Writing to %s...\n", out_f_name );
		XB::write( out_f_name, xb_book );
	} else {
		XB::write( stdout, xb_book );
	}
	
	//happy thoughts
	if( verbose ) printf( "*** Done, goodbye. ***\n" );
	return 0;
}

//------------------------------------------------------------------------------------
//implementation of the function that drives the doppler correction
//in parallel.
//NOTE: for now, the fedault beam out is always used as the direction
//      in the near future, the directional information from the tracker
//      will be used when available
//NOTE2: I do apologize to whoever will read this function, it is a bit
//       more complicated than usual -especially because it's a bit of a fest
//       of double-pointers and whatever.
void apply_doppler_correction( std::vector<XB::data*> &xb_book,
                               std::vector<XB::track_info*> &xb_track_book,
                               unsigned int default_beam_out, correct_mode mode,
                               bool verbose ){
	//ok, now do some actual work:
	//prepare the thing by ordering the vectors according to the event number
	std::sort( xb_track_book.begin(), xb_track_book.end(), evnt_id_comparison );
	std::sort( xb_book.begin(), xb_book.end(), evnt_id_comparison );

	//prepare the beta interpolator
	XB::b_interp interp_beta_0( xb_track_book );
	
	//some useful iterators
	XB::track_info **xbtb_begin = &xb_track_book.front();
	XB::track_info **xbtb_end = &xb_track_book.back()+1;
	XB::track_info **track_iter;
	
	//the comparison functional
	is_event_id is_evnt;
	
	//this is a perfect candidate to do in parallel
	#pragma omp parallel shared( xb_book, xb_track_book, xbtb_begin, xbtb_end )\
	 private( track_iter, is_evnt )
	{
	
	//some verbosty cosmetics
	unsigned int thread_num = omp_get_thread_num();
	if( verbose && !thread_num ) printf( "Event: 0000000000" );
	
	switch( mode ){
		case RELAXED : //process them all if we aren't fastidious
		
			//loop on all of them (cleverly and in parallel)
			#pragma omp for schedule( static, 10 ) 
			for( int i=0; i < xb_book.size(); ++i ){
				
				if( verbose && !thread_num ) printf( "\b\b\b\b\b\b\b\b\b\b%010d", i );
				
				//if the event is in the track bunch (and we didn't reach
				//the end of that vector) use the track data to correct
				if( std::binary_search( xbtb_begin, xbtb_end, xb_book.at(i), evnt_id_comparison ) ){
					
					//find the track info
					is_evnt = *xb_book.at(i); //set up the functional
					track_iter = std::find_if( xbtb_begin, xbtb_end, is_evnt ); //find it: we
					                                                            //know it's here
					//and correct it
					XB::doppler_correct( *xb_book.at(i),
					                     (*track_iter)->beta_0,
					                     default_beam_out );
				
				//else, use the default beam out direction (given as a crystal)
				//and interpolate the beta
				} else XB::doppler_correct( *xb_book.at(i),
				                            interp_beta_0( xb_book.at(i)->in_beta ),
				                            default_beam_out );
			}
			break;
		case FASTIDIOUS : //just those in the track book
		
			#pragma omp for schedule( static, 10 )
			for( int i=0; i < xb_book.size(); ++i ){
				
				if( verbose && !thread_num ) printf( "\b\b\b\b\b\b\b\b\b\b%010d", i );
				
				//if the event is in the track bunch (and we didn't reach
				//the end of that vector) use the track data to correct
				if( std::binary_search( xbtb_begin, xbtb_end, xb_book.at(i), evnt_id_comparison ) ){
				    					
					//find the track info
					is_evnt = *xb_book.at(i); //set up the functional
					track_iter = std::find_if( xbtb_begin, xbtb_end, is_evnt ); //find it: we
					                                                            //know it's here
					//and correct it
					XB::doppler_correct( *xb_book.at(i),
					                     (*track_iter)->beta_0,
					                     default_beam_out );
				
				//if the datum doesn't have tracking information
				//mark it for deletion
				} else xb_book.at(i)->evnt = 0;
			}
			break;
		case VERY_FASTIDIOUS : //just those in the track book AND with a single fragment
			#pragma omp for schedule( static, 10 )
			for( int i=0; i < xb_book.size(); ++i ){
			
				if( verbose && !thread_num ) printf( "\b\b\b\b\b\b\b\b\b\b%010d", i );
				
				//if the event is in the track bunch (and we didn't reach
				//the end of that vector) use the track data to correct
				if( std::binary_search( xbtb_begin, xbtb_end, xb_book.at(i), evnt_id_comparison ) ){
									
					//find the track info
					is_evnt = *xb_book.at(i); //set up the functional
					track_iter = std::find_if( xbtb_begin, xbtb_end, is_evnt ); //find it: we
					                                                            //know it's here
					//since we are very fastidious, check the number fo fragments
					//if it's more than one, makr for deletion and continue
					if( (*track_iter)->n != 1 ){
						xb_book.at(i)->evnt = 0;
						continue;
					}
					
					//and correct it
					XB::doppler_correct( *xb_book.at(i),
					                     (*track_iter)->beta_0,
					                     (*track_iter)->outgoing[0] );
				//else mark it for deletion
				} else xb_book.at(i)->evnt = 0;
			}
			break;
		case SIMULATION : //process them all if we aren't fastidious
			//init a random sequence
			srand( time( NULL ) );
			
			//loop on all of them (cleverly and in parallel)
			#pragma omp for schedule( dynamic ) 
			for( int i=0; i < xb_book.size(); ++i ){
				
				if( verbose && !thread_num ) printf( "\b\b\b\b\b\b\b\b\b\b%010d", i );
				
				//The simulation has NO in_beta information available, so we need it
				//from the track info of the run we are tailoring the simulation to.
				track_iter = &xb_track_book.at( rand()%xb_track_book.size() ); //set up the functional
				
				//and correct it
				//added some error handling (for now, only for simulations
				//but it will be extended).
				try{
					XB::doppler_correct( *xb_book.at(i),
					                     (*track_iter)->beta_0,
					                     default_beam_out );
				} catch( XB::error e ){
					//TODO: figure out how to get rid of the corrupted events
					continue;
				}
			}
			break;
	}
	
	if( verbose && !thread_num ) printf( "\n" );
	
	} //parallel pragma ends here
	
	//if we were fastidious, clean up the events. In a clever way.
	int n_zeroed = 0;
	if( mode != RELAXED && mode != SIMULATION ){
		if( verbose ) printf( "Pruning data...\n" );
	
		//first: sort them again, all the 0-ed events will bubble up at the beginning.
		std::sort( xb_book.begin(), xb_book.end(), evnt_id_comparison );
		
		//then, find the first non-zero event. To do so, count the 0-ed elements,
		//which are now at the beginning of the vector.
		is_evnt = 0; //get a functional for event id 0
		n_zeroed = std::count_if( xb_book.begin(), xb_book.end(), is_evnt );
		
		//finally, chop off those elements.
		xb_book.erase( xb_book.begin(), xb_book.begin()+n_zeroed );
	}
		
	
}

//------------------------------------------------------------------------------------
//implementation of the (piped) interface to xb_data_translator
void translate_track_info( std::vector<XB::track_info*> &xb_track_book,
                           unsigned int track_f_count, char track_f_name[][256],
                           bool verbose ){

	//prepare the command
	char command[64*256 + 512]; //potentially, this is huge

	//build the command:
	strcpy( command, "./xb_data_translator " );

	//cat the filenames
	if( verbose ) printf( "Translator will read from file(s):\n" );
	for( int f=0; f < track_f_count; ++f ){
		if( verbose ) printf( "\t%s\n", track_f_name[f] );
		strcat( command, track_f_name[f] );
		strcat( command, " " );
	}

	//add the option to get tracking data
	strcat( command, "-t" );
	
	if( verbose ) printf( "Calling:\n\n%s\n\n", command );
	
	//open the read pipe
	FILE *p_xb_data_translator = popen( command, "r" );

	//receive the data
	XB::load( p_xb_data_translator, xb_track_book );

	//close the pipe
	pclose( p_xb_data_translator );
}

//------------------------------------------------------------------------------------
//the comparison utility
bool evnt_id_comparison( const XB::event_holder *one, const XB::event_holder *two ){
	return one->evnt < two->evnt;
}
