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

/*
How does the TPat works?
The Tpat is a bitfield that shouldn't be used as such.
Each of the 16 bits is a "raw" triggers, and they are as follows
0 -- POS & ~ROLU : what's called "Goo Beam" or minimum bias, but we'll reserve minb for the higher level
1 -- POS : The position detecto fires, but we know nothing about the ROLU
2 -- Land multiplicity :
3 -- Land cosmic : we think a cosmic went into LAND

4 -- Fragment wall :
5 -- Fragment wall delayed :
6 -- Proton wall :
7 -- Proton wall delayed :

8 -- CB OR : the crystal ball fired (either of the halves)
9 -- CB OR delayed : as above, later
A -- CB sum :
B -- CB sum delayed :

C -- S8 :
D -- pix :
E -- NTF :
F -- CB left & right :
The bit position is given in hex.
The reaction trigger is a number given by an | on the various reaction triggers
And it should be checked exactly -- numerically, with the == operator.
*/
#define POS_NOT_ROLU 0x0001
#define POS          0x0002
#define LAND_MULT    0x0004
#define LAND_COSM    0x0008
#define FRWALL       0x0010
#define FRWALL_DEL   0x0020
#define PWALL        0x0040
#define PWALL_DEL    0x0080
#define CB_OR        0x0100
#define CB_OR_DEL    0x0200
#define CB_SUM       0x0400
#define CB_SUM_DEL   0x0800
#define S8           0x1000
#define PIX          0x2000
#define NTF          0x4000
#define CB_STEREO    0x8000
/*
The fifteen different tpat conditions are:
---Onspill
Good beam :      POS_NOT_ROLU                       (also known as minimum bias)
Fragment :       POS_NOT_ROLU | FRWALL
FRS S8 :         S8
CB sum :         POS_NOT_ROLU | FRWALL | CB_SUM
Proton :         POS_NOT_ROLU | FRWALL | PWALL
GB - pileup :    POS_NOT_ROLU                       (that's iffy, don't use)
Pix :            POS_NOT_ROLU | PIX
Neutron :        POS_NOT_ROLU | LAND_MULT | FRWALL
---Offspill
CB muon :        CB_SUM_DEL
Land cosm :      LAND_COSM
TFW delayes :    FRWALL_DEL
CB gamma :       CB_OR_DEL
DFT cosm :       PWALL_DEL
NTF :            NTF
CB stereo muon : CB_STEREO
*/

namespace XB{

	//----------------------------------------------------------------------------
	//An utility that translates a string into a tpat mask
	int str2tpat( const char *tpat_str );

	//----------------------------------------------------------------------------
	//a functional to select data and track info
	class select_tpat : public std::unary_function < _xb_event_structure&, bool > {
		public:
			select_tpat(): _mask( 0 ) {};
			select_tpat( int mask, char sgn = 0 ): _mask( mask ), _sgn( sgn ) {};
			select_tpat( const select_tpat &given ): _mask( given._mask ), _sgn( given._sgn ) {};
			select_tpat &operator=( const select_tpat &right ){
				_mask = right._mask; _sgn = right._sgn; return *this; };
			
			//since this goes into a std::remove_if, return false on true.
			bool operator()( _xb_event_structure &datum ){
				bool val = ( datum.tpat && //prune zeros
				             (datum.tpat & _mask) == _mask );
				return _sgn ? val : !val;
			};
		private:
			int _mask;
			char _sgn;
	};
	
	//----------------------------------------------------------------------------
	//actually get rid of the data that doesn't match the mask
	template< class T >
	int select_on_tpat( int mask, std::vector<T> &xb_book ){
		typename std::vector<T>::iterator last;
		select_tpat sel( mask & 0x0000ffff ), selnot( (mask >> 16) & 0x0000ffff, 1 );
		int sz = xb_book.size();
		
		if( mask & 0x0000ffff ){
			last = std::remove_if( xb_book.begin(), xb_book.end(), sel );
			xb_book.erase( last, xb_book.end() );
		}
		if( mask & 0xffff0000 ){
			last = std::remove_if( xb_book.begin(), xb_book.end(), selnot );
			xb_book.erase( last, xb_book.end() );
		}
		
		return sz - xb_book.size();
	}	
}

#endif
