//This GNU Octave function provides an interface to write data
//to files readable with XB::read function.
//NOTE: there's no guarantee whatsoever that this function
//      would be at all usable with MATLAB in a MEX file.
//      It will *not* be tested and it is *not* ment as such.

//octave documentation string
#define O_DOC_STRING "-*- texinfo -*-\n\
@deftypefn{Function File} xb_write_clusterZ( @var{filename}, @var{clusterZ} )\n\
Writes a file named @var{filename} readable with the programs of the xb_progs toolkit.\n\
\n\
The file is created and overwritten if existing: no warning messages are displayed.\n\
\n\
The format of @var{clusterZ} should be the same-ish used in the xb_progs toolkit:\n\
@example\n\
@group\n\
structure array data:\n\
    n\n\
    evnt\n\
    tpat\n\
    in_Z\n\
    in_A_on_Z\n\
    in_beta\n\
     structure array clusters:\n\
         n\n\
         centroid_id\n\
         c_altitude\n\
         c_azimuth\n\
         sum_e\n\
         array crys_e\n\
         array crys\n\
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

//includes from the toolkit
#include "xb_io.h" //XB::load
#include "xb_data.h" //XB::data
#include "xb_error.h" //XB::error

//a function to convert from scalar maps to versors
std::vector<XB::cluster> struct2cluster( const octave_map &given );

DEFUN_DLD( xb_write_clusterZ, args, , O_DOC_STRING ){
	if( sizeof(octave_uint32) != sizeof(unsigned int) ){
		error( "Quirky types." );
	}

	//check that there are two arguments and
	//that the first one is a string
	if( args.length() != 2 || !args(0).is_string() ){
		error( "xb_wite_clusterZ: need a filename and an array of structures.\n" );
		return octave_value_list();
	}
	
	//retrieve the array of structures
	//and check that we have got it.
	octave_map o_data_m = args(1).map_value();
	/*if( !o_data_m.is_zero_by_zero() ){
		error( "xb_data_write: invalid argument\n" );
		return octave_value_list();
	}*/
	
	std::vector<XB::clusterZ> data;
	
	//if we got here, we should be able to proceed.
	//declare the necessary bits and pieces
	unsigned int current_numel = 0;
	XB::clusterZ buf;
	octave_scalar_map o_map;
	
	//loop-copy the data
	for( int i=0; i < o_data_m.length(); ++i ){
		o_map = o_data_m(i);
		
		buf.n = o_map.getfield( "n" ).uint_value();
		buf.evnt = o_map.getfield( "evnt" ).uint_value();
		buf.tpat = o_map.getfield( "tpat" ).uint_value();
		buf.in_Z = o_map.getfield( "in_Z" ).float_value();
		buf.in_A_on_Z = o_map.getfield( "in_A_on_Z" ).float_value();
		buf.in_beta = o_map.getfield( "in_beta" ).float_value();
		buf.clusters = struct2cluster( o_map.getfield( "clusters" ).map_value() );		
		
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
		error( e.what() );
	}
	
	//happy thoughts
	return octave_value_list();
}

//implementation of the helper function for struct to versor tranlation
std::vector<XB::cluster> struct2cluster( const octave_map &given ){
	//some buffers
	octave_scalar_map m_buf;
	XB::cluster c_buf;
	
	//make the cluster array
	std::vector<XB::cluster> klZ( given.length() );
	
	unsigned int current_numel;
	for( int i=0; i < given.length(); ++i ){
		m_buf = given(i); //get the i-th map
		
		//copy the easy ones
		if( m_buf.isfield( "n" ) ) current_numel = m_buf.getfield( "n" ).uint_value();
		klZ[i].n = current_numel;
		if( m_buf.isfield( "centroid_id" ) )
			klZ[i].centroid_id = m_buf.getfield( "centroid_id" ).int_value();
		if( m_buf.isfield( "c_altitude" ) )
			klZ[i].c_altitude = m_buf.getfield( "c_altitude" ).float_value();
		if( m_buf.isfield( "c_azimuth" ) )
			klZ[i].c_azimuth = m_buf.getfield( "c_azimuth" ).float_value();
		if( m_buf.isfield( "sum_e" ) )
			klZ[i].sum_e = m_buf.getfield( "sum_e" ).float_value();
		
		//vector copies
		if( m_buf.isfield( "crys_e" ) ){
			klZ[i].crys_e.resize( current_numel );
			memcpy( &klZ[i].crys_e[0],
			        m_buf.getfield( "crys_e" ).float_array_value().fortran_vec(), 
			        current_numel*sizeof(float) );
		}
		if( m_buf.isfield( "crys" ) ){
			klZ[i].crys.resize( current_numel );
			memcpy( &klZ[i].crys[0],
			        m_buf.getfield( "crys" ).uint32_array_value().fortran_vec(), 
			        current_numel*sizeof(unsigned int) );
		}
	}
	
	return klZ;
}
