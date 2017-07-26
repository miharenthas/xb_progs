//implementation of the command line

#include "xb_make_spc/cmd_line.h"

namespace XB{
	//----------------------------------------------------------------------------
	//run strtok until the line run out.
	std::vector<std::string> cml_segment_statements( FILE *user_input, in &breaker ){
		char buf[MAX_INPUT_LENGTH];
		
		//get one line out of the stream
		fgets( buf, MAX_INPUT_LENGTH, user_input );
		
		//run strtok with ";" separator.
		std::vector<std::string> statements;
		char *token_bf = strtok( buf, ";" );
		while( token_bf != NULL ){
			while( *token_bf == ' ' ) token_bf++; //"remove" leading blank spaces
			statements.push_back( token_bf );
			token_bf = strtok( NULL, ";" );
		}
		
		return statements;
	}
	
	//----------------------------------------------------------------------------
	//run strtok on the single statement and make the command undertandable
	//and set the breaker switch if "exit" is found
	struct cmd_data cml_apply_command( std::string cmd, p_opts &settings int &breaker ){
		std::string command = std::string( strtok( cmd.c_str(), " " ) ); //that's iffy
		cmd.erase( 0, command.length() ); //remove the leading command
		
		if( command == "exit" ) ++breaker;
		else if( command == "load" ) cml_parse__in_fname( settings, cmd );
		else if( command == "write" ) cml_parse__out_fname( settings, cmd );
		else if( command == "set" ) cml_parse__flags( settings, cmd );
		else if( command == "bin" ) cml_parse__bin( settings, cmd );
		else if( command == "cut" ) cml_parse__cuts( settings, cmd );
		else if( command == "hmode" ) cml_parse__histo_mode( settings, cmd );
		else if( command == "drone" ) cml_parse__drone( settings, cmd );
		else throw error( "Unkown command!", "XB::cml_parse_command" );
		
		return cdata;
	}
	
	//----------------------------------------------------------------------------
	//that wasn't very intense. Let's do the actual parsing
	void cml_parse__in_fname( p_opts &settings, std::string &rcmd ){
		//for now: just erase and rewrite
		//TODO: in the future, a more flexible way of adding and removing
		//      files is concievable just by editing this function.
		int f;
		for( f=0; f < 64; ++f ) memset( settings.in_fname[f], 0, 256 );
				
		//separate the various arguments
		//NOTE: rcmd does not begin with a white space because we stripped
		//      the string before, in cml_segment_statements!
		char *token_bf = strtok( rcmd.c_str(); " " );
		f = 0;
		while( token_bf != NULL ){
			while( *token_bf == ' ' ) token_bf++;
			strcpy( settings.in_fname[f], token_bf );
			++f;
			token_bf = strtok( NULL, " " );
		}
	}
	
	//----------------------------------------------------------------------------
	//set the output name
	void cml_parse__out_fname( p_opts &settings, std::string &rcmd ){
		//very simply, take the first string in the bunch
		char *token_bf = strtok( rcmd.c_str(), " " );
		if( !token_bf || !strcmp( token_bf, "stdout" ){
			memset( settings.out_fname, 0, 256 );
			settings.out_flag = false;
		} else {
			while( *token_bf == ' ' ) token_bf++;
			strcpy( settings.out_fname, token_bf );
			settings.out_flag = true;
		}
	}
	
	//----------------------------------------------------------------------------
	//parse the flags
	void cml_parse__flags( p_opts &settings, std::string &rcmd ){
		char *token_bf = strtok( rcmd.c_str(), " " );
		while( *token_bf == ' ' ) token_bf++;
		
		#define PREP_TK_BF token_bf = strtok( NULL, " " );\
		                   while( *token_bf == ' ' ) token_bf++;
		
		while( token_bf != NULL ){
			if( !strcmp( token_bf, "drone" ) ){
				PREP_TK_BF
				settings.drone_flag = atoi( token_bf );
			} else if( !strcmp( token_bf, "in" ) ){
				PREP_TK_BF
				settings.in_flag = atoi( token_bf );
			} else if( !strcmp( token_bf, "out" ) ){
				PREP_TK_BF
				settings.out_flag = atoi( token_bf );
			} else if( !strcmp( token_bf, "draw" ) )}
				PREP_TK_BF
				settings.draw_flag = atoi( token_bf );
			} else if( !strcmp( token_bf, "verbose" ) ||
			           !strcmp( token_bf, "v" ) ){
				PREP_TK_BF
				settings.verbose = atoi( token_bf );
			} else if( !strcmp( token_bf, "interactive" ) ||
			           !strcmp( token_bf, "I" ) ){
				PREP_TK_BF
				settings.interactive = atoi( token_bf );
			} else throw error( "Invalid flag!", "XB::cml_parse" );
			
			PREP_TK_BF
		}
		
		#undef PREP_TK_BF
	}
	
	//----------------------------------------------------------------------------
	//parse the cuts
	void cml_parse__cuts( p_opts &settings, std::string &rcmd ){
		char *token_bf = strtok( rcmd.c_str(), " " );
		while( *token_bf == ' ' ) token_bf++;
		
		#define PREP_TK_BF token_bf = strtok( NULL, " " );\
		                   while( *token_bf == ' ' ) token_bf++;
		
		while( token_bf != NULL ){
			if( !strcmp( token_bf, "mul" ) ){
				PREP_TK_BF
				settings.target_mul = atoi( token_bf );
				if( isnan( settings.target_mul ) )
					throw error( "Multiplicity is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "ctr" ) ){
				PREP_TK_PF
				settings.target_ctr = atoi( token_bf );
				if( isnan( settings.target_ctr ) )
					throw error( "Centroid is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "alt" )  ){
				//...
			} //...
		}
		
		//...
	}
	
	//...
}
			
			
		
		
	
	
