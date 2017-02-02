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
		bool operator()( XB::event_holder &given ){ return given.evnt == evnt; };
		
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

//the wrapper function that applies the doppler correction
void apply_doppler_correction( std::vector<XB::clusterZ> &xb_book,
                               std::vector<XB::track_info*> &xb_track_book,
                               unsigned int default_beam_out, correct_mode mode,
                               bool verbose );
//an utility collection for sorting
bool evnt_id_comparison( const XB::event_holder *one, const XB::event_holder *two );
bool evnt_id_comparison_ref( const XB::event_holder &one, const XB::event_holder &two );
#endif
