//an header file that contains some of the support gobbins for xb_doppc
#ifndef XB_DOPPC__H
#define XB_DOPPC__H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

#include <algorithm>
#include <functional>
#include <vector>

#include "xb_doppler_corr.h"
#include "xb_cluster.h"
#include "xb_data.h"
#include "xb_io.h"
#include "xb_error.h"

//flags
#define VERBOSE 0x0001 //if true, verbose output
#define IN_FROM_FILE 0x0002 //if true, read from a file and not stdin
#define TRACK_FROM_FILE 0x0004 //if true, read the track info from file and not stdin
#define CLUSTER_FLAG 0x0008 //if true, correct clusters instead of just events.
#define OUT_TO_FILE 0x0010 //if true, write to a file and not to stdout
#define USE_TRANSLATOR 0x0020 //if true, use xb_data_translator to read the TRACK file
#define NO_TRACK 0x0040 //don't use a track, rely upon the in_beta.

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
		is_event_id( const XB::event_holder &given  ): evnt( given.evnt ) {}; //from XB::event_holder
		is_event_id( const is_event_id &given ): evnt( given.evnt ) {}; //copy
		
		//the main thing
		bool operator()( XB::event_holder *given ){ return given->evnt == evnt; };
		bool operator()( XB::event_holder const &given ){ return given.evnt == evnt; };
		
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
void apply_doppler_correction( std::vector<XB::data> &xb_book,
                               std::vector<XB::track_info> &xb_track_book,
                               unsigned int default_beam_out, correct_mode mode,
                               int flagger );

//the wrapper function that applies the doppler correction
void apply_doppler_correction( std::vector<XB::clusterZ> &xb_book,
                               std::vector<XB::track_info> &xb_track_book,
                               unsigned int default_beam_out, correct_mode mode,
                               int flagger );

//again two overloads, for the "without a track" option
//for the data
void apply_doppler_correction( std::vector<XB::data> &xb_book,
                               unsigned int default_beam_out,
                               int flagger );
//and for the clusterZ
void apply_doppler_correction( std::vector<XB::clusterZ> &xb_book,
                               unsigned int default_beam_out,
                               int flagger );
//an utility collection for sorting
bool evnt_id_comparison( const XB::event_holder &one, const XB::event_holder &two );
#endif
