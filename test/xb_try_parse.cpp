//this try ptogram tests the config file parser

#include <time.h>
#include <stdlib.h>
#include <vector>

#include "xb_parse_cnf_file.h"
#include "xb_cut.h"
#include "xb_draw_cut.h"
#include "xb_data.h"

#define N_OF_POINTS 10000

//------------------------------------------------------------------------------------
//implementation of the point extractor
class ext : public XB::cut_data_2D< XB::event_holder > {
	public:
		ext( std::vector< XB::event_holder* > &data_array ):
			XB::cut_data_2D< XB::event_holder >( data_array ) {};
	
		XB::pt_holder operator()( unsigned int i ){
			double a = _data_array[i]->n;
			double b = _data_array[i]->evnt;
			return XB::pt_holder( a, b );
	}
};


//------------------------------------------------------------------------------------
//da mainZ
int main( int argc, char **argv ){
	//parse the options directly
	XB::cut_cnf *the_cut = XB::config_alloc( 1 );
	XB::parse_config( &argv[1], the_cut );
	
	//first: create the test set
	std::vector< XB::event_holder* > some_data;
	
	srand( time(NULL) );
	for( int i=0; i < N_OF_POINTS; ++i ){
		some_data.push_back( new XB::event_holder );
		some_data[i]->n = rand()%1000;
		some_data[i]->evnt = rand()%1500;
	}
	
	ext point_extractor( some_data );
	
	//apply the cut
	std::vector<bool> flags( N_OF_POINTS );
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = the_cut[0].cut_2D->contains( (double*)point_extractor( i ) );
	}
	
	//draw the cut and see what happens
	gnuplot_ctrl *gp_h = XB::draw_cut< XB::event_holder >( the_cut[0].cut_2D, point_extractor, flags, "try", "this" );
	
	char ch = getchar();
	gnuplot_close( gp_h );
	
	XB::config_free( the_cut );
	
	return 0;
}
