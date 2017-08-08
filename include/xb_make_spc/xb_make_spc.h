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
#include "xb_make_spc/selectors.h"

//some useful defines: they will also be used by the command line
//to instruct the program's execution. This because the command line
//and the program have been suboptimally designed and thus can
//interact in weird ways such as this. This is not portable.
//We'll use a 16 bit (unsigned) integer to carry the execution flags
// bbbb bbbb bbbb bbbb
// the 4 most significant bits are the flags to control the command line exectuion:
// 0x8000 -- exit the command line
// 0x4000 -- execute and reenter
// 0x2000 -- return
// (0x1000 -- nothing yet)
// the 4 secont-to-most significant bits are unassigned yet
// the 4 second-to-least significant bits control the data hacking
// 0x0080 -- call hack_data(), apply the cuts
// 0x0040 -- call unload_files()
// 0x0020 -- call load_files()
// 0x0010 -- call populate_histogram()
// the 4 least significant bits control the output operations
// 0x0008 -- call save_histogram()
// 0x0004 -- call save_data()
// 0x0002 -- call put_histogram()
// 0x0001 -- call put_data()
#define DO_EXIT 0x8000
#define DO_EXECUTE 0x4000
#define DO_RETURN 0x2000
#define DO_HACK_DATA 0x0080
#define DO_UNLOAD 0x0040
#define DO_LOAD 0x0020
#define DO_POP_HISTO 0x0010
#define DO_SAVE_HISTO 0x0008
#define DO_SAVE_DATA 0x0004
#define DO_PUT_HISTO 0x0002
#define DO_PUT_DATA 0x0001

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
	MORE = 0,
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
	unsigned int target_cry; //target nb of crystals -- 0 indicates "all"
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
		xb_make_spc(); //default constructor.
		~xb_make_spc(); //death maker
		
		//methods
		void draw_histogram(); //draw the histogram (with gnuplot)
		void reset( p_opts &settings ); //update the settings
		void exec( unsigned short int prog ); //execute the program
	private:
		void load_files(); //file loader
		void unload_files(); //file unloader
		
		void hack_data(); //apply the cut to the data.
		void populate_histogram(); //populate the histogram
		
		void save_histogram(); //save the histogram to file.
		void save_data(); //save the data onto a file.
		
		void put_histogram(); //output the histogram on the set stream (drone.out)
		void put_data(); //output the data onto the set stream (drone.out)
		
		void select( XB::selsel selector_type, moreorless m ); //select inside the cluster structure
		void target_multiplicity( moreorless m ); //select the target_multiplicity
		//and select the rest
		template< class the_selector >
		void target_field( const the_selector &in_kl_selector );
		
		std::vector<XB::clusterZ> event_klZ[64]; //the clusters
		gnuplot_ctrl *gp_h; //the handle to the gnuplot session
		gsl_histogram *histo[64]; //the histograms
		p_opts settings; //the settings
};

#endif
