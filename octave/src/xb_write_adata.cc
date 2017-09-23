//This GNU Octave function provides an interface to write arbitrary data
//to files readable with XB::read function.
//NOTE: there's no guarantee whatsoever that this function
//      would be at all usable with MATLAB in a MEX file.
//      It will *not* be tested and it is *not* ment as such.

//octave documentation string
#define O_DOC_STRING "-*- texinfo -*-\n\
@deftypefn{Function File} xb_write_data( @var{filename}, @var{adata} )\n\
Writes a file named @var{filename} readable with the programs of the xb_progs toolkit.\n\
\n\
The file is created and overwritten if existing: no warning messages are displayed.\n\
\n\
@example\n\
@group\n\
@result{} structure array adata:\n\
     n\n\
     evnt\n\
     tpat\n\
     in_Z\n\
     in_A_on_Z\n\
     [named fields]...\n\
@end group\n\
@end example\n\
\n\
For more information about the content of the fields, use the documentation of the toolkit.\n\
@end deftypefn"

//stl includes
#include <vector>

//includes from octave
#include <octave/oct.h> //all the gobbins for OCT files
#include <octave/oct-map.h> //data will be reconstructed as a structure (octave_map)
#include <octave/Array.h> //octave arrays arrays
#include <octave/Cell.h> //octave cell arrays
#include <octave/str-vec.h> //for the field names

//includes from the toolkit
#include "xb_io.h" //XB::load
#include "xb_arbitrary_data.h" //XB::data
#include "xb_error.h" //XB::error

DEFUN_DLD( xb_write_adata, args, , O_DOC_STRING ){
	if( sizeof(octave_uint32) != sizeof(unsigned int) ){
		error( "Quirky types." );
	}

	//check that there are two arguments and
	//that the first one is a string
	if( args.length() != 2 || !args(0).is_string() ){
		error( "xb_adata_wite: need a filename and an array of structures.\n" );
		return octave_value_list();
	}
	
	//retrieve the array of structures
	//and check that we have got it.
	octave_map o_data_m = args(1).map_value();
	/*if( !o_data_m.is_zero_by_zero() ){
		error( "xb_data_write: invalid argument\n" );
		return octave_value_list();
	}*/
	
	std::vector<XB::adata> data;
	
	//if we got here, we should be able to proceed.
	//declare the necessary bits and pieces
	XB::adata buf;
	octave_scalar_map o_map;
	
	//get the list of fields to copy in the arbitrary part
	//of arbitrary_data
	string_vector fld_names = o_data_m.fieldnames();
	string_vector std_names;
	std_names.append( "n" );
	std_names.append( "evnt" );
	std_names.append( "tpat" );
	std_names.append( "in_Z" );
	std_names.append( "in_A_on_Z" );
	Array< octave_idx_type > unwanted = fld_names.lookup( std_names );
	fld_names.clear( unwanted );
	const int nf = fld_names.numel();
	
	//now, we are good to go
	for( int i=0; i < o_data_m.length(); ++i ){
		o_map = o_data_m(i);
		
		//copy the standart things
		buf.n = o_map.getfield( "n" ).uint_value();
		buf.evnt = o_map.getfield( "evnt" ).uint_value();
		buf.tpat = o_map.getfield( "tpat" ).uint_value();
		buf.in_A = o_map.getfield( "in_A" ).float_value();
		buf.in_A_on_Z = o_map.getfield( "in_A_on_Z" ).float_value();
		
		//and now the loop-copy of the named fields
		for( int f=0; f < nf; ++f )
			buf.dofield( fld_names(f).c_str(),
			             buf.n*sizeof(float),
			             o_map.getfield( fld_names(f).float_array_value().fortran_vec() );
		
		data.push_back( buf );
		o_data_m(i).clear;
	}
	
	o_data_m.clear();
	
	//write on file
	char out_fname[256];
	strcpy( out_fname, args(0).string_value().c_str() );
	try{
		XB::write( out_fname, data );
	} catch( XB::error e ){
		error( e.what() );
	}
	
	//more cleanup
	data.clear();
	
	//happy thoughts
	return octave_value_list();
}
	
