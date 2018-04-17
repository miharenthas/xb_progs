//implementation of xb_cluster

#include "xb_cluster.h"
#include <stdio.h>


namespace XB{

	//-------------------------------------------------------------------
	//ordered energy deposit list operators
	//basically, what they do is restrict the comparison to the energy
	//except the equality and disequality ones, which compare all the entries
	bool weak_equality( const oed &one, const oed &two ){
		return (one.e == two.e || one.t == two.t || one.i == two.i);
	}
	
	bool weak_inequality( const oed &one, const oed &two ){
		return (one.e != two.e || one.t != two.t || one.i != two.i);
	}
	
	bool operator==( const oed &one, const oed &two ){
		return (one.e == two.e && one.t == two.t && one.i == two.i);
	}

	bool operator!=( const oed &one, const oed &two ){
		return !(one.e == two.e && one.t == two.t && one.i == two.i);
	}

	bool operator<( const oed &one, const oed &two ){
		return one.e < two.e;
	}

	bool operator>( const oed &one, const oed &two ){
		return one.e > two.e;
	}

	bool operator<=( const oed &one, const oed &two ){
		return one.e <= two.e;
	}

	bool operator>=( const oed &one, const oed &two ){
		return one.e >= two.e;
	}

	//-------------------------------------------------------------------
	//the function that makes the ordered energy list starting from an event
	//the length of the list is guaranteed to be the same as the length of the event.
	//NOTE: this thing *allocates* on the heap, use free to clean up after use!
	oed* make_energy_list( const data &evnt ){
		//if the event is empty, return NULL
		if( evnt.n == 0 ) return NULL;

		//allocate the list
		oed* ordered_energy_list = (oed*)calloc( evnt.n, sizeof(oed) );
		//check the allocation
		if( ordered_energy_list == NULL ) throw error( "Memory error!", "make_energy_list" );
	
		for( int i=0; i < evnt.n; ++i ){
			//get the timestamp
			ordered_energy_list[i].t = evnt.t[i];
		
			//this might be refined later, in dependence of what actually
			//you can find in the data of your experiment
			if( !evnt.empty_e ) ordered_energy_list[i].e = evnt.e[i];
		
			//get the crystal index
			ordered_energy_list[i].i = evnt.i[i];
		}
	
		//now, sort
		std::sort( ordered_energy_list, ordered_energy_list+evnt.n );
		std::reverse( ordered_energy_list, ordered_energy_list+evnt.n );
		
		//return the list
		return ordered_energy_list;
	}

	//----------------------------------------------------------------------
	//the function that calculates the distance of two crystal (int the altitude,azimuth space)
	//NOTE: since this quantity is used for clustering, the actual radius of the XB is irrelevant
	//      and thus assumed equal to unity
	//NOTE: The formula used is the Vincenty formula, in the special case for a sphere.
	//      if less precision is required, a cast to a lesser numerical type can be used.
	long double angular_distance( long double alt_1, long double az_1,
	                              long double alt_2, long double az_2 ){
		long double c_1 = cos( alt_1 );
		long double s_1 = sin( alt_1 );
		long double c_2 = cos( alt_2 );
		long double s_2 = sin( alt_2 );
		long double c_D = cos( abs( az_2 - az_1 ) );
		long double s_D = sin( abs( az_2 - az_1 ) );
	
		return atan2( sqrt( pow( c_2*s_D, 2 ) + pow( c_1*s_2 - s_1*c_2*c_D, 2) ),
                  (s_1*s_2 + c_1*c_2*c_D) );
	}

	//-----------------------------------------------------------------------
	//the function that makes one cluster based on near-neigbours of order "order"
	//around "centroid". It's the less smart-but-faster option.
	cluster make_one_cluster_NN( const data &evnt, unsigned int order ){
		//get the centroid's index (max energy deposit)
		oed* list = make_energy_list( evnt ); //produce the list
		
		//make a crystal ball
		xb_ball cb;
		
		//init the cluster
		cluster kl; //initiate the cluster
		kl.centroid_id = 0; //set the cluster's centroid
		kl.sum_e = 0; //init the sum energy to 0
		kl.n = 0; //init the number of crystals in the cluster to 0
		
		if( list == NULL ) throw error( "Empty event!", "make_one_cluster_NN" ); //check that it's not empty

		//make the cluster
		//loop on the list until you find an entry that makes sense
		for( int i=0; i < evnt.n; ++i ){
			if( IS_LIST_INVALID( list[i] ) ){ continue; }
			else{
				kl.centroid_id = list[i].i;
				kl.c_altitude = cb.at( kl.centroid_id ).altitude;
				kl.c_azimuth = cb.at( kl.centroid_id ).azimuth;
				break;
			}
		}
		//check that something has been found
		if( kl.centroid_id == 0 ) throw error( "Empty event!", "make_one_cluster_NN" );

		//get the list of near neigbours
		unsigned int n_neighs = 0; //here the number of neighbours will be stored
		unsigned int *neighbours = neigh( cb.ball[kl.centroid_id-1], order, n_neighs ); //get the neighbours
		                                                                              //up to order "order"
		//look for them into the list
		for( int i=0; i < evnt.n; ++i ){
			if( std::binary_search( neighbours, neighbours+n_neighs, evnt.i[i] ) ){ //we have a match
				kl.crys.push_back( evnt.i[i] ); //add the crystal to the cluster
				kl.crys_e.push_back( evnt.e[i] ? evnt.e[i] : 0 ); //add the single energy deposit
				++kl.n; //increment the number of crystals in the cluster
			}
		}
		
		//calculate the energy sum
		for( int i=0; i < kl.crys_e.size(); ++i ) kl.sum_e += kl.crys_e[i];
		
		//a cleanup that went missing
		free( list );
		free( neigbours );
		
		return kl;
	}

	//------------------------------------------------------------------------
	//make one cluster by "beading" the energy deposits together instead of just summing
	//the ring of neighbours
	cluster make_one_cluster_bead( const data &evnt, unsigned int order ){
		oed *list = make_energy_list( evnt ); //an ordered energy list
		unsigned int lile = evnt.n; //length of the list
		xb_ball cb; //the crystal ball
		
		//init the cluster
		cluster kl; //initiate the cluster
		kl.centroid_id = 0; //set the cluster's centroid
		kl.sum_e = 0; //init the sum energy to 0
		kl.n = 0; //init the number of crystals in the cluster to 0
		
		if( list == NULL ) throw error( "Empty event!", "make_one_cluster_bead" ); //check that it's not empty

		//make the cluster
		//loop on the list until you find an entry that makes sense
		for( int i=0; i < evnt.n; ++i ){
			if( IS_LIST_INVALID( list[i] ) ){ continue; }
			else{
				kl.centroid_id = list[i].i;
				kl.c_altitude = cb.at( kl.centroid_id ).altitude;
				kl.c_azimuth = cb.at( kl.centroid_id ).azimuth;
				break;
			}
		}
		//check that something has been found
		if( kl.centroid_id == 0 ) throw error( "Empty event!", "make_one_cluster_bead" );
		
		unsigned int n_neigh;
		oed current_k = {0, 1, 1};
		unsigned int neighbours = neigh( cb.at( kl.centroid_id ), 1, n_neigh );
		while( kl.n <= order && current_k.i && current_k.e ){
			current_k.i = 0;
			current_k.e = 0;
			for( int i=0; i < lile; ++i ){
				//NOTE: the first crystal added will always be the centroid
				//      typically at list[0].
				if( std::binary_search( neighbours, neighbours+n_neigh, list[i].i &&
				    list[i].i != current_k.i && list[i].e > current_k.e ){
					current_k.i = list[i].i;
					current_k.e = list[i].e;
				}
			}
			
			//if no crystal was found, get out of the while loop
			free( neighbours ); neighbours = NULL;
			if( current_k.i && current_k.e ){
				kl.crys.push_back( current_k.i );
				kl.crys_e.push_back( current_k.e );
				neighbours = neigh( cb.at( current_k.i ), 1, neigh_n );
				std::remove( list, list+lile, current_k );
				lile--;
			}
		}
		//calculate the energy sum
		for( int i=0; i < kl.crys_e.size(); ++i ) kl.sum_e += kl.crys_e[i];
		
		free( list );
		return kl;
	}
	
	//------------------------------------------------------------------------
	//the function that does clustering on a near-neighbours base.
	//it basically runs make_one_cluster_NN() until the event is empty.
	clusterZ make_clusters( const data &evnt, unsigned int order=1,
	                        cluster (*k_alg)( data&, unsigned int ) ){
		data new_evnt, the_evnt( evnt );
		clusterZ the_clusters; //alloc the clustes
		cluster kl; //a cluster

		//loop until empty
		the_clusters.n = 0;
		the_clusters.evnt = the_evnt.evnt;
		the_clusters.in_beta = the_evnt.in_beta;
		the_clusters.tpat = the_evnt.tpat;
		the_clusters.in_Z = the_evnt.in_Z;
		the_clusters.in_A_on_Z = the_evnt.in_A_on_Z;
		while( the_evnt.n ){
			try{
				kl = k_alg( the_evnt, order );
				the_clusters.clusters.push_back( kl );
				++the_clusters.n;
			}catch( error e ){ return the_clusters; }
			
			//empty the event of the associated crystals
			new_evnt = data( the_evnt.n-kl.n, evnt.evnt ); //redo the event
			new_evnt.tpat = the_evnt.tpat;
			new_evnt.in_Z = the_evnt.in_Z;
			new_evnt.in_A_on_Z = the_evnt.in_A_on_Z;
			new_evnt.in_beta = the_evnt.in_beta;

			new_evnt.empty_t = the_evnt.empty_t;
			new_evnt.empty_pt = the_evnt.empty_pt;
			new_evnt.empty_e = the_evnt.empty_e;
			new_evnt.empty_he = the_evnt.empty_he;
			new_evnt.empty_sum_e = the_evnt.empty_sum_e;
			new_evnt.sum_e = the_evnt.sum_e;

			int cc=0; //loop index for "new_evnt"
			for( int c=0; c < the_evnt.n && cc < new_evnt.n; ++c ){
				if( !std::binary_search( kl.crys.begin(), kl.crys.end(), the_evnt.i[c] ) ){
					new_evnt.i[cc] = the_evnt.i[c];
					new_evnt.t[cc] = the_evnt.t[c];
					new_evnt.pt[cc] = the_evnt.pt[c];
					new_evnt.e[cc] = the_evnt.e[c];
					new_evnt.he[cc] = the_evnt.he[c];
					++cc;
				}
			}
			
			//swap the events
			the_evnt = new_evnt;
		}
		
		return the_clusters;
	}
} //namespace
