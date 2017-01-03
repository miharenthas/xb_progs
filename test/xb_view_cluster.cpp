//a program that tests the cluster visualizing routine
//why is this important? I need to look at the clusters to be sure that
//the clustering algorithm is doing its job. It can also be beneficial to
//look at the clusters for comparison between different cluster algorithms

#include <stdio.h>
#include <unistd.h>

#include "xb_draw_cluster_ball.h"

int main( int argc, char **argv ){
	XB::clusterZ klZ; //the clusterZ container struct
	XB::cluster dummy_k[2]; //the dummy clusters we're going to look at.
	XB::xb_ball the_cb; //get the usual ball
	unsigned int *neigh_buf, len; //the neighbour buffer and its length
	
	//input variables
	int c_id[] = {0, 161}; //the centroids
	int order = 2; //the neighbourhood order
	bool verbose = false;
	
	//parse input
	char iota = 0;
	while( (iota = getopt( argc, argv, "1:2:o:v" ) ) != -1 ){
		switch( iota ){
			case '1': //-1=<first centroid>
				c_id[0] = atoi( &optarg[1] )-1;
				break; 
			case '2': //-2=<second centroid>
				c_id[1] = atoi( &optarg[1] )-1;
				break;
			case 'o': //-o=<order>
				order = atoi( &optarg[1] );
				break;
			case 'v':
				verbose = true;
				break;
		}
	}
	
	if( verbose ){
		printf( "centroid ids: %u, %u.\n", c_id[0], c_id[1] );
		printf( "order: %u.\n", order );
	}
	
	//set the two clusters 
	neigh_buf = XB::neigh( the_cb.ball[c_id[0]], order, len );
	dummy_k[0].n = len;
	dummy_k[0].centroid_id = c_id[0];
	dummy_k[0].c_altitude = the_cb.ball[c_id[0]].altitude;
	dummy_k[0].c_azimuth = the_cb.ball[c_id[0]].azimuth;
	dummy_k[0].crys = std::vector<unsigned int>( neigh_buf, neigh_buf+len );
	
	if( verbose ) printf( "cluster 1 populated.\n" );
	
	neigh_buf = XB::neigh( the_cb.ball[c_id[1]], order, len );
	dummy_k[1].n = len;
	dummy_k[1].centroid_id = c_id[1];
	dummy_k[1].c_altitude = the_cb.ball[c_id[1]].altitude;
	dummy_k[1].c_azimuth = the_cb.ball[c_id[1]].azimuth;
	dummy_k[1].crys = std::vector<unsigned int>( neigh_buf, neigh_buf+len );
	
	if( verbose ) printf( "cluster 2 populated.\n" );
	
	//add the two clusters to the clusterZ bin
	klZ.multiplicity = 2;
	klZ.clusters.push_back( dummy_k[0] );
	klZ.clusters.push_back( dummy_k[1] );
	
	if( verbose ) printf( "clusters added to clusterZ struct.\n" );
	
	//draw it
	XB::draw_cluster_ball( klZ );
	
	//let the user admire the product.
	sleep( 120 );
	
	//happy toughts
	return 0;
}
