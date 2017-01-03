//this is a port of the function used by the main class
//of the program "xb_make_spc", to be used (also) elsewhere.
#include "xb_draw_gsl_histogram.h"

//------------------------------------------------------------------------------------
//implementation of the drawing function
gnuplot_ctrl *XB::draw_gsl_histogram( gsl_histogram **histo,
                                      XB::gp_options &gp_opt,
                                      unsigned int howmany,
                                      XB::histogram_mode histo_mode ){
	gnuplot_ctrl *gp_h = gnuplot_init(); //init the gnuplot session
	
	//declare the colors
	char *colors[8]; //8 colors should be enough?
					 //at max capacity, each will appear 8 times...
	colors[0] = "\"red\"";
	colors[1] = "\"blue\"";
	colors[2] = "\"green\"";
	colors[3] = "\"pink\"";
	colors[4] = "\"cyan\"";
	colors[5] = "\"violet\"";
	colors[6] = "\"gold\"";
	colors[7] = "\"fuchsia\"";
	
	//set the terminal
	char gp_command[374];
	switch( gp_opt.term ){
		case QT:
			gnuplot_cmd( gp_h, "set terminal qt" );
			gnuplot_cmd( gp_h, "set output \"STDOUT\"" );
			break;
		case PNG:
			strcpy( gp_command, "set output \"" );
			strcat( gp_command, gp_opt.out_fname );
			strcat( gp_command, "\"" );
			gnuplot_cmd( gp_h, "set terminal png" );
			gnuplot_cmd( gp_h, gp_command );
			break;
		default :
			gnuplot_cmd( gp_h, "set terminal qt" );
			gnuplot_cmd( gp_h, "set output \"STDOUT\"" );
			break;
	}
	
	//the KeV per bin and the number of binZ
	unsigned int binZ = (unsigned int)gsl_histogram_bins( histo[0] );
	float kev_bin = ( gsl_histogram_max( histo[0] ) - gsl_histogram_min( histo[0] ) )/
	                (float)binZ;
	
	//some cosmetic commands
	gnuplot_cmd( gp_h, "set title \"%s\"", gp_opt.title );
	gnuplot_set_xlabel( gp_h, gp_opt.x_label );
	gnuplot_cmd( gp_h, "set ylabel \"Counts/%f%s\"", kev_bin, gp_opt.x_label );
	gnuplot_cmd( gp_h, "set xrange [%f:%f]",
	             gsl_histogram_min( histo[0] ),
	             gsl_histogram_max( histo[0] ) );
	if( gp_opt.is_log ) gnuplot_cmd( gp_h, "set logscale y" );

	//the actual plot
	//build the command
	char plot_cmd[2048];
	strcpy( plot_cmd, "plot " );
	int color=0;
	for( color=0; color < ( histo_mode == COMPARE ? howmany-1 : 0 ); ++color ){
		strcat( plot_cmd, "'-' with histeps title \"%d\" lc rgb " );
		sprintf( plot_cmd, plot_cmd, color+1 );
		strcat( plot_cmd, colors[color%8] );
		strcat( plot_cmd, ", " );
	}
	strcat( plot_cmd, "'-' with histeps title \"%u\" lc rgb " );
	sprintf( plot_cmd, plot_cmd, howmany );
	strcat( plot_cmd, colors[color%8] );
	
	gnuplot_cmd( gp_h, plot_cmd );
	
	//fill in the data
	double b_min, b_max; //max and min of the current bin
	//loop on the data
	for( int i=0; i < ( histo_mode == COMPARE ? howmany : 1 ); ++i ){
		for( int b=0; b < binZ; ++b ){
			gsl_histogram_get_range( histo[i], b, &b_min, &b_max ); //get the range of the current bin
			gnuplot_cmd( gp_h, "%f %f", (b_max + b_min)/2, 
				         gsl_histogram_get( histo[i], b ) ); //cat the data into gnuplot
		}
		gnuplot_cmd( gp_h, "e\n" ); //end the data series.
	}
	
	return gp_h;
}
