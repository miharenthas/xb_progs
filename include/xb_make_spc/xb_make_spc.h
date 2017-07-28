//data structures and classes to be used EXCLUSIVELY in the
//xb_make_spc program

#ifndef XB_MAKE_SPC__H
#define XB_MAKE_SPC__H

#include <gsl/gsl_histogram.h>
extern "C"{
	#include "gnuplot_i.h"
}

#include "xb_draw_gsl_histogram.h"
#include "xb_cluster.h"
#include "xb_io.h"

//------------------------------------------------------------------------------------
//drone info structure
typedef struct _drone_settings{
	char instream[256];
	char outstream[256];
	char in_pof;
	char out_pof;
	FILE *in;
	FILE *out;
} d_opts;

//------------------------------------------------------------------------------------
//a switcher structure
typedef enum more_or_less{
	MORE,
	LESS
} moreorless;

//------------------------------------------------------------------------------------
//the program's setting container
typedef struct _program_settings{
	//input variables
	char in_fname[64][256]; //input file names
	char out_fname[256]; //output file name
	int drone_flag; //if true, the program works in drone mode.
	int in_flag; //if true, read from file (2 for pipe).
	int out_flag; //if true, save to file (2 for pipe).
	int draw_flag; //if true, draw (also to file).
	int verbose; //if true, print verbose output.
	int interactive; //if true, display prompt.
	unsigned int in_f_count; //number of input files.
	unsigned int num_bins; //number of bins with which to create the histogram
	unsigned int target_mul; //target multiplicity -- 0 indicates "sum all"
	unsigned int target_ctr; //target centroid -- 0 indicates "all"
	unsigned int target_cty; //target nb of crystals -- 0 indicates "all"
	unsigned int target_alt; //target altitude -- 180? indicates "all"
	unsigned int target_azi; //target azimuth -- 180? indicates "all"
	unsigned int target_nrg; //target energy
	moreorless mol_mul; //target multiplicity this/all others
	moreorless mol_cry; //target crystals direction
	moreorless mol_ctr; //target centroid this/all others
	moreorless mol_alt; //target altitude direction
	moreorless mol_azi; //target azimut direction
	moreorless mol_nrg; //terget energy direction
	float range[2]; //the range of the histogram
	XB::gp_options gp_opt; //gnuplot options
	XB::histogram_mode histo_mode; //the histogram population mode.
	d_opts drone; //the drone stuff.
} p_opts;

//------------------------------------------------------------------------------------
//the program's class
class xb_make_spc{
	public:
		//ctors, dtor
		xb_make_spc( p_opts &settings ); //custom constructor
		~xb_make_spc(); //death maker
		
		//methods
		void hack_data(); //apply the cut to the data.
		void reload_data(); //re-read the files, without the cuts!
		
		void populate_histogram(); //populate the histogram
		void draw_histogram(); //draw the histogram (with gnuplot)
		
		void save_histogram(); //save the histogram to file. TODO
		void save_data(); //save the data onto a file.
		
		void put_histogram(); //output the histogram on the set stream (drone.out) TODO
		void put_data(); //output the data onto the set stream (drone.out) TODO
		
		void reset( p_opts &settings ); //update the settings
	private:
		xb_make_spc(); //default constructor. This class cannot be instantiated without valid settings.
		
		void load_files(); //file loader
		void select( XB::selsel selector_type, moreorless m ); //select inside the cluster structure
		void target_multiplicity( moreorless m ); //set the target_multiplicity
		
		int _do_files, _do_hists, _do_data;//useful switches
		std::vector<XB::clusterZ> event_klZ[64]; //the clusters
		gnuplot_ctrl *gp_h; //the handle to the gnuplot session
		gsl_histogram *histo[64]; //the histograms
		p_opts settings; //the settings
};

#endif
