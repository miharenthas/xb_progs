//this header defines the data structure for handling XB data internally (after a pawn_ntuple conversion run).
#ifndef XB_DATA__H
#define XB_DATA__H

#include <math.h>
#include <stdlib.h>

#include "xb_error.h"

namespace XB{
	//----------------------------------------------------------------------------
	//a structure that holds an unitary vector
	typedef struct _xb_versor{
		float i;
		float j;
		float k;
	} versor;
	
	//----------------------------------------------------------------------------
	//a structure from which the actual data structures will inherit
	typedef class _xb_event_structure{
		public:
			unsigned int n; //a multiplicity indicator
			unsigned int evnt; //the event ID container
	} event_holder;		
				
	
	//----------------------------------------------------------------------------
	//a structure that will hold the data from the XB
	typedef class _xb_data_structure : public event_holder {
		//variables
		public: //yes, public, live with it
			unsigned int n; //number of signal delivering crystals
			unsigned int evnt; //event number
			unsigned int* i; //indexes thereof
			float *t; //timestamps
			float *pt; //transverse
			float *e; //energy per crystal
			float *he; //something-energy per crystal
			float sum_e; //sum of all energy deposits
			float in_beta; //incoming beta
			//And here we have the flags (if some fields are empty)
			bool empty_t;
			bool empty_pt;
			bool empty_e;
			bool empty_he;
			bool empty_sum_e;
		private:
			void *buf; //this is a contiguos buffer holding the class' data
			void make_buf(); //allocates the buffer and links the pointers
		public:
			//ctors, dtor
			_xb_data_structure();
			_xb_data_structure( unsigned int n, unsigned int evnt );
			_xb_data_structure( const _xb_data_structure& );
			~_xb_data_structure();
		
			//methods
			void probe_for_crap();
			
			//operators
			_xb_data_structure &operator=( const _xb_data_structure& );
				
	} data;
	
	//----------------------------------------------------------------------------
	//a structure that will hold the tracking data
	typedef class _xb_track_data_structure : public event_holder {
		public:
			unsigned int n; //number of fragments
			unsigned int evnt; //event ID
			float in_beta; //the beta of the incoming beam
			float beta_0; //the beta at the centre of the target
			float in_Z; //the incoming charge
			float in_A_on_Z; //mass to charge ratio
			float *fragment_A; //an array containing the mass numbers of the fragments
			float *fragment_Z; //an array containing the charge of the fragments
			float *fragment_beta; //an array containing the betas of the fragments
			versor *incoming; //a versor describing the incoming direction
			versor *outgoing; //a versor describing the outgoing direction of the fragment
		private:
			void *buf;
			void make_buf();
		public:
			_xb_track_data_structure();
			_xb_track_data_structure( unsigned int n_frag, unsigned int evnt );
			_xb_track_data_structure( const _xb_track_data_structure& );
			~_xb_track_data_structure();
			
			_xb_track_data_structure &operator=( const _xb_track_data_structure& );
	} track_info;
	
	//----------------------------------------------------------------------------
	//comparison operators for XB::versor
	bool operator==( const _xb_versor&, const _xb_versor& );
	bool operator!=( const _xb_versor&, const _xb_versor& );

	//----------------------------------------------------------------------------
	//comparison operators for XB::data
	bool operator==( const _xb_data_structure&, const _xb_data_structure& );
	bool operator!=( const _xb_data_structure&, const _xb_data_structure& );

	//----------------------------------------------------------------------------
	//comparison operators for XB::track_info
	bool operator==( const _xb_track_data_structure&, const _xb_track_data_structure& );
	bool operator!=( const _xb_track_data_structure&, const _xb_track_data_structure& );
}

#endif
