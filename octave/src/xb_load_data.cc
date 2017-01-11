//This GNU Octave function provides an interface to load data
//from files created with the XB toolkit.
//The data must have been generated from the class XB::data.
//This means that translated data from experiment and simulation
//can be loaded with this interface.
//NOTE: there's no guarantee whatsoever that this function
//      would be at all usable with MATLAB in a MEX file.
//      It will *not* be tested and it is *not* ment as such.

//stl includes
#include <vector>

//includes from octave
#include <octave/oct.h> //all the gobbins for OCT files
#include <octave/oct-map.h> //data will be reconstructed as a structure (octave_map)
#include <octave/Array.h> //octave arrays arrays
#include <octave/Cell.h> //octave cell arrays

//includes from the toolkit
#include "xb_io.h" //XB::load
#include "xb_data.h" //XB::data
#include "xb_error.h" //XB::error

DEFUN_DLD( xb_load_data, args, nargout, "XB::load data interface for Octave" ){
	//argument checks: they must be one or more strings
	int nargin = args.length();
	
	//chek on the numerical types
	if( sizeof(octave_uint32) != sizeof(unsigned int) ){
		error( "Quirky types." );
	}
	
	if( nargin == 0 ){
		error( "At least one file name must be provided" );
	}
	
	//loop-load the files
	std::vector<XB::data*> data, data_buf;
	char in_fname[256];
	for( int f=0; f < nargin; ++f ){
		if( args(f).is_string() ){ //if the argument is a string
		                           //attempt to load the file
			try{
				strcpy( in_fname, args(f).string_value().c_str() );
				XB::load( in_fname, data_buf );
				data.insert( data.end(), data_buf.begin(), data_buf.end() );
			} catch( XB::error e ) {
				error( e.what );
				continue;
			}
		} else {
			octave_stdout << "xb_load_data: warning: argument "
			              << f << " is not a valid filename.\n";
			continue;
		}
	}
	//check that something has been read
	if( !data.size() ){
		octave_stdout << "xb_load_data: warning: no data loaded.\n";
		return octave_value_list();
	}
	
	//now, begin the translation into octave structure
	//first, allocate the octave_map that will hold the thing
	dim_vector o_dim_v( data.size(), 1 ), o_dim_null( 0, 0 ); 
	
	//copy the data:
	//prepare the fields:
	Cell o_field_n( o_dim_v );
	Cell o_field_evnt( o_dim_v );
	Cell o_field_sum_e( o_dim_v );
	Cell o_field_in_beta( o_dim_v );
	Cell o_field_i( o_dim_v );
	Cell o_field_t( o_dim_v );
	Cell o_field_pt( o_dim_v );
	Cell o_field_e( o_dim_v );
	Cell o_field_he( o_dim_v );
	//and some buffers
	Array<octave_uint32> i_buf;
	Array<float> f_buf;

	unsigned int current_numel = 0;
	for( int i=0; i < data.size(); ++i ){
		//check for NULLs
		if( data[i] == NULL ) continue;
		
		current_numel = data[i]->n;
	
		//make the structure:
		//firts, copy the trivially copiable
		o_field_n(i) = current_numel;
		o_field_evnt(i) = data[i]->evnt;
		o_field_sum_e(i) = data[i]->sum_e;
		o_field_in_beta(i) = data[i]->in_beta;
				
		//then, copy the arrays
		//sizing
		dim_vector o_dim( current_numel, 1 );
		i_buf.resize( o_dim );	
		f_buf.resize( o_dim );
		
		//crystal indexes
		if( current_numel ){
			memcpy( i_buf.fortran_vec(), data[i]->i,
			        current_numel*sizeof(unsigned int) );
			o_field_i(i) = i_buf;
		} else {
			o_field_i(i) = Array<octave_uint32>( o_dim_null );
			continue;
		}
		
		//all the rest
		if( !data[i]->empty_t ){
			memcpy( f_buf.fortran_vec(), data[i]->t,
			        current_numel*sizeof(float) );
			o_field_t(i) = f_buf;
		} else o_field_t(i) = Array<float>( o_dim_null );
		
		if( !data[i]->empty_pt ){
			memcpy( f_buf.fortran_vec(), data[i]->t,
			        current_numel*sizeof(float) );
			o_field_t(i) = f_buf;
		} else o_field_t(i) = Array<float>( o_dim_null );
		
		if( !data[i]->empty_e ){
			memcpy( f_buf.fortran_vec(), data[i]->t,
			        current_numel*sizeof(float) );
			o_field_t(i) = f_buf;
		} else o_field_t(i) = Array<float>( o_dim_null );
		
		if( !data[i]->empty_he ){
			memcpy( f_buf.fortran_vec(), data[i]->t,
			        current_numel*sizeof(float) );
			o_field_t(i) = f_buf;
		} else o_field_t(i) = Array<float>( o_dim_null );
		
		//finally, deallocate and nullify the copied element
		delete data[i];
		data[i] = NULL;
		i_buf.clear();
		f_buf.clear();
	}
	
	//make the map
	octave_map o_data_m;
	o_data_m.setfield( "n", o_field_n );
	o_data_m.setfield( "evnt", o_field_evnt );
	o_data_m.setfield( "i", o_field_i );
	o_data_m.setfield( "t", o_field_t );
	o_data_m.setfield( "pt", o_field_pt );
	o_data_m.setfield( "e", o_field_e );
	o_data_m.setfield( "he", o_field_he );
	o_data_m.setfield( "sum_e", o_field_sum_e );
	o_data_m.setfield( "in_beta", o_field_in_beta );
	
	//happy thoughts
	return octave_value_list( octave_value( o_data_m ) );
	return octave_value_list();
}		
