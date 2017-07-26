//this header contains all the selector usable in xb_make_spc

#ifndef XB_MAKE_SPC__SELECTORS__H
#define XB_MAKE_SPC__SELECTORS__H

#include <functional>

#include "xb_data.h"
#include "xb_cluster.h"

namespace XB{
	//------------------------------------------------------------------------------------
	//selector enumerator -- used to select selector
	typedef enum _select_selector{
		IS_NOT_MULTIPLICITY = 0, //for historical reasons, it works this way...
		IS_CENTROID,
		IS_MORE_CRYSTALS
		IS_MORE_ALTITUDE,
		IS_MORE_AZIMUTH,
		IS_MORE_NRG
	} selsel;
		
	//------------------------------------------------------------------------------------
	//a functional to test for multiplicity
	//this functional returns TRUE whenever the_multiplicity is NOT the one
	//with which is constructed. This is because std::remove_if removes on TRUE.
	typedef class is_not_multiplicity : public std::unary_function< clusterZ, bool > {
		public:
			//constructors
			is_not_multiplicity(): _mul( 1 ) {};
			is_not_multiplicity( unsigned int mult ): _mul( mult ) {};
			is_not_multiplicity( const is_not_multiplicity &given ):
				_mul( given._mul ) {};
	
			//the operator()
			bool operator()( const XB::clusterZ &klZ ){ return klZ.n != _mul; };
		
			//assignment operator?
			is_not_multiplicity &operator=( const is_not_multiplicity &given ){
				this->_mul = given._mul; return *this;
			};
		private:
			unsigned int _mul;
	} isntm;
	
	//------------------------------------------------------------------------------------
	//select on a particular number of crystals
	//returns true if a particular cluster has the target centroid
	class is_centroid : public std::unary_function< cluster, bool > {
		public:
			//ctors
			is_centroid(): _ctr( 42 ) {}; //by default, select the right answer
			is_centroid( unsigned int ctr ): _ctr( ctr ) {
				if( _ctr > 162 ) _ctr = _ctr%162;
			};
			is_centroid( const is_centroid &given ): _ctr( given._ctr ) {};
			
			//the operator()
			bool operator()( const XB::cluster &kl ){ return kl.centroid_id == _ctr; }
			
			//assignmet operator
			is_centroid &operator=( const is_centroid &given ){
				this->_ctr = given._ctr; return *this;
			};
		private:
			unsigned int _ctr;
	}
	
	//------------------------------------------------------------------------------------
	//check on the number of crystals in the cluster
	//true if the number of crystals is strictly more than _nb_cry
	class is_more_crystals : public std::unary_function< cluster, bool > {
		public:
			is_more_crystals(): _nb_cry( 1 ) {};
			is_more_crystals( unsigned int nb_cry ): _nb_cry( nb_cry ) {};
			is_more_crystals( cosnt is_more_crystals &given ):
				_nb_cry( given._nb_cry ) {};
				
			bool operator()( const XB::cluster &given ) {
				return kl.n > _nb_cry;
			};
			
			is_more_crystals &operator=( is_more_crystals &given ){
				this->_nb_cry = given._nb_cry; return *this;
			};
		private:
			unsigned int _nb_cry;
	};
	
	//------------------------------------------------------------------------------------
	//check on the altitude
	//returns true if the centroid's altitude is strictly more
	//than _alt
	class is_more_altitude : public std::unary_function< cluster, bool > {
		public:
			is_more_altitude(): _alt( 1 ) {};
			is_more_altitude( unsigned int alt ): _alt( alt ) {};
			is_more_altitude( cosnt is_more_altitude &given ):
				_alt( given._alt ) {};
				
			bool operator()( const XB::cluster &given ) {
				return kl.c_altitude > _alt;
			};
			
			is_more_altitude &operator=( is_more_altitude &given ){
				this->_alt = given._alt; return *this;
			};
		private:
			unsigned int _alt;
	};
	
	//------------------------------------------------------------------------------------
	//check on the azimuth
	//returns true if the centroid's azimuth is strictly more
	//than _azi
	class is_more_azimuth : public std::unary_function< cluster, bool > {
		public:
			is_more_azimuth(): _azi( 1 ) {};
			is_more_azimuth( unsigned int azi ): _azi( azi ) {};
			is_more_azimuth( cosnt is_more_azimuth &given ):
				_azi( given._azi ) {};
				
			bool operator()( const XB::cluster &given ) {
				return kl.c_azimuth > _azi;
			};
			
			is_more_azimuth &operator=( is_more_azimuth &given ){
				this->_azi = given._azi; return *this;
			};
		private:
			unsigned int _azi;
	};
	
	//------------------------------------------------------------------------------------
	//check on the energy
	//returns true if the cluster's sum energy is strictly more
	//than _nrg
	class is_more_energy : public std::unary_function< cluster, bool > {
		public:
			is_more_energy(): _nrg( 1 ) {};
			is_more_energy( unsigned int nrg ): _nrg( nrg ) {};
			is_more_energy( cosnt is_more_energy &given ):
				_nrg( given._nrg ) {};
				
			bool operator()( const XB::cluster &given ) {
				return kl.sum_e > _nrg;
			};
			
			is_more_energy &operator=( is_more_energy &given ){
				this->_nrg = given._nrg; return *this;
			};
		private:
			unsigned int _nrg;
	};
}

#endif
