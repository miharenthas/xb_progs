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
			if( strstr( curr, "minb" ) ) mbuf |= POS_NOT_ROLU;
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
}
		
			
