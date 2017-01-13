//This GNU Octave function provides an interface to load data
//from files created with the XB toolkit.
//The data must have been generated from the struct XB::clusterZ.
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
#include <octave/file-stat.h> //file_stat

//includes from the toolkit
#include "xb_io.h" //XB::load
#include "xb_cluster.h" //XB::cluster, XB::clusterZ
#include "xb_error.h" //XB::error

//a helper function that does a XB::versor to struct converison.
octave_map cluster2struct( XB::clusterZ &given );

//------------------------------------------------------------------------------------
//the interface
DEFUN_DLD( xb_load_clusterZ, args, nargout, "XB::load data interface for Octave" ){
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
	std::vector<XB::clusterZ> data, data_buf;
	char in_fname[256];
	for( int f=0; f < nargin; ++f ){
		if( args(f).is_string() ){ //if the argument is a string
		                           //attempt to load the file
			octave::sys::file_stat fs( args(f).string_value() );
			if( fs.exists() ){
				strcpy( in_fname, args(f).string_value().c_str() );
				try{
					XB::load( in_fname, data_buf );
				} catch( XB::error e ){
					error( e.what );
				}
				data.insert( data.end(), data_buf.begin(), data_buf.end() );
			} else {
				octave_stdout << "xb_load_clusterZ: warning: file \""
				              << args(f).string_value() << "\" doesn't exist.\n";
				continue;
			}
		} else {
			octave_stdout << "xb_load_clusterZ: warning: argument "
			              << f << " is not a valid filename.\n";
			continue;
		}
	}
	//check that something has been read
	if( !data.size() ){
		octave_stdout << "xb_load_clusterZ: warning: no data loaded.\n";
		return octave_value_list();
	}
	
	//now, begin the translation into octave structure
	//first, allocate the octave_map that will hold the thing
	dim_vector o_dim_v( data.size(), 1 ), o_dim_null( 0, 0 ); 
	
	//copy the data:
	//prepare the fields:
	Cell o_field_multiplicity( o_dim_v );
	Cell o_field_clusters( o_dim_v );
	
	for( int i=0; i < data.size(); ++i ){
		//copy the number of clusters at event i
		o_field_multiplicity(i) = data[i].multiplicity;
		
		//load the clusters
		o_field_clusters(i) = cluster2struct( data[i] );
	}
	
	//make the map
	octave_map o_data_m;
	o_data_m.setfield( "multiplicity", o_field_multiplicity );
	o_data_m.setfield( "clusters", o_field_clusters );
	
	//happy thoughts
	return octave_value_list( octave_value( o_data_m ) );
}

//------------------------------------------------------------------------------------
//implementation of the helper function
octave_map cluster2struct( XB::clusterZ &given ){
	//dimensions
	dim_vector o_dim_v( given.multiplicity, 1 );
	
	//instantiate the fields
	Cell o_field_n( o_dim_v );
	Cell o_field_centroid_id( o_dim_v );
	Cell o_field_c_altitude( o_dim_v );
	Cell o_field_c_azimuth( o_dim_v );
	Cell o_field_sum_e( o_dim_v );
	Cell o_field_crys_e( o_dim_v );
	Cell o_field_crys( o_dim_v );
	
	//and some buffers
	Array<float> f_buf;
	Array<octave_uint32> u_buf;
	
	//and an alias
	std::vector<XB::cluster> &klZ = given.clusters;
	
	unsigned int current_numel = 0;
	for( int i=0; i < klZ.size(); ++i ){
		//number of elements
		current_numel = klZ[i].n;
		
		//copy the trivially copiable
		o_field_n(i) = klZ[i].n;
		o_field_centroid_id(i) = klZ[i].centroid_id;
		o_field_c_altitude(i) = klZ[i].c_altitude;
		o_field_c_azimuth(i) = klZ[i].c_azimuth;
		o_field_sum_e(i) = klZ[i].sum_e;
		
		//prepare the vector buffers
		dim_vector o_dim( current_numel, 1 );
		f_buf.resize( o_dim );
		u_buf.resize( o_dim );
		
		//copy the vector bufers
		if( current_numel ){
			memcpy( f_buf.fortran_vec(), &klZ[i].crys_e[0],
			        current_numel*sizeof(float) );
			memcpy( u_buf.fortran_vec(), &klZ[i].crys[0],
			        current_numel*sizeof(unsigned int) );
		}
		
		o_field_crys_e(i) = f_buf;
		o_field_crys(i) = u_buf;
		
		//cleanup
		f_buf.clear();
		u_buf.clear();
	} 
	
	//make the map
	octave_map o_map;
	o_map.setfield( "n", o_field_n );
	o_map.setfield( "centroid_id", o_field_centroid_id );
	o_map.setfield( "c_altitude", o_field_c_altitude );
	o_map.setfield( "c_azimuth", o_field_c_azimuth );
	o_map.setfield( "sum_e", o_field_sum_e );
	o_map.setfield( "crys_e", o_field_crys_e );
	o_map.setfield( "crys", o_field_crys );
		
	return o_map;
}
