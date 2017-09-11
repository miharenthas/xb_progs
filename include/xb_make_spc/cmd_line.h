//this header file defines the functions of the command line/drone controls
//for xb_make_spc

#ifndef XB_MAKE_SPC__CMD_LINE__H
#define XB_MAKE_SPC__CMD_LINE__H

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>

#include "xb_error.h"
#include "xb_draw_gsl_histogram.h"
#include "xb_make_spc/xb_make_spc.h"
#include "xb_make_spc/selectors.h"

#define MAX_INPUT_LENGTH 4096 //number of bytes readable per line of input

namespace XB{
	
	//----------------------------------------------------------------------------
	//worker functions:
	//segment different commands in a single string into multiple strings
	//reads directly from the selected stream.
	std::vector<std::string> cml_segment_statements( FILE* user_input );
	//parse a string into a command and its arguments, and apply to settings
	//NOTE: comments beginning with # are supported!
	//NOTE: also, scripting (and live scripting) is supported!
	std::string cml_parse( std::string cmd, p_opts& settings, unsigned short int &breaker );
	//loop apply the settings.
	unsigned short int cml_loop( FILE* user_input, p_opts &settings );
	unsigned short int cml_loop_prompt( FILE* user_input, p_opts &settings );
	
	//----------------------------------------------------------------------------
	//specialized parsers:
	//the function parses the part of the settings indicated in the name
	//__flags are all the members with the _flag suffix plus "verbose, interactive"
	//__cuts are all the member with the target_ prefix plus range
	void cml_parse__save( unsigned short int &breaker, std::string &rcmd );
	void cml_parse__put( unsigned short int &breaker, std::string &rcmd );
	void cml_parse__script( p_opts &settings, std::string &rcmd,
	                        unsigned short int &breaker );
	void cml_parse__in_fname( p_opts &settings, std::string &rcmd );
	void cml_parse__out_fname( p_opts &settings, std::string &rcmd );
	void cml_parse__flags( p_opts &settings, std::string &rcmd );
	void cml_parse__bin( p_opts &settings, std::string &rcmd );
	void cml_parse__cuts( p_opts &settings, std::string &rcmd );
	void cml_parse__gp_opts( p_opts &settings, std::string &rcmd );
	void cml_parse__histo_mode( p_opts &settings, std::string &rcmd );
	void cml_parse__drone( p_opts &settings, std::string &rcmd );

	//----------------------------------------------------------------------------
	//an utility
	char *get_c_str( const std::string &str );
}

#endif
	


