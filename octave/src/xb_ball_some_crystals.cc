//implementation of the array of idexes to map-array routine

#include "xb_ball_some_crystals.h"
#include "octave/oct.h"

octave_map xb_ball_some_crystals( const unsigned int *indexes, const unsigned int howmany ){
	dim_vector o_dim( howmany, 1 );
	Cell o_field_i( o_dim );
	Cell o_field_sh( o_dim );
	Cell o_field_altitude( o_dim );
	Cell o_field_azimuth( o_dim );
	Cell o_field_neigh( o_dim );
	Cell o_field_opp( o_dim );
	
	Array<int> neigh( dim_vector( 6, 1 ) );
	XB::xb_ball the_ball;
	

	//do the copying
	for( int i=0; i < howmany; ++i ){
		o_field_i(i) = the_ball.at(indexes[i]).i;
		o_field_sh(i) = (int)the_ball.at(indexes[i]).sh;
		o_field_altitude(i) = the_ball.at(indexes[i]).altitude;
		o_field_azimuth(i) = the_ball.at(indexes[i]).azimuth;
		o_field_opp(i) = the_ball.at(indexes[i]).opp->i;
		
		neigh(5) = 0;
		for( int n=0; n < (( the_ball.at(indexes[i]).sh == XB::PENTAGON )? 5 : 6); ++n ){
			neigh(n) = the_ball.at(indexes[i]).neigh[n]->i;
		}
		o_field_neigh(i) = neigh;
	}
	
	//make the map and return it
	octave_map o_map;
	o_map.setfield( "i", o_field_i );
	o_map.setfield( "sh", o_field_sh );
	o_map.setfield( "altitude", o_field_altitude );
	o_map.setfield( "azimuth", o_field_azimuth );
	o_map.setfield( "opp", o_field_opp );
	o_map.setfield( "neigh", o_field_neigh );

	return o_map;
}
