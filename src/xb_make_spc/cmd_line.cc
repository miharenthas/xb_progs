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
	std::string cml_parse( std::string cmd, p_opts &settings int &breaker ){
		std::string command = std::string( strtok( cmd.c_str(), " " ) ); //that's iffy
		cmd.erase( 0, command.length() ); //remove the leading command
		
		if( command[0] == '#' ); //it's a comment, do nothing.
		else if( command == "exit" || command == "quit" ) breaker = DO_EXIT;
		else if( command == "exec" || command == "go" ) breaker = DO_EXECUTE;
		else if( command == "return" ) breaker = DO_RETURN;
		else if( command == "script" ) cml_parse__script( settings, cmd, breaker ); 
		else if( command == "load" ) cml_parse__in_fname( settings, cmd );
		else if( command == "write" ) cml_parse__out_fname( settings, cmd );
		else if( command == "set" ) cml_parse__flags( settings, cmd );
		else if( command == "bin" ) cml_parse__bin( settings, cmd );
		else if( command == "cut" ) cml_parse__cuts( settings, cmd );
		else if( command == "hmode" ) cml_parse__histo_mode( settings, cmd );
		else if( command == "drone" ) cml_parse__drone( settings, cmd );
		else return cmd;
		
		return std::string();
	}
	
	//----------------------------------------------------------------------------
	//parse a script
	//NOTE: this is essentially recursive, a "script" command in a script will
	//      open a script and so on until explosion or successfull execution.
	void cml_parse__script( p_opts &settings, std::string &rcmd, int &breaker ){
		char *token_bf = strtok( rcmd.c_str(), " " );
		while( *token_bf == ' ' ) ++token_bf;
		
		FILE *script = fopen( token_bf, "r" );
		breaker = cml_loop( script, settings );
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
			if( *token_bf == '#' ) return;
			
			strcpy( settings.in_fname[f], token_bf );
			++f;
			token_bf = strtok( NULL, " " );
		}
		settings.in_f_count = f;
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
		if( *token_bf == '#' ) return;
		
		#define PREP_TK_BF token_bf = strtok( NULL, " " );\
		                   while( *token_bf == ' ' ) token_bf++;\
		                   if( *token_bf == '#' ) return;
		
		while( token_bf != NULL ){
			if( !strcmp( token_bf, "drone" ) ){
				throw error( "Can't touch drone!", "XB::parse" );
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
		if( *token_bf == '#' ) return;
		
		#define PREP_TK_BF token_bf = strtok( NULL, " " );\
		                   while( *token_bf == ' ' ) token_bf++;\
		                   if( *token_bf == '#' ) return;
		#define CK_MOL( settage ) if( token_bf[0] == '<' ){\
		                              (settage) = LESS;\
		                              token_bf = strtok( NULL, " " );\
		                              while( *token_bf == ' ' ) token_bf++;\
		                              if( *token_bf == '#' ) return;\
		                          } else if( token_bf[0] == '>' ){\
		                              (settage) = MORE;\
		                              token_bf = strtok( NULL, " " );\
		                              while( *token_bf == ' ' ) token_bf++;\
		                              if( *token_bf == '#' ) return;\
		                          }
		
		while( token_bf != NULL ){
			if( !strcmp( token_bf, "mul" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_mul )
				settings.target_mul = atoi( token_bf );
				if( isnan( settings.target_mul ) )
					throw error( "Multiplicity is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "cry" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_cry )
				settings.target_ctr = atoi( token_bf );
				if( isnan( settings.target_cry ) )
					throw error( "Crystal is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "ctr" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_ctr )
				settings.target_ctr = atoi( token_bf );
				if( isnan( settings.target_ctr ) )
					throw error( "Centroid is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "alt" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_alt )
				settings.target_alt = atof( token_bf );
				if( isnan( settings.target_alt ) )
					throw error( "Altitude is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "azi" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_azi )
				settings.target_azi = atof( token_bf );
				if( isnan( settings.target_azi ) )
					throw error( "Azimuth is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "nrg" ) ){
				PREP_TK_BF
				CK_MOL( settings.mol_nrg )
				settings.target_nrg = atof( token_bf );
				if( isnan( settings.target_nrg ) )
					throw error( "Energy is NaN!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "rng" ) ){
				PREP_TK_BF
				sscanf( token_bf, "%f:%f", &settings.range[0],
				        &settings.range[1] );
				if( isnan( settings.range[0] ) || isnan( settings.range[1] ) )
					throw error( "Range(s) is NaN!", "XB::cml_parse" );
			} else throw error( "Invalid cut!", "XB::cml_parse" );
			
			PREP_TK_BF
		}
		#undef PREP_TK_BF
		#undef CK_MOL		
	}
	
	//----------------------------------------------------------------------------
	//parse the terminal options (boring)
	void cml_parse__gp_opts( p_opts &settings, std::string &rcmd ){
		char *token_bf = strtok( rcmd.c_str(), " " );
		while( *token_bf == ' ' ) token_bf++;
		if( *token_bf == '#' ) return;
		
		#define PREP_TK_BF token_bf = strtok( NULL, " " );\
		                   while( *token_bf == ' ' ) token_bf++;\
		                   if( *token_bf == '#' ) return;
		
		while( token_bf != NULL ){
			if( !strcmp( token_bf, "term" ) ){
				PREP_TK_BF
				if( !strcmp( token_bf, "qt" ) ) settings.gp_opt.term = QT;
				if( !strcmp( token_bf, "png" ) ) setting.gp_opt.term = PNG;
			} else if( !strcmp( token_bf, "log" ) ) settings.gp_opt.is_log = true;
			else if( !strcmp( token_bf, "lin" ) ) settings.gp_opt.is_log = false;
			else if( !strcmp( token_bf, "title" ) ){
				PREP_TK_BF
				if( strlen( token_bf ) < 128 )
					strcpy( settings.gp_opt.title, token_bf );
				else throw error( "Title too long!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "xlabel" ) ){
				PREP_TK_BF
				if( strlen( token_bf ) < 32 )
					strcpy( settings.gp_opt.x_label, token_bf );
				else throw error( "Xlabel too long!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "ylabel" ) ){
				PREP_TK_BF
				if( strlen( token_bf ) < 32 )
					strcpy( settings.gp_opt.y_label, token_bf );
				else throw error( "Ylabel too long!", "XB::cml_parse" );
			} else if( !strcmp( token_bf, "fout" ) ){
				PREP_TK_BF
				if( strlen( token_bf ) < 256 )
					strcpy( settings.gp_opt.out_fname, token_bf );
				else throw error( "File name too long!", "XB::cml_parse" );
			} else throw error( "Invalid hmode!", "XB::cml_parse" );
			
			PREP_TK_BF
		}
		#undef PREP_TK_BF
	}
	
	//----------------------------------------------------------------------------
	//set the drone
	//NOTE: this is vulnerable to overflow attacks
	//      but ideally nobody should care.
	void cml_parse__drone( p_opts &settings, std::string &rcmd ){
		if( settings.drone_flag == false ) return;
		
		rcmd.erase( rcmd.find( '#' ), rcmd.length() ); //do away with comments.
		sscanf( rcmd.c_str(), "%c:%s::%c:%s",
		        &settings.drone.in_pof,
		        &settings.drone.in,
		        &settings.drone.out_pof,
		        &settings.drone.out );
		settings.drone.in = ( settings.drone.in_pof == 'p' )?
		                    popen( settings.drone.instream, "r" ) :
		                    fopen( settings.drone.instream, "r" );
		settings.drone.out = ( settings.drone.out_pof == 'p' )?
		                     popen( settings.drone.outstream, "w" ) :
		                     fopen( settings.drone.outstream, "w" );
		                     
		//if the drone is in some way broken, die immediately with the correct answer.
		if( settings.drone.in == NULL || settings.drone.out == NULL ){
			fprintf( "stderr", "FATAL: drone throughtput is broken.\n" );
			exit( 42 );
		}
	}
	
	//----------------------------------------------------------------------------
	//the command loop
	int cml_loop( FILE *user_input, p_opts &settings ){
		int breaker = 0;
		std::vector<std::string> commands;
		while( !breaker ){
			commands = cml_segment_statements( user_input );
			for( int c=0; c < commands.size(); ++c )
				cml_parse_settings( commands[c], settings, breaker );
		}
		
		return breaker;
	}
	
	int cml_loop_prompt( FILE *user_input, p_opts &settings ){
		int breaker = 0;
		std::vector<std::string> commands;
		while( !breaker ){
			if( user_input == stdin ) printf( "xb> " );
			commands = cml_segment_statements( user_input );
			for( int c=0; c < commands.size(); ++c )
				cml_parse_settings( commands[c], settings, breaker );
		}
		
		return breaker;
	}	
				
}
