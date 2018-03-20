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
			else if( strstr( curr, "frag" ) ) mbuf |= FRAG;
			else if( strstr( curr, "ntr" ) ) mbuf |= LAND;
			else if( strstr( curr, "cbsumf" ) ) mbuf |= FRAG_XB_SUMF;
			else if( strstr( curr, "cbsum" ) ) mbuf |= FRAG_XB_SUM;
			else if( strstr( curr, "cbor" ) ) mbuf |= FRAG_XB_OR;
			else if( strstr( curr, "pix" ) ) mbuf |= PIX;
			else if( strstr( curr, "landc" ) ) mbuf |= LAND_COSM;
			else if( strstr( curr, "tfwc" ) ) mbuf |= TFW_COSM;
			else if( strstr( curr, "ntfc" ) ) mbuf |= NTF_COSM;
			else if( strstr( curr, "cbc" ) ) mbuf |= XB_COSM;
			else if( strstr( curr, "cboffsp" ) ) mbuf |= XB_SUM_OFFSP;
			else if( strstr( curr, "pixoffsp" ) ) mbuf |= PIX_OFFSP;
			else continue;
			
			if( strstr( curr, "^" ) )mbuf = mbuf << 16;
			mask |= mbuf;
			curr = strtok( NULL, ":" );
		}
		
		free( full );
		return mask;
	}
	
	//convert a bunch of hex flags passed at the command line into a tpat mask
	int hex2tpat( const char *tpat_str ){
		char *full = (char*)malloc( strlen( tpat_str ) );
		strcpy( full, tpat_str );
		char *curr = strtok( full, ":" );
		int mask = 0, mbuf = 0;
		while( curr ){
			mbuf = 0;
			if( strstr( curr, "^" ) ){
				sscanf( curr, "^%x", &mbuf );
				mbuf = mbuf << 16;
				mask |= mbuf;
			} else {
				sscanf( curr, "%x", &mbuf );
				mask |= mbuf;
			}
			curr = strtok( NULL, ":" );
		}
		
		free( full );
		return mask;
	}
	
	//----------------------------------------------------------------------------
	//allocate a gsl histogram of the right size
	gsl_histogram *tpat_stats_alloc(){
		gsl_histogram *stats = gsl_histogram_alloc( 17 ); //all the flags plus no flag set
		gsl_histogram_set_ranges_uniform( stats, 0, 16 ); //flags are numbered 1 to 16
		                                                  //NOTE: in the **EXACT** order
		                                                  //      they are listed in the header
		return stats;
	}

	//----------------------------------------------------------------------------
	//populate the histogram
	void tpat_stats_push( gsl_histogram *stats, const event_holder &hld ){
		if( !hld.tpat ){ gsl_histogram_accumulate( stats, 0, 1 ); return; }
		if( hld.tpat & POS_NOT_ROLU ) gsl_histogram_accumulate( stats, 1, 1 );
		if( hld.tpat & PNR_PUP ) gsl_histogram_accumulate( stats, 2, 1 );
		if( hld.tpat & FRAG ) gsl_histogram_accumulate( stats, 3, 1 );
		if( hld.tpat & LAND ) gsl_histogram_accumulate( stats, 4, 1 );
		if( hld.tpat & FRAG_XB_SUMF ) gsl_histogram_accumulate( stats, 5, 1 );
		if( hld.tpat & FRAG_XB_SUM ) gsl_histogram_accumulate( stats, 6, 1 );
		if( hld.tpat & FRAG_XB_OR ) gsl_histogram_accumulate( stats, 7, 1 );
		if( hld.tpat & PIX ) gsl_histogram_accumulate( stats, 8, 1 );
		if( hld.tpat & LAND_COSM ) gsl_histogram_accumulate( stats, 9, 1 );
		if( hld.tpat & TFW_COSM ) gsl_histogram_accumulate( stats, 10, 1 );
		if( hld.tpat & NTF_COSM ) gsl_histogram_accumulate( stats, 11, 1 );
		if( hld.tpat & XB_COSM ) gsl_histogram_accumulate( stats, 12, 1 );
		if( hld.tpat & XB_SUM_OFFSP ) gsl_histogram_accumulate( stats, 13, 1 );
		if( hld.tpat & PIX_OFFSP ) gsl_histogram_accumulate( stats, 14, 1 );
		//look also in the not-in-use bits, since you're at it
		if( hld.tpat & 0x4000 ) gsl_histogram_accumulate( stats, 15, 1 );
		if( hld.tpat & 0x8000 ) gsl_histogram_accumulate( stats, 16, 1 );
	}
	
	//----------------------------------------------------------------------------
	//the big thing: the printing of the stats.
	void tpat_stats_printf( FILE *stream, gsl_histogram *stats ){
		if( stats->n != 17 ) throw error( "Wrong histogram!", "XB::tpat_stats_printf" );
		
		//scale the histogram
		double htop = 0;
		for( int i=0; i < 17; ++i )
			if( stats->bin[i] > htop ) htop = stats->bin[i];
		
		if( !htop ) throw error( "Empty histogram!", "XB::tpat_stats_printf" );
		
		//find out the terminal size
		struct winsize w;
		ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
		const int NB_PLUSSES = ( w.ws_col > 30 )? w.ws_col - 30 : 1;
		
		htop = NB_PLUSSES/htop;
		double scaled_bin[17];
		for( int i=0; i < 17; ++i )
			scaled_bin[i] = round( stats->bin[i]*htop );
		
		//do the printing
		fprintf( stream, "===TPAT stats===\n" );
		char flag_names[17][14] = {
			"UNSET.......",
			"POS_NOT_ROLU",
			"PNR_PUP.....",
			"FRAG........",
			"LAND........",
			"FRAG_XB_SUMF",
			"FRAG_XB_SUM.",
			"FRAG_XB_OR..",
			"PIX.........",
			"LAND_COSM...",
			"TFW_COSM....",
			"NTF_COSM....",
			"XB_COMS.....",
			"XB_SUM_OFFSP",
			"PIX_OFFSP...",
			"0x4000......",
			"0x8000......"
		};
		for( int i=0; i < 17; ++i ){
			fprintf( stream, "%s:", flag_names[i] );
			int count;
			for( count=0; count < scaled_bin[i]; ++count ) fprintf( stream, "+" );
			for( int c=count; c <= NB_PLUSSES; ++c ) fprintf( stream, " " );
			fprintf( stream, " | %9.0lf\n", stats->bin[i] );
		}
		fprintf( stream, "==/TPAT stats===\n" );
	}
}
