//This GNU Octave function provides an interface to write data
//to files readable with XB::read function.
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

DEFUN_DLD( xb_write_data, args, , "XB::write data interface for Octave" ){
	if( sizeof(octave_uint32) != sizeof(unsigned int) ){
		error( "Quirky types." );
	}

	//check that there are two arguments and
	//that the first one is a string
	if( args.length() != 2 || !args(0).is_string() ){
		error( "xb_data_wite: need a filename and an array of structures.\n" );
		return octave_value_list();
	}
	
	//retrieve the array of structures
	//and check that we have got it.
	octave_map o_data_m = args(1).map_value();
	/*if( !o_data_m.is_zero_by_zero() ){
		error( "xb_data_write: invalid argument\n" );
		return octave_value_list();
	}*/
	
	std::vector<XB::data*> data;
	
	//if we got here, we should be able to proceed.
	//declare the necessary bits and pieces
	unsigned int current_numel = 0, current_evnt = 0;
	XB::data *buf;
	octave_scalar_map o_map;
	
	//loop-copy the data
	for( int i=0; i < o_data_m.length(); ++i ){
		o_map = o_data_m(i);
		
		current_numel = o_map.getfield( "n" ).uint_value();
		current_evnt = o_map.getfield( "evnt" ).uint_value();
		
		//make the data (dynamic)
		buf = new XB::data( current_numel, current_evnt );
		
		//do the copying
		buf->sum_e = o_map.getfield( "sum_e" ).float_value();
		if( !buf->sum_e ) buf->empty_sum_e = true;
		buf->in_beta = o_map.getfield( "in_beta" ).float_value();
		
		if( o_map.isfield( "i" ) ){
			if( !o_map.getfield( "i" ).is_zero_by_zero() ){
				memcpy( buf->i,
					o_map.getfield( "i" ).uint32_array_value().fortran_vec(),
					current_numel*sizeof(unsigned int) );
			}
		}
		
		if( o_map.isfield( "t" ) ){
			if( !o_map.getfield( "t" ).is_zero_by_zero() ){
				memcpy( buf->t,
					o_map.getfield( "t" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
				buf->empty_t = false;
			} else buf->empty_t = true;
		} else buf->empty_t = true;
		
		if( o_map.isfield( "pt" ) ){
			if( !o_map.getfield( "pt" ).is_zero_by_zero() ){
				memcpy( buf->pt,
					o_map.getfield( "pt" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
				buf->empty_pt = false;
			} else buf->empty_pt = true;
		} else buf->empty_pt = true;
		
		if( o_map.isfield( "e" ) ){
			if( !o_map.getfield( "e" ).is_zero_by_zero() ){
				memcpy( buf->e,
					o_map.getfield( "e" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
				buf->empty_e = false;
			} else buf->empty_e = true;
		} else buf->empty_e = true;
		
		if( o_map.isfield( "he" ) ){
			if( !o_map.getfield( "he" ).is_zero_by_zero() ){
				memcpy( buf->he,
					o_map.getfield( "he" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
				buf->empty_he = false;
			} else buf->empty_he = true;
		} else buf->empty_he = true;
		
		
		
		data.push_back( buf );
		
		//remove the current element (hopefully)
		o_data_m(i).clear();
	}
	
	//cleanup
	o_data_m.clear();
	
	//write on file
	char out_fname[256];
	strcpy( out_fname, args(0).string_value().c_str() );
	XB::write( out_fname, data );
	
	//more cleanup
	for( int i=0; i < data.size(); ++i ) delete data[i];
	
	//happy thoughts
	return octave_value_list();
}
