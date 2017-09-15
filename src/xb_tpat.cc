//implementation of the tpat handlers

#include "xb_tpat.h"

namespace XB{
	//----------------------------------------------------------------------------
	//convert the blood string
	int str2tpat( const char *tpat_str ){
		char *full = (char*)malloc( strlen( tpat_str ) );
		strcpy( full, tpat_str );
		char *curr = strtok( full, ":" );
		int mask = 0;
		while( curr ){
			if( !strcmp( curr, "all" ) ){ mask = 0xFFFF; break; }
			else if( !strcmp( curr, "spill" ) ) mask |= ~S_UNSET;
			else if( !strcmp( curr, "minb" ) ) mask |= S_MINBIAS;
			else if( !strcmp( curr, "frag" ) ) mask |= S_FRAGMENT;
			else if( !strcmp( curr, "frs" ) ) mask |= S_FRSS8;
			else if( !strcmp( curr, "cbsum" ) ) mask |= S_CBSUM;
			else if( !strcmp( curr, "prt" ) ) mask |= S_PROTON;
			else if( !strcmp( curr, "gbpup" ) ) mask |= S_GBPILEUP;
			else if( !strcmp( curr, "pix" ) ) mask |= S_PIX;
			else if( !strcmp( curr, "ntr" ) ) mask |= S_NEUTRON;
			else if( !strcmp( curr, "offsp" ) ) mask |= ~C_UNSET;
			else if( !strcmp( curr, "cbmu" ) ) mask |= C_CBMUON;
			else if( !strcmp( curr, "landc" ) ) mask |= C_LANDCOSM;
			else if( !strcmp( curr, "tfwc" ) ) mask |= C_TFWCOSM;
			else if( !strcmp( curr, "cbgam" ) ) mask |= C_CBGAMMA;
			else if( !strcmp( curr, "dftc" ) ) mask |= C_DFTCOSM;
			else if( !strcmp( curr, "ntfc" ) ) mask |= C_NTFCOSM;
			else if( !strcmp( curr, "cblrmu" ) ) mask |= C_CBLRMUON;
			else continue;
			curr = strtok( NULL, ":" );
		}
		
		free( full );
		return mask;
	}
	
	//----------------------------------------------------------------------------
	//actually get rid of the data that doesn't match the mask
	int select_on_tpat( int mask, std::vector<data> &xb_book ){
		std::vector<data>::iterator last;
		select_tpat sel( mask );
		int sz = xb_book.size();
		
		last = std::remove_if( xb_book.begin(), xb_book.end(), sel );
		xb_book.erase( last, xb_book.end() );
		
		return sz - xb_book.size();
	}
	
	int select_on_tpat( int mask, std::vector<track_info> &xb_book ){
		std::vector<track_info>::iterator last;
		select_tpat sel( mask );
		int sz = xb_book.size();
		
		last = std::remove_if( xb_book.begin(), xb_book.end(), sel );
		xb_book.erase( last, xb_book.end() );
		
		return sz - xb_book.size();
	}
}
		
			
