//this header contains the routine to draw clusters.
//it is separated from the actual clustering algorithm for convenience.

#ifndef XB_DRAW_CLUSTER_BALL__H
#define XB_DRAW_CLUSTER_BALL__H

#include <algorithm>

extern "C"{
	#include "gnuplot_i.h"
}

#include "xb_cluster.h"
#include "xb_ball.h"

namespace XB{

	//a data structure handy in the drawing routine
	typedef struct xb_clustered_crystal_flag_with_index{
		bool K_ed; //is clustered?
		unsigned int K_ind; //in which cluster
		unsigned int C_ind; //the index of the crystal
	} K_cry;

	//an operator to order the above given structure
	bool operator<( const K_cry &one, const K_cry &two );

	//the function itself
	//clusters is a populated clusterZ container
	//(see xb_cluster.h for more details on it)
	gnuplot_ctrl *draw_cluster_ball( clusterZ &clusters );

}

#endif
