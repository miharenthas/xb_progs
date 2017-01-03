//implementation of draw_cluster_ball

#include "xb_draw_cluster_ball.h"

namespace XB{
	//-----------------------------------------------------------------------
	//an operator to order the above given structure
	bool operator<( const K_cry &one, const K_cry &two ){
		return one.K_ind < two.K_ind;
	}

	//-----------------------------------------------------------------------
	//the drawing function
	gnuplot_ctrl *draw_cluster_ball( XB::clusterZ &clusters ){
		//get a ball
		XB::xb_ball the_cb;
		//open the gnuplot handle
		gnuplot_ctrl *gp_h = gnuplot_init();
	
		//declater the colors
		char *colors[10]; //10 colors should be enough?
		colors[0] = "\"red\"";
		colors[1] = "\"blue\"";
		colors[2] = "\"green\"";
		colors[3] = "\"pink\"";
		colors[4] = "\"cyan\"";
		colors[5] = "\"violet\"";
		colors[6] = "\"gold\"";
		colors[7] = "\"fuchsia\"";
		colors[8] = "\"orange\"";
		colors[9] = "\"brown\"";
	
		//compose the command
		const char std_cmd[] = "'-' using 1:2:3:(sprintf( \"%%d\", $4 ) ) with labels offset char 2 point pt 7 lc rgb ";
		const char unk_cmd[] = "'-' using 1:2:3:(sprintf( \"%%d\", $4 ) ) with labels offset char 2 point pt 6 lc rgb ";
		char gp_c[2048];
		strcpy( gp_c, "splot " );
		for( int i=0; i < clusters.multiplicity ; ++i ){ //loop on the clusters
			strcat( gp_c, std_cmd );
			strcat( gp_c, colors[i%10] );
			strcat( gp_c, ", " );
		}
		//add the codon for the unclustered crystals
		strcat( gp_c, unk_cmd );
		strcat( gp_c, "\"black\"" );
	
		//compile a list of the of crystal with the pertinence cluster
		K_cry clustered[162]; //clustered
		for( int c=0; c < 162; ++c ){
			clustered[c].K_ed = false;
			clustered[c].K_ind = clusters.multiplicity+1;
			clustered[c].C_ind = c;
			for( int k=0; k < clusters.multiplicity; ++k ){
				if( binary_search( clusters.clusters[k].crys.begin(),
					  clusters.clusters[k].crys.end(), c+1 ) ){ //c+1 because I need to look for the global
					                                            //index referred to the CB, not the array index!
					clustered[c].K_ed = true;
					clustered[c].K_ind = k;
					break;
				}
			}
		}
	
		//order the list by cluster
		std::sort( clustered, clustered+162 );		
		
		//prepare to draw
		gnuplot_cmd( gp_h, "set view equal xyz" );
		gnuplot_cmd( gp_h, gp_c );
	
		//give in the data
		//loop on the crystals, converting them to cartesian coords
		float v[3], altitude, azimuth; //the vertex buffer and two angle buffers 
		for( int c=0; c < 162; ++c ){
			//get the angles
			altitude = the_cb.ball[clustered[c].C_ind].altitude;
			azimuth = the_cb.ball[clustered[c].C_ind].azimuth;
		
			//convert
			v[0] = cos(altitude)*cos(azimuth);
			v[1] = cos(altitude)*sin(azimuth);
			v[2] = sin(altitude);
		
			//put the point
			gnuplot_cmd( gp_h, "%f %f %f %d", v[0], v[1], v[2], clustered[c].C_ind+1 );
		
			//if we reached the end of the cluster, close the current one
			if( clustered[c].K_ind != clustered[(c+1)%162].K_ind )
				gnuplot_cmd( gp_h, "e\n" );
		}
		
		return gp_h;
	}

}
