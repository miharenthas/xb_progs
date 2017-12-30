//this file provides some tools to actually perform the cut.
//included, there's the interface base class to extract the
//data points to feed into the cut primitives

#ifndef XB_CUT__H
#define XB_CUT__H

#include <string.h> //memcpy
#include <functional>
#include <vector>
#include <algorithm> //std::remove_if

#include <unistd.h> //sysconf
#ifndef __clang__
	#include <omp.h> //for OpenMP
#endif
#include <math.h> //ceil

#include "xb_data.h"
#include "xb_cut_typedefs.h"

#define __XB_CUT_PARALLEL_THRS 100000
#define __XB_CUT_PARALLEL_POLY_THRS 1000
#define __XB_CUT_PARALLEL_REGULAR_POLY_THRS 100

namespace XB{
	//----------------------------------------------------------------------------
	//a little class that will hold the extracted data as a couple of doubles
	//NOTE: this may become a template...
	typedef class _xb_pt_holder {
		public:
			_xb_pt_holder(); //neutered std constructor
			_xb_pt_holder( double &x, double &y );
			_xb_pt_holder( double *pt );
			_xb_pt_holder( _xb_pt_holder &given );
		
			double &operator[]( unsigned int index );
			operator double*();
			_xb_pt_holder &operator=( _xb_pt_holder& right );
		private:			
			double _pt[2];
	} pt_holder;
	
	//----------------------------------------------------------------------------
	//extractor classes: this is a base class to comfortalby extract data
	//from arrays of XB::data classes. A class of these must provide
	//a constructor with array of data and size and the operator
	
	//1D base class:
	template< class xb_T >
	class cut_data_1D : public std::unary_function<unsigned int, double> {
		public:
			cut_data_1D( std::vector<xb_T> &data_array ):
				_data_array( data_array ) {};
			
			virtual double operator()( unsigned int index ) =0;
			inline unsigned int size(){ return _data_array.size(); };
		protected:
			std::vector<xb_T> &_data_array; //the address of the data array
			                                 //that will be
			                                 //handled by the class.
			                                 //This is *NOT* owned
			                                 //and will not be deallocated.
	};
	
	//2D base class
	template< class xb_T >
	class cut_data_2D : public std::unary_function<unsigned int, pt_holder> {
		public:
			cut_data_2D( std::vector<xb_T> &data_array ):
				_data_array( data_array ) {};
			
			virtual pt_holder operator()( unsigned int index ) =0;
			inline unsigned int size(){ return _data_array.size(); };
		protected:
			std::vector<xb_T> &_data_array; //the address of the data array
			                                 //that will be
			                                 //handled by the class.
			                                 //This is *NOT* owned
			                                 //and will not be deallocated.
	};
	
	//----------------------------------------------------------------------------
	//some functions: they apply the_cut to the_data according to the
	//specification of the_spec.
	
	//----------------------------------------------------------------------------
	//these functions apply the cut-those-outside kind of cut (keep
	//those inside)
	//make the 1D cut
	template< class xb_T >
	std::vector<bool> do_cut( cut_data_1D<xb_T> *the_spec,
	                          cut_segment *the_cut );
	                    
	//make the 2D cut
	template< class xb_T >
	std::vector<bool> do_cut( cut_data_2D<xb_T> *the_spec,
	                          _xb_2D_cut_primitive *the_cut ); //all of the 2D

	//these functions applu the cut-those-inside kind of cut (keep
	//those outside)
	//make the 1D cut
	template< class xb_T >
	std::vector<bool> do_cut_not( cut_data_1D<xb_T> *the_spec,
	                              cut_segment *the_cut );
	                    
	//make the 2D cut
	template< class xb_T >
	std::vector<bool> do_cut_not( cut_data_2D<xb_T> *the_spec,
	                              _xb_2D_cut_primitive *the_cut ); //all of the 2D
	
	//----------------------------------------------------------------------------
	//apply the cut given the flags
	template< class xb_T >
	int apply_flagged_cut( std::vector<xb_T> &the_data, std::vector<bool> &flags );
	                
	//============================================================================
	//function implementations (they are templates...)
	//also, they contain some parallel execution support.
	//granularity is very coarse except in the case of polygons and regular poly
	//which may be computationally intensive
	
	//----------------------------------------------------------------------------
	//1D cut
	template< class xb_T >
	std::vector<bool> do_cut( cut_data_1D<xb_T> *the_spec,
	                          cut_segment *the_cut ){
		unsigned int data_size = the_spec->size();
		std::vector<bool> flags( data_size );
		
		//work out the number of threads to use
		//it shall not exceed twice the number of online CPUs
		int n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_THRS );
		n_thr = ( n_thr > 2*sysconf(_SC_NPROCESSORS_ONLN) )?
		        2*sysconf(_SC_NPROCESSORS_ONLN) : n_thr;
		
		#pragma omp parallel num_threads( n_thr ) shared( the_cut, the_spec )
		{
		#pragma omp for schedule( dynamic, __XB_CUT_PARALLEL_THRS )
		for( int i=0; i < data_size; ++i ){
			if( the_cut->contains( (*the_spec)( i ) ) ){
				flags[i] = true;
			}
		}
		} //parallel pragma
		
		return flags;
	}
	
	//----------------------------------------------------------------------------
	//2D cut
	template< class xb_T >
	std::vector<bool> do_cut( cut_data_2D<xb_T> *the_spec,
	                          _xb_2D_cut_primitive *the_cut ){
		unsigned int data_size = the_spec->size();
		std::vector<bool> flags( data_size );
		
		//work out the parallel thershold to use
		//and thus the number of threads, depending on
		//the cut's identity
		int n_thr, p_thrs;
		switch( the_cut->type() ){
			case CUT_POLYGON :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_POLY_THRS );
				p_thrs = __XB_CUT_PARALLEL_POLY_THRS;
				break;
			case CUT_REGULAR_POLYGON :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_REGULAR_POLY_THRS );
				p_thrs = __XB_CUT_PARALLEL_REGULAR_POLY_THRS;
				break;
			default :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_THRS );
				p_thrs = __XB_CUT_PARALLEL_THRS;
				break;
		}
		n_thr = ( n_thr > 2*sysconf(_SC_NPROCESSORS_ONLN) )?
		        2*sysconf(_SC_NPROCESSORS_ONLN) : n_thr;
		
		#pragma omp parallel num_threads( n_thr ) shared( the_cut, the_spec )
		{
		#pragma omp for schedule( dynamic, p_thrs )
		for( int i=0; i < data_size; ++i ){
			if( the_cut->contains( (*the_spec)( i ) ) ){
				flags[i] = true;
			}
		}
		}
		
		return flags;
	}

	//----------------------------------------------------------------------------
	//1D cut
	template< class xb_T >
	std::vector<bool> do_cut_not( cut_data_1D<xb_T> *the_spec,
	                              cut_segment *the_cut ){
		unsigned int data_size = the_spec->size();
		std::vector<bool> flags( data_size );
		
		//work out the number of threads to use
		//it shall not exceed twice the number of online CPUs
		int n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_THRS );
		n_thr = ( n_thr > 2*sysconf(_SC_NPROCESSORS_ONLN) )?
		        2*sysconf(_SC_NPROCESSORS_ONLN) : n_thr;
	
		#pragma omp parallel num_threads( n_thr ) shared( the_cut, the_spec )
		{
		#pragma omp for schedule( dynamic, __XB_CUT_PARALLEL_THRS )
		for( int i=0; i < data_size; ++i ){
			if( !the_cut->contains( (*the_spec)( i ) ) ){
				flags[i] = true;
			}
		}
		}
		
		return flags;
	}
	
	//----------------------------------------------------------------------------
	//2D cut
	template< class xb_T >
	std::vector<bool> do_cut_not( cut_data_2D<xb_T> *the_spec,
	                              _xb_2D_cut_primitive *the_cut ){
		unsigned int data_size = the_spec->size();
		std::vector<bool> flags( data_size );

		//work out the parallel thershold to use
		//and thus the number of threads, depending on
		//the cut's identity
		int n_thr, p_thrs;
		switch( the_cut->type() ){
			case CUT_POLYGON :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_POLY_THRS );
				p_thrs = __XB_CUT_PARALLEL_POLY_THRS;
				break;
			case CUT_REGULAR_POLYGON :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_REGULAR_POLY_THRS );
				p_thrs = __XB_CUT_PARALLEL_REGULAR_POLY_THRS;
				break;
			default :
				n_thr = (int)ceil( the_spec->size()/__XB_CUT_PARALLEL_THRS );
				p_thrs = __XB_CUT_PARALLEL_THRS;
				break;
		}
		n_thr = ( n_thr > 2*sysconf(_SC_NPROCESSORS_ONLN) )?
		        2*sysconf(_SC_NPROCESSORS_ONLN) : n_thr;
		
		#pragma omp parallel num_threads( n_thr ) shared( the_cut, the_spec )
		{
		#pragma omp for schedule( dynamic, p_thrs )
		for( int i=0; i < data_size; ++i ){
			if( !the_cut->contains( (*the_spec)( i ) ) ){
				flags[i] = true;
			}
		}
		}
		
		return flags;
	}
	
	//----------------------------------------------------------------------------
	//apply flagged cut and count how many elements are removed
	//here is a little unary function that allows for the usage of
	//std::remove_if in the flagged_cut function.
	template< class xb_T >
	bool is_marked( xb_T &given ){
		return ((XB::event_holder)given).evnt == 0 &&
		       ((XB::event_holder)given).n == 0; }
				
	template< class xb_T >
	int apply_flagged_cut( std::vector<xb_T> &the_data, std::vector<bool> &flags ){
		int count = 0;
		
		//destroy the unflagged members and set their pointer to NULL
		//meanwhile, count them.
		xb_T *indirect;
		for( int i=0; i < the_data.size(); ++i ){
			if( !flags[i] ){
				indirect = &the_data.at(i);
				indirect->evnt = 0;
				indirect->n = 0;
				++count;
			}
		}
		
		//retrieve the pointers to the beginning and end of the
		//std::vector holding the data, and get another one
		//for the new size
		typename std::vector< xb_T >::iterator pbegin = the_data.begin();
		typename std::vector< xb_T >::iterator pend = the_data.end(), pnew_end;
		
		//get the new end with std::remove_if
		pnew_end = std::remove_if( pbegin, pend, is_marked< xb_T > );
		
		//chop away the tail of the container, now full of nulls
		the_data.erase( pnew_end, pend );
		
		//happy thoughs
		return count;
	}
} //end of namespace

#endif
