//this program visualizes the crystal ball
//so far, just the crystal centroids

#include <stdio.h>
#include <unistd.h>

#include "xb_ball.h"
extern "C" {
	#include "gnuplot_i.h"
}

#define PI 3.14159265359

int main( int argc, char **argv ){
	//set up also a range
	int crystal_range[2] = {0, 161};
	
	if( argc == 3 ){
		if( strstr( argv[1], "-" ) ) crystal_range[0] = 0;
		else crystal_range[0] = atoi( argv[1] ) -1;
		if( strstr( argv[2], "-" ) ) crystal_range[1] = 161;
		else crystal_range[1] = atoi( argv[2] ) -1;
	}


	//open the gnuplot pipe
	gnuplot_ctrl *gp_h = gnuplot_init();
	
	//create a ball
	XB::xb_ball the_cb;
	
	//set the scale equal on all axes
	gnuplot_cmd( gp_h, "set view equal xyz" );
	gnuplot_cmd( gp_h, "set xlabel \"X\"" );
	gnuplot_cmd( gp_h, "set ylabel \"Y\"" );
	gnuplot_cmd( gp_h, "set zlabel \"Z\"" );

	//issue the 3d plot command
	gnuplot_cmd( gp_h, "splot '-' using 1:2:3:(sprintf( \"%%d\", $4 ))\
	                    with labels point pt 7 offset char 3" );
	
	//loop on the crystals, converting them to cartesian coords
	float v[3], altitude, azimuth; //the vertex buffer and two angle buffers 
	for( int c=crystal_range[0]; c <= crystal_range[1]; ++c ){
		//get the angles
		altitude = the_cb.ball[c].altitude;
		azimuth = the_cb.ball[c].azimuth;
		
		//convert
		v[1] = cos(altitude)*sin(azimuth);
		v[0] = cos(altitude)*cos(azimuth);
		v[2] = sin(altitude);
		
		//put the point
		gnuplot_cmd( gp_h, "%f %f %f %d", v[0], v[1], v[2], c+1 );
	}
	gnuplot_cmd( gp_h, "e\n" );
	
	//pause here
	getc( stdin );
	
	//close the pipe
	gnuplot_close( gp_h );
	
	return 0;
}
