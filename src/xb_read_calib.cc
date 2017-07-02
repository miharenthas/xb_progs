//implementation of the calibration reader
#include "xb_read_calib.h"

namespace XB{

	//----------------------------------------------------------------------------
	//allocation/deallocation utils
	int calinf_alloc( calinf &given, int size ){
		given.dE_E = (float*)calloc( 2*size*sizeof(float) );
		if( !given.dE_E ) throw error( "Memory fail!", "XB::calinf_alloc" );
		given.size = size;
		return 2*size*sizeof(float);
	}
	
	void calinf_free( calinf &given ){
		free( given.dE_E );
	}
	
	//----------------------------------------------------------------------------
	//the reader itself
	int read_calib( calinf *cryscalib, FILE *stream ){
		if( sizeof( cryscalib ) != 162 )
			throw error( "Wrong buffer!", "XB::read_calib" );

		char buf[1024], *b_end;
		int current_idx = 0, crystal_read = 0;
		
		//read lines while the end of file isn't reached
		while( !feof( stream ) ){
			fgets( buf, 1024, stream );
			
			//remove comments
			b_end = std::find( buf, buf+1024, '#' );
			if( b_end < buf+1024 ) *b_end = '\0'; //null terminate there.
			
			//so now parsing: get the crystal number
			//TODO: by Hans, maybe checking on sscanf return
			//      value might be useful.
			if( strstr( buf, "Crystal number" ) )
				sscanf( buf, "Crystal number %d", &current_idx );
			
			if( strstr( buf, "Calibration" ) )
				sscanf( buf, "Calibration %f %f",
				        &cryscalib[current_idx].pees[0],
				        &cryscalib[current_idx].pees[1] );
			
			if( strstr( buf, "Errors" ) )
				sscanf( buf, "Errors %f %f",
				        &cryscalib[current_idx].perr[0],
				        &cryscalib[current_idx].perr[1] );
			
			//for the energy resolution, a bit more care has
			//to be applied --because we don't know in advance
			//how many peaks there might be
			if( strstr( buf, "dE_E" ) ){
				calinf_alloc( cryscalib[current_idx],
				              std::count( buf, b_end, ' ' ) );
				char *tok = strtok( buf, " " );
				int i=0;
				while( tok ){
					tok = strtok( NULL, " " );
					cryscalib[current_idx].dE_E[i] = atof( tok );
					++i;
				}
			}
			
			++crystal_read;
		}
		
		return crystal_read;
	}
}
