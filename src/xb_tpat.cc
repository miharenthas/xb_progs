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
			else if( strstr( curr, "minb" ) ) mbuf |= POS_NOT_ROLU;
			else if( strstr( curr, "frag" ) ) mbuf |= POS_NOT_ROLU | FRWALL;
			else if( strstr( curr, "frs" ) ) mbuf |= S8;
			else if( strstr( curr, "cbsum" ) ) mbuf |= POS_NOT_ROLU | FRWALL | CB_SUM;
			else if( strstr( curr, "prt" ) ) mbuf |= POS_NOT_ROLU | FRWALL | PWALL;
			else if( strstr( curr, "cbpup" ) ) mbuf |= POS_NOT_ROLU;
			else if( strstr( curr, "pix" ) ) mbuf |= POS_NOT_ROLU | PIX;
			else if( strstr( curr, "ntr" ) ) mbuf |= POS_NOT_ROLU | LAND_MULT | FRWALL;
			else if( strstr( curr, "cbmu" ) ) mbuf |= CB_SUM_DEL;
			else if( strstr( curr, "landc" ) ) mbuf |= LAND_COSM;
			else if( strstr( curr, "tfwc" ) ) mbuf |= FRWALL_DEL;
			else if( strstr( curr, "cbgam" ) ) mbuf |= CB_OR_DEL;
			else if( strstr( curr, "dftc" ) ) mbuf |= PWALL_DEL;
			else if( strstr( curr, "ntfc" ) ) mbuf |= NTF;
			else if( strstr( curr, "cblrmu" ) ) mbuf |= CB_STEREO;
			//raw triggersf from here.
			//NOTE: some are synonims with the Tpat names
			//      I don't care.
			else if( strstr( curr, "Rpnr" ) ) mbuf |= POS_NOT_ROLU;
			else if( strstr( curr, "Rpos" ) ) mbuf |= POS;
			else if( strstr( curr, "Rlm" ) ) mbuf |= LAND_MULT;
			else if( strstr( curr, "Rlc" ) ) mbuf |= LAND_COSM;
			else if( strstr( curr, "Rfrw" ) ) mbuf |= FRWALL;
			else if( strstr( curr, "Rfrw-d" ) ) mbuf |= FRWALL_DEL;
			else if( strstr( curr, "Rpw" ) ) mbuf |= PWALL;
			else if( strstr( curr, "Rpw-d" ) ) mbuf |= PWALL_DEL;
			else if( strstr( curr, "Rcbo" ) ) mbuf |= CB_OR;
			else if( strstr( curr, "Rcbo-d" ) ) mbuf |= CB_OR_DEL;
			else if( strstr( curr, "Rcbs" ) ) mbuf |= CB_SUM;
			else if( strstr( curr, "Rcbs-d" ) ) mbuf |= CB_SUM_DEL;
			else if( strstr( curr, "Rs8" ) ) mbuf |= S8;
			else if( strstr( curr, "Rpix" ) ) mbuf |= PIX;
			else if( strstr( curr, "Rntf" ) ) mbuf |= NTF;
			else if( strstr( curr, "Rcbstereo" ) ) mbuf |= CB_STEREO;
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
		
			
