//this is a port of the function used by the main class
//of the program "xb_make_spc", to be used (also) elsewhere.
#include <string.h>

#include <gsl/gsl_histogram.h>
extern "C"{
	#include "gnuplot_i.h"
}

namespace XB{
	//------------------------------------------------------------------------------------
	//some options containers
	typedef enum _gnuplot_terminal { //gnuplot terminal setting
		QT = 0,
		PNG
	} gp_terminal;

	typedef enum _histogram_populating_mode { //histogram population mode
		JOIN = 0, //sum all the files
		COMPARE, //compare all the files
		SUBTRACT //subtract all the files exept the first from the first one
	} histogram_mode;

	typedef struct _gnuplot_draw_histogram_options{ //more gnuplot settings
		gp_terminal term;
		bool is_log;
		char title[128];
		char x_label[32];
		char y_label[32];
		char out_fname[256];
	} gp_options;

	//------------------------------------------------------------------------------------
	//the function:
	//-histo: is an (array, optionally) of gsl_histograms
	//-gp_opt: is the gnuplot options
	//-howmay: how many histograms we are dealing with
	//-histo_mode: how to compose them.
	gnuplot_ctrl *draw_gsl_histogram( gsl_histogram **histo,
		                          XB::gp_options &gp_opt,
		                          unsigned int howmany,
		                          XB::histogram_mode histo_mode );
}
