/*this is the interface for a clustering algorithm for the crystall ball */
/*it has been develoed for use with DATA and SIMULATIONS relative to     */
/*EXPERIMENT s412. For other purposes, please ask.                       */
/*                                                                       */
/*The algorithm itself works (sort of) like K-means, although, due to    */
/*the not huge number of crystals, it will usually behave in a simpler   */
/*way.                                                                   */

#ifndef XB_CLUSTER__H
#define XB_CLUSTER__H

#include <algorithm>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "xb_ball.h"
#include "xb_data.h"
#include "xb_error.h"

//TODO: the time info SHOULD be used, but it isn't yet
//      also, it's not checked any longer because it create problems
//      with minimum bias events (?)
#define IS_LIST_INVALID( l ) l.i < 1 || l.i > 162 /*|| l.t == 0*/ || l.e == 0

namespace XB{
	//this structure holds the information about one cluster
	typedef struct _xb_one_cluster{
		unsigned int n; //number of crystals in the cluster
		unsigned int centroid_id; //index of the crystal where the centroid is
		float c_altitude; //angles describing the
		float c_azimuth; //centroid (altitude and azimuth)
		float sum_e; //the sum of the energies in the cluster
		std::vector<float> crys_e; //distances (in radians) of each crystal from the centroid.
		std::vector<unsigned int> crys; //array of crystal indexes participating
	} cluster;
	
	//this structure holds all the clusters
	typedef class _xb_clusters_per_event : public event_holder {
		public:
			float in_beta; //in beat, for doppler correction.
			std::vector<cluster> clusters; //array of culsters
	} clusterZ;
	
	
	//this structure allows to keep track of the energy
	//and the index of a crystal
	typedef struct _xb_one_energy_deposit {
		double t; //timestamp
		double e; //energy readout
		unsigned int i; //index of the crystal (global!)
	} oed;
	
	//the XB::oed strucure comes with its own bool operators
	bool weak_equality( const oed &one, const oed &two );
	bool weak_inequality( const oed &one, const oed &two );
	bool operator==( const oed &one, const oed &two );
	bool operator!=( const oed &one, const oed &two );
	bool operator<( const oed &one, const oed &two );
	bool operator>( const oed &one, const oed &two );
	bool operator<=( const oed &one, const oed &two );
	bool operator>=( const oed &one, const oed &two );
	

	/*** K-MEANS is not implemented yet!
	//This function is supposed to work out by itself how many clusters there ought to be
	//uses K-means --this is supposed to help when there are many hits (>5).
	clusterZ make_clusters_Kmeans( const data &evnt );
	//this function requires an explicit number of centroids specified (then it finds them)
	//uses K-means --this is supposed to help when there are many hits (>5).
	clusterZ make_clusters_Kmeans( const data &evnt, unsigned int K );
	//core K-means functions
	void Kmeans_associate( oed*, clusterZ&, unsigned int ); //associate the deposits in oed* to
	                                                        //the closest centroid
	bool Kmeans_centroid_update( clusterZ&, unsigned int ); //update the centroids
	***/
	
	//calls the algorithm pointed at by function (k_alg), which should take in
	//a data structure (an event), an integer (order) and must return a cluster
	clusterZ make_clusters( const data &evnt, unsigned int order,
	                        cluster (*k_alg)( data&, unsigned int ) );
	
	//this function uses a neares-neighbour clustering
	//kicks in when there are less hits (<5).
	cluster make_one_cluster_NN( const data &evnt, unsigned int order ); //builds a near-neighbours
	                                                                     //based cluster	
	//this function are a more refined nearest neighbour algorithm
	//they essentially follow the energy deposits up to "order" many
	cluster make_one_cluster_bead( const data &evnt, unsigned int order );
	
	//aux functions
	oed* make_energy_list( const data &evnt ); //get a sorted list, by energy, of the event
	long double angular_distance( long double, long double,
	                              long double, long double ); //calculate the angular distance of
	                                                          //two crystals (radians)
}

#endif
