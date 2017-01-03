//this program tries to debug the NN clustering...
//it's an extension of xb_energy_list

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "xb_cluster.h"
#include "xb_data.h"
#include "xb_io.h"
#include "xb_draw_cluster_ball.h"

int main( int argc, char **argv ){
	//input thingies
	char in_fname[256];
	bool in_flag = false;
	int wait_for = 5;
	unsigned int k_4_kmeans = 2;
	
	//parse input
	char iota = 0;
	while( (iota = getopt( argc, argv, "i:w:o:" )) != -1 ){
		switch( iota ){
			case 'i': //-i <file_name>
				in_flag = true;
				if( strlen( optarg ) <= 256 )
					strcpy( in_fname, optarg );
				else{
					printf( "Input file name is too long.\n" );
					exit( 1 );
				}
				break;
			case 'w': //-w <wait_for_sec>
				wait_for = atoi( optarg );
				break;
			case 'k': //-k <neighbour_order>
				k_4_kmeans = atoi( optarg );
				break;
		}
	}
	
	//declare the data array
	std::vector<XB::data*> xb_book;
	
	printf( "Reading from: %s\n", in_fname );
	
	//populate it (also from stdin)
	if( in_flag ) XB::load( in_fname, xb_book );
	else XB::load( stdin, xb_book );
	
	//declare the energy list
	XB::oed *energy_list;
	XB::clusterZ klZ; //and the cluster bin
	
	gnuplot_ctrl *gp_h; //a handle to gnuplot, so that the events don't stack
	
	//loop on the events
	for( int i=0; i < xb_book.size(); ++i ){
		//create the list and look at it
		energy_list = XB::make_energy_list( *xb_book[i] );
		for( int j=0; j < xb_book[i]->n; ++j )
			printf( "Timestamp: %f\nEnergy: %f\nCrystal ID: %u\n\n",
			        energy_list[j].t, energy_list[j].e, energy_list[j].i );
		
		//use it to drive one instance of make_one_cluster_NN
		try{
			klZ = XB::make_clusters_Kmeans( *xb_book[i], k_4_kmeans );
		}catch( XB::error e ){
			printf( "This happened: %s\n", e.what );
			continue;
		}
		
		printf( "klZ multiplicity: %u\n", klZ.multiplicity );
		
		//look at it
		for( int i=0; i < klZ.multiplicity; ++i ){
			printf( "cluster %d:\n", i );
			printf( "centroid_id: %u\n", klZ.clusters[i].centroid_id );
			printf( "azimuth: %f\n", klZ.clusters[i].c_azimuth );
			printf( "altitude: %f\n\n", klZ.clusters[i].c_altitude );
		}
		
		//gp_h = XB::draw_cluster_ball( klZ );
		sleep( wait_for );
		//gnuplot_close( gp_h );
	}
	
	return 0;
}
