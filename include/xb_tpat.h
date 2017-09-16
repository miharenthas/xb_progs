//this is the tpat handling toolkit for the library
//it's simple: assists in the creation of the mask
//and applies is to the data or track.

#ifndef XB_TPAT__H
#define XB_TPAT__H

#include <string.h>
#include <vector>
#include <functional>
#include <algorithm>

#include "xb_data.h"

//define collection of all the trigger conditions
//as found in the land02 website
//On spill condition (beam is on)
//On true, the trigger contition is matched
#define S_UNSET		0x0000 //meta contition: on-spill unset;	spill
#define S_MINBIAS	0x0001 //minimum bias (is there beam);		minb
#define S_FRAGMENT	0x0011 //is there a fragment?			frag
#define S_FRSS8		0x1000 //FRS S8 ok				frs
#define S_CBSUM		0x0411 //CB reports both halves			cbsum
#define S_PROTON	0x0051 //Is it a proton?			prt
#define S_GBPILEUP	0x0001 //Good beam -- pileup flag (DODGY)	gbpup
#define S_PIX		0x2001 //Silicon detector reporting		pix
#define S_NEUTRON	0x0015 //Do we have a neutron in LAND?		ntr

//off spill conditions
#define C_UNSET		0x0001 //meta condition: off-spill unset	offsp
#define C_CBMUON	0x0800 //We have a muon in the CB		cbmu
#define C_LANDCOSM	0x0008 //We have a cosmic in LAND		landc
#define C_TFWCOSM	0x0020 //We have a cosmic in the ToF wall	twfc
#define C_CBGAMMA	0x0200 //We have a gamma in CB			cbgam
#define C_DFTCOSM	0x0080 //We have a cosmic in DTF		dftc
#define	C_NTFCOSM	0x4000 //We have a cosmic in the New ToF	ntfc
#define C_CBLRMUON	0x8000 //We have a stereo muon in CB		cblrmu

namespace XB{

	//----------------------------------------------------------------------------
	//An utility that translates a string into a tpat mask
	int str2tpat( const char *tpat_str );

	//----------------------------------------------------------------------------
	//a functional to select data and track info
	class select_tpat : public std::unary_function < _xb_event_structure&, bool > {
		public:
			select_tpat(): _mask( 0 ) {};
			select_tpat( int mask ): _mask( mask ) {};
			select_tpat( const select_tpat &given ): _mask( given._mask ) {};
			select_tpat &operator=( const select_tpat &right ){
				_mask = right._mask; return *this; };
			select_tpat &operator=( int right ){ _mask = right; return *this; };
			
			//since this goes into a std::remove_if, return false on true.
			bool operator()( _xb_event_structure &datum ){
				return !( datum.tpat & _mask );
			};
		private:
			int _mask;
	};
	
	//----------------------------------------------------------------------------
	//A function to run the selection.
	int select_on_tpat( int mask, std::vector<XB::data> &xb_book );
	int select_on_tpat( int mask, std::vector<XB::track_info> &xb_book );
}

#endif
