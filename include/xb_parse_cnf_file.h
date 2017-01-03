//this header contains the definitions of the functions/classes
//necessary to parse and write a configuration file for the xb_progs toolkit
//for speed of development, it is based upon getopt_long_only, which is a
//colossal hack and may not even be such a nice way to do it.. but what can you do.

#ifndef XB_PARSE_CNF_FILE__H
#define XB_PARSE_CNF_FILE__H

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <gsl/gsl_matrix.h>

#include "xb_cut_typedefs.h"

namespace XB{
	//----------------------------------------------------------------------------
	//a structure containing the configured cut and some info
	typedef struct _xb_cut_configuration {
		bool is_1D;
		bool is_2D;
		cut_primitive_2D cut_type;
		_xb_1D_cut_primitive *cut_1D;
		_xb_2D_cut_primitive *cut_2D;
	} cut_cnf;
	
	//----------------------------------------------------------------------------
	//parsing routine:
	
	/*****************************************************************************
	                               NOTE ON THE FORMAT:
	The available options are:
	--cut-type [TYPE OF THE CUT] : same as in "xb_cut_typedefs.h" without the leading CUT_
	--cut-segment-begin <float> : where the segment begins
	--cut-segment-end <float>, where the segment ends
	--cut-centroid (<float>,<float>) : cooridnates of the centroid
	--cut-rotation <float> : angle of rotation around the centroid, in RADIANS
	--cut-square-side <float> : the side of a square
	--cut-circle-radius <float> : the radius of a circle
	--cut-ellipse-major-saxis <float> : the "major" axis of an ellipse
	  NOTE: as such it is intended the one parallel to the X axis
	--cut-ellipse-minor-saxis <float> : the "mino" axis of an ellipse (see NOTE above)
	--cut-regular-polygon-nb-sides <u integer> : number of sites of a regular polygon
	--cut-regular-polygon-radius <float> : the "radius" of a regula polygon
	--cut-polygon-nb-sides <u integer> : the number of sides of a polygon
	  NOTE: this ***MUST*** be specified before giving the vertices, as the
	        memory is allocated according to this information!
	--cut-polygon-vertex v<u integer>(<float>,<float>) : a vertex for a polygon
	  NOTE: this option must appear ***EXACTLY*** as many times as
	        "--cut-polygon-nb-sides" specifies.
	--cut-transformation [<f>,<f>,<f>,<f>,<f>,<f>,<f>,<f>,<f>] : defines the affine
	                     transformation for the cut.
	*****************************************************************************/
	cut_cnf parse_config_opts( char *the_str ); //parse one config string
	cut_cnf parse_config_opts( unsigned int str_count, char **str_vec ); //parse already split
	//parse many configurations:
	//	char **the_str: an array of "howmany" configuration strings.
	//	struct cut_cnf *cnfs: an **ALREADY ALLOCATED** array of config structs.
	//NOTE: the size information is stored byt config_alloc at the beginning of the buffer.
	//      if you allocate by yourself, everything breaks.
	void parse_config( char **the_str, cut_cnf *configs );
	
	//----------------------------------------------------------------------------
	//allocation/deallocation routines
	cut_cnf *config_alloc( const unsigned int howmany );
	void config_free( cut_cnf *cnfs );
}

#endif
