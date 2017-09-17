//implementation of the tpat handlers

#include "xb_tpat.h"

namespace XB{
	//----------------------------------------------------------------------------
	//convert the blood string
	int str2tpat( const char *tpat_str ){
		char *full = (char*)malloc( strlen( tpat_str ) );
		strcpy( full, tpat_str );
		char *curr = strtok( full, ":" );
		int mask = 0, mbuf = 0;
		while( curr ){
			mbuf = 0;
			if( strstr( curr, "all" ) ) mbuf = 0xffff;
			else if( strstr( curr, "minb" ) ) mbuf |= S_MINBIAS;
			else if( strstr( curr, "frag" ) ) mbuf |= S_FRAGMENT;
			else if( strstr( curr, "frs" ) ) mbuf |= S_FRSS8;
			else if( strstr( curr, "cbsum" ) ) mbuf |= S_CBSUM;
			else if( strstr( curr, "prt" ) ) mbuf |= S_PROTON;
			else if( strstr( curr, "gbpup" ) ) mbuf |= S_GBPILEUP;
			else if( strstr( curr, "pix" ) ) mbuf |= S_PIX;
			else if( strstr( curr, "ntr" ) ) mbuf |= S_NEUTRON;
			else if( strstr( curr, "cbmu" ) ) mbuf |= C_CBMUON;
			else if( strstr( curr, "landc" ) ) mbuf |= C_LANDCOSM;
			else if( strstr( curr, "tfwc" ) ) mbuf |= C_TFWCOSM;
			else if( strstr( curr, "cbgam" ) ) mbuf |= C_CBGAMMA;
			else if( strstr( curr, "dftc" ) ) mbuf |= C_DFTCOSM;
			else if( strstr( curr, "ntfc" ) ) mbuf |= C_NTFCOSM;
			else if( strstr( curr, "cblrmu" ) ) mbuf |= C_CBLRMUON;
			else continue;
			
			if( strstr( curr, "^" ) )mbuf = ~mbuf;
			mask |= mbuf;
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
		
			
