//this header contains all the selector usable in xb_make_spc

#ifndef XB_MAKE_SPC__SELECTORS__H
#define XB_MAKE_SPC__SELECTORS__H

#include <functional>

#include "xb_data.h"
#include "xb_cluster.h"

namespace XB{
	//------------------------------------------------------------------------------------
	//selector enumerator -- used to select selector
	typedef enum _select_selector {
		IS_MULTIPLICITY = 'A',
		IS_CENTROID,
		IS_MORE_CRYSTALS,
		IS_MORE_ALTITUDE,
		IS_MORE_AZIMUTH,
		IS_MORE_NRG
	} selsel;
	
	//------------------------------------------------------------------------------------
	//a (morally) vitual class to allow for polymorphism
	class xb_selector : public std::unary_function< cluster, bool > {
		public:
			virtual bool operator()( const cluster &kl ) { return 0; };
	};
	
	//------------------------------------------------------------------------------------
	//a functional to test for multiplicity
	//this functional returns TRUE whenever the_multiplicity is NOT the one
	//with which is constructed. This is because std::remove_if removes on TRUE.
	class is_multiplicity : public std::unary_function< clusterZ, bool > {
		public:
			//constructors
			is_multiplicity(): _mul( 1 ), _r_val( 0 ) {};
			is_multiplicity( unsigned int mult, bool r_val=0 ):
				_mul( mult ), _r_val( r_val ) {};
			is_multiplicity( const is_multiplicity &given ):
				_mul( given._mul ), _r_val( given._r_val ) {};
	
			//the operator()
			bool operator()( const clusterZ &klZ ){
				return ( klZ.n == _mul )? _r_val : !_r_val ; };
		
			//assignment operator?
			is_multiplicity &operator=( const is_multiplicity &given ){
				_mul = given._mul; _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _mul;
			bool _r_val;
	};
	
	//------------------------------------------------------------------------------------
	//select on a particular number of crystals
	//returns true if a particular cluster has the target centroid
	class is_centroid : public xb_selector {
		public:
			//ctors
			is_centroid(): _ctr( 42 ), _r_val( 0 ) {};
			is_centroid( unsigned int ctr, bool rv ): _ctr( ctr ), _r_val( rv ) {
				if( _ctr > 162 ) _ctr = _ctr%162;
			};
			is_centroid( const is_centroid &given, bool rv=0 ):
				_ctr( given._ctr ), _r_val( rv ) {};
			
			//the operator()
			bool operator()( const cluster &kl ){
				return ( kl.centroid_id == _ctr )? _r_val : !_r_val; }
			
			//assignmet operator
			is_centroid &operator=( const is_centroid &given ){
				_ctr = given._ctr; _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _ctr;
			bool _r_val;
	};
	
	//------------------------------------------------------------------------------------
	//check on the number of crystals in the cluster
	//true if the number of crystals is strictly more than _nb_cry
	class is_more_crystals : public xb_selector {
		public:
			is_more_crystals(): _nb_cry( 1 ), _r_val( 0 ) {};
			is_more_crystals( unsigned int nb_cry, bool rv=0 ):
				_nb_cry( nb_cry ), _r_val( rv ) {};
			is_more_crystals( const is_more_crystals &given ):
				_nb_cry( given._nb_cry ), _r_val( given._r_val ) {};
				
			bool operator()( const cluster &given ) {
				return ( given.n > _nb_cry ) ? _r_val : !_r_val;
			};
			
			is_more_crystals &operator=( is_more_crystals &given ){
				_nb_cry = given._nb_cry; _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _nb_cry;
			bool _r_val;
	};
	
	//------------------------------------------------------------------------------------
	//check on the altitude
	//returns true if the centroid's altitude is strictly more
	//than _alt
	class is_more_altitude : public xb_selector {
		public:
			is_more_altitude(): _alt( 1 ), _r_val( 0 ) {};
			is_more_altitude( unsigned int alt, bool rv=0 ):
				_alt( alt ), _r_val( rv ) {};
			is_more_altitude( const is_more_altitude &given ):
				_alt( given._alt ), _r_val( given._r_val ) {};
				
			bool operator()( const cluster &given ) {
				return given.c_altitude > _alt;
			};
			
			is_more_altitude &operator=( is_more_altitude &given ){
				_alt = given._alt, _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _alt;
			bool _r_val;
	};
	
	//------------------------------------------------------------------------------------
	//check on the azimuth
	//returns true if the centroid's azimuth is strictly more
	//than _azi
	class is_more_azimuth : public xb_selector {
		public:
			is_more_azimuth(): _azi( 1 ), _r_val( 0 ) {};
			is_more_azimuth( unsigned int azi, bool rv=0 ):
				_azi( azi ), _r_val( rv ) {};
			is_more_azimuth( const is_more_azimuth &given ):
				_azi( given._azi ), _r_val( given._r_val ) {};
				
			bool operator()( const cluster &given ) {
				return ( given.c_azimuth > _azi )? _r_val : !_r_val;
			};
			
			is_more_azimuth &operator=( is_more_azimuth &given ){
				_azi = given._azi; _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _azi;
			bool _r_val;
	};
	
	//------------------------------------------------------------------------------------
	//check on the energy
	//returns true if the cluster's sum energy is strictly more
	//than _nrg
	class is_more_energy : public xb_selector {
		public:
			is_more_energy(): _nrg( 1 ), _r_val( 0 ) {};
			is_more_energy( unsigned int nrg, bool rv=0 ):
				_nrg( nrg ), _r_val( rv ) {};
			is_more_energy( const is_more_energy &given ):
				_nrg( given._nrg ), _r_val( given._r_val ) {};
				
			bool operator()( const cluster &given ) {
				return ( given.sum_e > _nrg )? _r_val : !_r_val;
			};
			
			is_more_energy &operator=( is_more_energy &given ){
				_nrg = given._nrg; _r_val = given._r_val; return *this;
			};
		private:
			unsigned int _nrg;
			bool _r_val;
	};
}

#endif
