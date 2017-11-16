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
		if( hld.tpat & POS ) gsl_histogram_accumulate( stats, 2, 1 );
		if( hld.tpat & LAND_MULT ) gsl_histogram_accumulate( stats, 3, 1 );
		if( hld.tpat & LAND_COSM ) gsl_histogram_accumulate( stats, 4, 1 );
		if( hld.tpat & FRWALL ) gsl_histogram_accumulate( stats, 5, 1 );
		if( hld.tpat & FRWALL_DEL ) gsl_histogram_accumulate( stats, 6, 1 );
		if( hld.tpat & PWALL ) gsl_histogram_accumulate( stats, 7, 1 );
		if( hld.tpat & PWALL_DEL ) gsl_histogram_accumulate( stats, 8, 1 );
		if( hld.tpat & CB_OR ) gsl_histogram_accumulate( stats, 9, 1 );
		if( hld.tpat & CB_OR_DEL ) gsl_histogram_accumulate( stats, 10, 1 );
		if( hld.tpat & CB_SUM ) gsl_histogram_accumulate( stats, 11, 1 );
		if( hld.tpat & CB_SUM_DEL ) gsl_histogram_accumulate( stats, 12, 1 );
		if( hld.tpat & S8 ) gsl_histogram_accumulate( stats, 13, 1 );
		if( hld.tpat & PIX ) gsl_histogram_accumulate( stats, 14, 1 );
		if( hld.tpat & NTF ) gsl_histogram_accumulate( stats, 15, 1 );
		if( hld.tpat & CB_STEREO ) gsl_histogram_accumulate( stats, 16, 1 );
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
			"POS.........",
			"LAND_MULT...",
			"LAND_COSM...",
			"FRWALL......",
			"FRWALL_DEL..",
			"PWALL.......",
			"PWALL_DEL...",
			"CB_OR.......",
			"CB_OR_DEL...",
			"CB_SUM......",
			"CB_SUM_DEL..",
			"S8..........",
			"PIX.........",
			"NTF.........",
			"CB_STERO...."
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
