//tiny program to test "sim_reader" function
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vector>

#include <gsl/gsl_histogram.h>

#include "xb_draw_gsl_histogram.h"
#include "xb_data.h"
#include "xb_reader.h"
extern "C" {
	#include "gnuplot_i.h"
}

int main( int argc, char **argv ){
	char in_fname[128];
	if( strlen( argv[1] ) < 128 ) strcpy( in_fname, argv[1] );
	else exit( 1 );
	
	std::vector<XB::data*> data;
	XB::sim_reader( data, in_fname );
	
	printf( "Number of events read: %d\n", data.size() );
	
	gsl_histogram *h = gsl_histogram_alloc( 164 );
	gsl_histogram_set_ranges_uniform( h, 1, 164 );
	
	for( int e=0; e < data.size(); ++e ){
		for( int t=0; t < data[e]->n; ++t ){
			gsl_histogram_accumulate( h, data[e]->i[t], 1. );
		}
	}
	
	XB::gp_options gp_opt = { XB::QT, false, "Try this", "Xtal", "" };
	gnuplot_ctrl *gp_h = XB::draw_gsl_histogram( &h, gp_opt, 1, XB::JOIN );

	/*gnuplot_ctrl *gp_h = gnuplot_init();
	gnuplot_cmd( gp_h, "plot '-' with histeps" );
	for( int i=0; i < 164; ++i ){
		gnuplot_cmd( gp_h, "%d %f", i, gsl_histogram_get( h, i ) );
	}
	gnuplot_cmd( gp_h, "e\n" );*/
	
	char c = getchar();
	gnuplot_close( gp_h );
	
	return 0;
}
	
