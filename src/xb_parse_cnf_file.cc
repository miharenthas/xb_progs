//implementation of the parser for the cut configuration

#include "xb_parse_cnf_file.h"

//ugly macros
#define __CHECK_2D if( !cnf.is_2D ){ \
						fprintf( stderr, "Inconsistent configuration: only 2D cuts have --%s.\n", \
						         parse_options[opt_index].name ); \
						exit( 1 ); \
					}

#define __CHECK_1D if( !cnf.is_1D ){ \
						fprintf( stderr, "Inconsistent configuration: only SEGMENT has --%s.\n", \
						         parse_options[opt_index].name ); \
						exit( 1 ); \
					}


namespace XB{
	//----------------------------------------------------------------------------------
	//config string parsers:
	
	//split the string before passing the options
	cut_cnf parse_config_opts( char *the_str ){
		//fisrt, split on space and count the substrings
		int str_count;
		char *str_vec[512]; //right now, I think 512 lines per config are enough...
		
		//do the splitting
		char *strbuf = strtok( the_str, " " );
		for( str_count=0; strbuf != NULL && str_count < 512; ++str_count ){
			str_vec[str_count] = (char*)malloc( strlen( strbuf )+1 );
			strcpy( str_vec[str_count], strbuf );
			strbuf = strtok( NULL, " " );
		}
		
		return parse_config_opts( str_count, str_vec ); 
	}
	
	//directly parse the options
	cut_cnf parse_config_opts( unsigned int str_count, char **str_vec ){ 
		//and now, all the options
		struct option parse_options[] = {
			{ "cut-type", required_argument, NULL, 0 },
			{ "cut-segment-beign", required_argument, NULL, 1 },
			{ "cut-segment-end", required_argument, NULL, 2 },
			{ "cut-centroid", required_argument, NULL, 3 },
			{ "cut-rotation", required_argument, NULL, 4 },
			{ "cut-square-side", required_argument, NULL, 5 },
			{ "cut-circle-radius", required_argument, NULL, 6 },
			{ "cut-ellipse-major-saxis", required_argument, NULL, 7 },
			{ "cut-ellipse-minor-saxis", required_argument, NULL, 8 },
			{ "cut-regular-polygon-nb-sides", required_argument, NULL, 9 },
			{ "cut-regular-polygon-radius", required_argument, NULL, 10 },
			{ "cut-polygon-nb-sides", required_argument, NULL, 11 },
			{ "cut-polygon-vertex", required_argument, NULL, 12 },
			{ "cut-transformation", required_argument, NULL, 13 },
			{ 0, 0, 0, 999 }
		};
		
		//some declarations:
		//the config struct, the loop switch and all the variables that will be relevant.
		cut_cnf cnf = { false, false, CUT_NOTHING, NULL, NULL };
		int iota = 0, opt_index, vertex_index;
		double centroid[2], vertex[2], matrix[9], saxis[2];
		double begin = 0, end = 0, radius = 0, rotation = 0, side = 0;
		int nb_sides = 0;
		double *vertices = NULL;
		gsl_matrix *transformation = gsl_matrix_alloc( 3, 3 );
		gsl_matrix_set_identity( transformation );
		
		//IMPORTANT: reset optind to -1, in order to parse correctly
		optind = -1; 
		while( ( iota = getopt_long( str_count, str_vec, "",
		         parse_options, &opt_index ) ) != -1 ){
			switch( iota ){
				case 0 : //cut-type
					if( !strcmp( optarg, "SEGMENT" ) ){ cnf.is_1D = true; cnf.is_2D = false; }
					else{
						cnf.is_1D = false;
						cnf.is_2D = true;
						if( !strcmp( optarg, "SQUARE" ) ) cnf.cut_type = CUT_SQUARE;
						else if( !strcmp( optarg, "CIRCLE" ) )  cnf.cut_type = CUT_CIRCLE;
						else if( !strcmp( optarg, "ELLIPSE" ) ) cnf.cut_type = CUT_ELLIPSE;
						else if( !strcmp( optarg, "REGULAR_POLYGON" ) ) cnf.cut_type = CUT_REGULAR_POLYGON;
						else if( !strcmp( optarg, "POLYGON" ) ) cnf.cut_type = CUT_POLYGON;
						else cnf.cut_type = CUT_NOTHING;
					}
					break;
				case 1 : //cut-segment-begin
					__CHECK_1D
					
					begin = atof( optarg );
					break;
				case 2 : //cut-segment-end
					__CHECK_1D
					
					end = atof( optarg );
					break;
				case 3 : //cut-centroid
					__CHECK_2D
					
					sscanf( optarg, "(%lf,%lf)", &centroid[0], &centroid[1] );
					break;
				case 4 : //cut-rotation
					__CHECK_2D
					
					rotation = atof( optarg );
					break;
				case 5 : //cut-square-side
					__CHECK_2D
					
					side = atof( optarg );
					break;
				case 6 : case 10 : //cut-circle-radius and cut-regular-polygon-radius
					__CHECK_2D
					
					radius = atof( optarg );
					break;
				case 7 : //cut-ellipse-major-saxis
					__CHECK_2D
					
					saxis[0] = atof( optarg );
					break;
				case 8 : //cut-ellipse-minor-saxis
					__CHECK_2D
					
					saxis[1] = atof( optarg );
					break;
				case 9 : case 11 : //cut-(regular)-polygon-nb-sides
					__CHECK_2D
					
					nb_sides = atoi( optarg );
					if( cnf.cut_type == CUT_POLYGON ){
						vertices = (double*)malloc( 2*nb_sides*sizeof(double) );
					}
					break;
				case 12 : //cut-polygon-vertex
					__CHECK_2D
					
					if( vertices == NULL ){
						fprintf( stderr, "Number of vertices ***MUST*** be specified before giving vertices!\n" );
						exit( 1 );
					}
					
					sscanf( optarg, "v%d(%lf,%lf)", vertex_index, vertex[0], vertex[1] );
					memcpy( vertices + 2*vertex_index, vertex, 2*sizeof(double) );
					break;
				case 13 : //cut-transformation
					__CHECK_2D
					
					sscanf( optarg, "[%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf]",
					        &matrix[0], &matrix[1], &matrix[2],
					        &matrix[3], &matrix[4], &matrix[5],
					        &matrix[6], &matrix[7], &matrix[8] );
					memcpy( transformation->data, matrix, 9*sizeof(double) );
					break;
			} //switch
		} //while
		
		//now, create the cut
		if( cnf.is_1D ){ //1D case
			cnf.cut_1D = new cut_segment( begin, end );
			return cnf;
		}
		
		switch( cnf.cut_type ){
			case CUT_SQUARE :
				cnf.cut_2D = (_xb_2D_cut_primitive*)new cut_square( centroid, side, rotation );
				break;
			case CUT_CIRCLE :
				cnf.cut_2D = (_xb_2D_cut_primitive*)new cut_circle( centroid, radius );
				break;
			case CUT_ELLIPSE :
				cnf.cut_2D = (_xb_2D_cut_primitive*)new cut_ellipse( centroid, saxis, rotation );
				break;
			case CUT_POLYGON :
				cnf.cut_2D = (_xb_2D_cut_primitive*)new cut_polygon( centroid, vertices, nb_sides );
				break;
			case CUT_REGULAR_POLYGON :
				cnf.cut_2D = (_xb_2D_cut_primitive*)new cut_regular_polygon( centroid, radius,
					                                                           nb_sides, rotation );
				break;
		}
		cnf.cut_2D->transform( transformation );
		
		return cnf;
	}
	
	//----------------------------------------------------------------------------------
	//parse many configurations
	void parse_config( char **the_str, cut_cnf *cnfs ){
		//retrieve the information
		unsigned int *size = (unsigned int*)cnfs - 1; //set the pointer one unsigned int
		                                              //before the beginning of the buffer
		for( int i=0; i < *size; ++i ){
			cnfs[i] = parse_config_opts( the_str[i] );
		}
	}
	
	//----------------------------------------------------------------------------------
	//allocate an array of cut configs
	cut_cnf *config_alloc( unsigned int howmany ){
		void *cnfs = malloc( howmany*sizeof(cut_cnf) + sizeof(unsigned int) ); //allocate the buffer
		
		if( cnfs == NULL ){
			fprintf( stderr, "XB::config_alloc: memory error.\n" );
			exit( 1 );
		}
		
		unsigned int *size = (unsigned int*)cnfs; //size points to the beginning of the buffer
		*size = howmany; //size has been written there
		
		cnfs = (unsigned int*)cnfs + 1; //shift the pointer cnfs of one unsigned int
		
		return (cut_cnf*)cnfs;
	}
	
	//----------------------------------------------------------------------------------
	//deallocate everything
	//which includes the actual cuts!
	void config_free( cut_cnf *cnfs ){
		//retireve the size
		unsigned int *size = (unsigned int*)cnfs - 1;
		
		//loop deallocate the cuts
		for( int i=0; i < *size; ++i ){
			if( cnfs[i].is_1D ) delete cnfs[i].cut_1D;
			if( cnfs[i].is_2D ) delete cnfs[i].cut_2D;
		}
		
		cnfs = NULL; //detach the pointer
		free( size ); //free the buffer (size is the beginning)
	}
} //namespace
