//this function collection draws the cut with gnuplot
//because they use a template thing (extractor),
//the are template themselves
//TODO: currently, only one cut can be drawn on the plots
//      although the red flagged areas are not dependent from it
//      adding the possibility of visualizin more than one cut
//      may be desireable in the future.

#ifndef XB_DRAW_CUT__H
#define XB_DRAW_CUT__H

#include <vector>

#include <gsl/gsl_histogram.h>

extern "C"{
	#include "gnuplot_i.h"
}
#include "xb_cut.h"
#include "xb_cut_typedefs.h"
#include "xb_error.h"

#define __XB_MAX_DRAWN_PTS 30000

namespace XB{

	//----------------------------------------------------------------------------
	//function definition:
	
	//for unidimensional cuts:
	//takes in:
	// -the cut that has to be applied
	// -the (extractor for the) cut data
	// -the (extractor for the) original data
	// -the label of the data
	//returns a handle to the gnuplot session
	template< class xb_T >
	gnuplot_ctrl *draw_cut( cut_segment *the_cut,
	                        cut_data_1D<xb_T*> &cut_data,
	                        std::vector<bool> &flags,
	                        char *data_label );
	
	//for 2D cuts
	//takes in:
	// -the cut that has to be applied
	// -the (extractor for the) cut data
	// -the (extractor for the) original data
	// -the labels for the data
	//returns a handle to the gnuplot session
	template< class xb_T >
	gnuplot_ctrl *draw_cut( _xb_2D_cut_primitive *the_cut,
	                        cut_data_2D<xb_T*> &cut_data,
                                std::vector<bool> &flags,
	                        char *data_label_x,
	                        char *data_label_y );
	
	//----------------------------------------------------------------------------
	//utils: they assist the main function in drawing the cut
	//on the present gnuplot session (it's already waiting for it)
	void draw_cut_square( gnuplot_ctrl *gp_h, cut_square& the_cut );
	void draw_cut_circle( gnuplot_ctrl *gp_h, cut_circle& the_cut );
	void draw_cut_ellipse( gnuplot_ctrl *gp_h, cut_ellipse& the_cut );
	void draw_cut_regular_polygon( gnuplot_ctrl *gp_h, cut_regular_polygon& the_cut );
	void draw_cut_polygon( gnuplot_ctrl *gp_h, cut_polygon& the_cut );
	
	//============================================================================
	//function implementation:
	
	//----------------------------------------------------------------------------
	//this function illustrates a 1D cut.
	//because of not being stoopid, let's do an histogram
	template< class xb_T >
	gnuplot_ctrl *draw_cut( cut_segment *the_cut,
	                        cut_data_1D<xb_T*> &cut_data,
	                        std::vector<bool> &flags,
	                        char *data_label )
	{
		//compile an histogram for the true flagged, one for the false flagged
		gsl_histogram *hist_true, *hist_false;
		hist_true = gsl_histogram_alloc( 1000 );
		hist_false = gsl_histogram_alloc( 1000 );
		gsl_histogram_set_ranges_uniform( hist_true, 0., 8000. ); //a well educated
		                                                           //guess
		gsl_histogram_set_ranges_uniform( hist_false, 0., 8000. );
		
		//populate them
		for( int i=0; i < cut_data.size(); ++i ){
			if( flags[i] )
				gsl_histogram_accumulate( hist_true, cut_data( i ), 1. );
			else
				gsl_histogram_accumulate( hist_false, cut_data( i ), 1. );
		}
		
		//get ready to draw:
		//start a gnuplot session
		gnuplot_ctrl *gp_h = gnuplot_init();
		
		//begin drawing:
		//blue what is flagged as FALSE
		//red what is flagged as TRUE
		char gp_cmd[1024], gp_cmd_format[1024];
		strcpy( gp_cmd_format, "plot '-' with histeps title \"TRUE\" lc \"red\", '-' with histeps title \"FALSE\" lc \"blue\", '-' with lines title \"%s\" lc \"violet\" linewidth 5" );
		sprintf( gp_cmd, gp_cmd_format, data_label );
		
		//issue the command
		gnuplot_cmd( gp_h, gp_cmd );
		
		//loop for the TRUE ones
		double bin_centroid = 0.;
		for( int i=0; i < 1000; ++i ){
			//get the bin's centroid
			bin_centroid = hist_true->range[i] + hist_true->range[i+1];
			bin_centroid /= 2;
			
			//issue the counts
			gnuplot_cmd( gp_h, "%f %f", bin_centroid, hist_true->bin[i] );
		}
		gnuplot_cmd( gp_h, "e\n" );
		
		//loop for the FALSE ones
		for( int i=0; i < 1000; ++i ){
			//get the bin's centroid
			bin_centroid = hist_false->range[i] + hist_false->range[i+1];
			bin_centroid /= 2;
			
			//issue the counts
			gnuplot_cmd( gp_h, "%f %f", bin_centroid, hist_false->bin[i] );
		}
		
		//finally, put the cut in there
		gnuplot_cmd( gp_h, "%f %f\n", the_cut->a(), 0. );
		gnuplot_cmd( gp_h, "%f %f\n", the_cut->b(), 0. );
		gnuplot_cmd( gp_h, "e\n" );
		
		return gp_h;
	}
	
	//----------------------------------------------------------------------------
	//this function illustrates the 2D cut with a scatter plot.
	template< class xb_T >
	gnuplot_ctrl *draw_cut( _xb_2D_cut_primitive *the_cut,
	                        cut_data_2D<xb_T> &cut_data,
                                std::vector<bool> &flags,
	                        char *data_label_x,
	                        char *data_label_y )
	{
		//open the gnuplot session
		gnuplot_ctrl *gp_h = gnuplot_init();
		
		//prepare the cut
		switch( the_cut->type() ){
			case CUT_SQUARE:
				draw_cut_square( gp_h, *(cut_square*)the_cut );
				break;
			case CUT_CIRCLE:
				draw_cut_circle( gp_h, *(cut_circle*)the_cut );
				break;
			case CUT_ELLIPSE:
				draw_cut_ellipse( gp_h, *(cut_ellipse*)the_cut );
				break;
			case CUT_REGULAR_POLYGON:
				draw_cut_regular_polygon( gp_h, *(cut_regular_polygon*)the_cut );
				break;
			case CUT_POLYGON:
				draw_cut_polygon( gp_h, *(cut_polygon*)the_cut );
				break;
			default:
				throw error( "No cut type!", "XB::draw_cut" );
				break;
		}
		
		//if the number of points in the data is very large, there may be
		//troubles in handling them all. This calculates a stride for the
		//visualisation. If the size is less than the defined maximum,
		//the stride is set to one.
		unsigned int strider = ceil( cut_data.size()/__XB_MAX_DRAWN_PTS );

		//begin drawing:
		//blue what is flagged as FALSE
		//red what is flagged as TRUE
		char gp_cmd[1024];
		strcpy( gp_cmd, "plot '-' with points pt 7 pointsize 0.1  lc \"red\" title \"TRUE\", '-' with points pt 7 pointsize 0.1 lc \"blue\" title \"FALSE\"" );
		
		//issue the commands
		gnuplot_set_xlabel( gp_h, data_label_x );
		gnuplot_set_ylabel( gp_h, data_label_y );
		gnuplot_cmd( gp_h, gp_cmd );
		
		
		//first, put the data points
		//the red ones:
		for( int i=0; i < cut_data.size(); ++i ){
			if( flags[i] && !(i%strider) )
				gnuplot_cmd( gp_h, "%f %f\n", cut_data( i )[0], cut_data( i )[1] );
		}
		gnuplot_cmd( gp_h, "e\n" );
		
		//then the blue ones:
		for( int i=0; i < cut_data.size(); ++i ){
			if( !flags[i] && !(i%strider) )
				gnuplot_cmd( gp_h, "%f %f\n", cut_data( i )[0], cut_data( i )[1] );
		}
		gnuplot_cmd( gp_h, "e\n" );

		return gp_h;
	}

} //namespace

#endif
