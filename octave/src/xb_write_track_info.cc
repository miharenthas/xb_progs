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

//a function to convert from scalar maps to versors
XB::versor struct2versor( const octave_scalar_map &given );

DEFUN_DLD( xb_write_track_info, args, , "XB::write data interface for Octave" ){
	if( sizeof(octave_uint32) != sizeof(unsigned int) ){
		error( "Quirky types." );
	}

	//check that there are two arguments and
	//that the first one is a string
	if( args.length() != 2 || !args(0).is_string() ){
		error( "xb_wite_track_info: need a filename and an array of structures.\n" );
		return octave_value_list();
	}
	
	//retrieve the array of structures
	//and check that we have got it.
	octave_map o_data_m = args(1).map_value();
	/*if( !o_data_m.is_zero_by_zero() ){
		error( "xb_data_write: invalid argument\n" );
		return octave_value_list();
	}*/
	
	std::vector<XB::track_info*> data;
	
	//if we got here, we should be able to proceed.
	//declare the necessary bits and pieces
	unsigned int current_numel = 0, current_evnt = 0;
	XB::track_info *buf;
	octave_scalar_map o_map;
	
	//loop-copy the data
	for( int i=0; i < o_data_m.length(); ++i ){
		o_map = o_data_m(i);
		
		current_numel = o_map.getfield( "n" ).uint_value();
		current_evnt = o_map.getfield( "evnt" ).uint_value();
		
		//make the data (dynamic)
		buf = new XB::track_info( current_numel, current_evnt );
		
		//do the copying
		buf->in_beta = o_map.getfield( "in_beta" ).float_value();
		buf->beta_0 = o_map.getfield( "beta_0" ).float_value();
		buf->in_Z = o_map.getfield( "in_Z" ).float_value();
		buf->in_A_on_Z = o_map.getfield( "in_A_on_Z" ).float_value();
		
		if( o_map.isfield( "fragment_A" ) ){
			if( !o_map.getfield( "fragment_A" ).is_zero_by_zero() ){
				memcpy( buf->fragment_A,
					o_map.getfield( "fragment_A" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
			}
		}
		
		if( o_map.isfield( "fragment_Z" ) ){
			if( !o_map.getfield( "fragment_Z" ).is_zero_by_zero() ){
				memcpy( buf->fragment_Z,
					o_map.getfield( "fragment_Z" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
			}
		}
		
		if( o_map.isfield( "fragment_beta" ) ){
			if( !o_map.getfield( "fragment_beta" ).is_zero_by_zero() ){
				memcpy( buf->fragment_beta,
					o_map.getfield( "fragment_beta" ).float_array_value().fortran_vec(),
					current_numel*sizeof(float) );
			}
		}
		
		if( o_map.isfield( "incoming" ) ){
			for( int v=0; v < current_numel; ++v ){
				buf->incoming[v] = struct2versor( o_map.getfield( "incoming" ).
				                                        cell_value()(v).
				                                        scalar_map_value() );
			}
		}
		
		if( o_map.isfield( "outgoing" ) ){
			for( int v=0; v < current_numel; ++v ){
				buf->outgoing[v] = struct2versor( o_map.getfield( "outgoing" ).
				                                        cell_value()(v).
				                                        scalar_map_value() );
			}
		}
		
		data.push_back( buf );
		
		//remove the current element (hopefully)
		o_data_m(i).clear();
	}
	
	//cleanup
	o_data_m.clear();
	
	//write on file
	char out_fname[256];
	strcpy( out_fname, args(0).string_value().c_str() );
	try{
		XB::write( out_fname, data );
	} catch( XB::error e ){
		error( e.what );
	}
	
	//more cleanup
	for( int i=0; i < data.size(); ++i ) delete data[i];
	
	//happy thoughts
	return octave_value_list();
}

//implementation of the helper function for struct to versor tranlation
XB::versor struct2versor( const octave_scalar_map &given ){
	XB::versor v;
	v.i = given.getfield( "i" ).float_value();
	v.j = given.getfield( "j" ).float_value();
	v.k = given.getfield( "k" ).float_value();
	
	return v;
}
