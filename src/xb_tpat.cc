//implementation of the tpat handlers

#include "xb_tpat.h"

namespace XB{
	//----------------------------------------------------------------------------
	//convert the blood string
	int str2tpat( const char *tpat_str ){
		char *full = (char*)malloc( strlen( tpat_str ) );
		strcpy( full, tpat_str );
		char *curr = strtok( full, ":" );
		int mask = strstr( tpat_str, "^" )? 0 : 0xffff0000, mbuf = 0;
		while( curr ){
			mbuf = 0;
			if( strstr( curr, "all" ) ) mbuf = 0xffff;
			else if( strstr( curr, "minb" ) ) mbuf |= POS_NOT_ROLU;
			else if( strstr( curr, "frag" ) ) mbuf |= POS_NOT_ROLU | FRWALL;
			else if( strstr( curr, "frs" ) ) mbuf |= S8;
			else if( strstr( curr, "cbsum" ) ) mbuf |= POS_NOT_ROLU | FRWALL | CB_SUM;
			else if( strstr( curr, "prt" ) ) mbuf |= POS_NOT_ROLU | FRWALL | PWALL;
			else if( strstr( curr, "gbpup" ) ) mbuf |= POS_NOT_ROLU;
			else if( strstr( curr, "pix" ) ) mbuf |= POS_NOT_ROLU | PIX;
			else if( strstr( curr, "ntr" ) ) mbuf |= POS_NOT_ROLU | LAND_MULT | FRWALL;
			else if( strstr( curr, "cbmu" ) ) mbuf |= CB_SUM_DEL;
			else if( strstr( curr, "landc" ) ) mbuf |= LAND_COSM;
			else if( strstr( curr, "tfwc" ) ) mbuf |= FRWALL_DEL;
			else if( strstr( curr, "cbgam" ) ) mbuf |= CB_OR_DEL;
			else if( strstr( curr, "dftc" ) ) mbuf |= PWALL_DEL;
			else if( strstr( curr, "ntfc" ) ) mbuf |= NTF;
			else if( strstr( curr, "cblrmu" ) ) mbuf |= CB_STEREO;
			//TODO: add the single trigger keywords
			else continue;
			
			if( strstr( curr, "^" ) )mbuf = mbuf << 16;
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
		
			
