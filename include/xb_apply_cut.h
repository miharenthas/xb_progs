//this header file contains some specializations of the
//"cut_data_xD" classes, in order to be used with more ease (?)

#ifndef XB_APPLY_CUT__H
#define XB_APPLY_CUT__H

#include <vector>

#include "xb_data.h"
#include "xb_cut.h"

namespace XB{
	//----------------------------------------------------------------------------
	//one dimensional cuts:
	
	//1D cut on the number of fragments
	class cut_on_fragment_nb : public cut_data_1D< track_info >{
		public:
			cut_on_fragment_nb( std::vector< track_info* > &data_array ):
				cut_data_1D< track_info >( data_array ) {};
			
			double operator()( unsigned int index );
	};
	
	//cut on the incoming beta
	class cut_on_inbeta : public cut_data_1D< track_info >{
		public:
			cut_on_inbeta( std::vector< track_info* > &data_array ):
				cut_data_1D< track_info >( data_array ) {};
			
			double operator()( unsigned int index );
	};
	
	//cut on the outgoing beta
	class cut_on_beta_0 : public cut_data_1D< track_info >{
		public:
			cut_on_beta_0( std::vector< track_info* > &data_array ):
				cut_data_1D< track_info >( data_array ) {};
			
			double operator()( unsigned int index );
	};
	
	//----------------------------------------------------------------------------
	//two dimensional cuts
	typedef class cut_on_charge_mass_charge_ratio : public cut_data_2D< track_info >{
		public:
			cut_on_charge_mass_charge_ratio( std::vector< track_info* > &data_array ):
				cut_data_2D< track_info >( data_array ) {};
			
			pt_holder operator()( unsigned int index );
	} cut_on_zaz;
}

#endif //XB_APPLY_CUT__H
