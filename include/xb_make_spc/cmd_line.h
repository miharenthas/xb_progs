//this header file defines the functions of the command line/drone controls
//for xb_make_spc

#ifndef XB_MAKE_SPC__CMD_LINE__H
#define XB_MAKE_SPC__CMD_LINE__H

#include <string.h>
#include <string>
#include <vector>
#include <map>

#include "xb_error.h"
#include "include/xb_make_spc.h"

#define MAX_INPUT_LENGTH 4096 //number of bytes readable per line of input

namespace XB{
	
	//----------------------------------------------------------------------------
	//looper functions (?)
	void cml_loop(); //without a prompt
	void cml_loop_prompt(); //with a prompt

	//----------------------------------------------------------------------------
	//worker functions:
	//segment different commands in a single string into multiple strings
	//reads directly from the selected stream.
	std::vector<std::string> cml_segment_statements( FILE* user_input );
	//parse a string into a command and its arguments, and apply to settings
	void cml_apply_command( std::string cmd, p_opts& settings, int &breaker );
	//enact the settings
	void cml_enact( xb_make_spc &prog, p_opts &settings );
	
	//----------------------------------------------------------------------------
	//specialized parsers:
	//the function parses the part of the settings indicated in the name
	//__flags are all the members with the _flag suffix plus "verbose, interactive"
	//__cuts are all the member with the target_ prefix plus range
	void cml_parse__in_fname( p_opts &settings, std::string &rcmd );
	void cml_parse__out_fname( p_opts &settings, std::string &rcmd );
	void cml_parse__flags( p_opts &settings, std::string &rcmd );
	void cml_parse__bin( p_opts &settings, std::string &rcmd );
	void cml_parse__cuts( p_opts &settings, std::string &rcmd );
	void cml_parse__gp_opts( p_opts &settings, std::string &rcmd );
	void cml_parse__histo_mode( p_opts &settings, std::string &rcmd );
	void cml_parse__drone( p_opts &settings, std::string &rcmd );
	
	//----------------------------------------------------------------------------
	//memory management (very useful since we're having voids)
	template< class TYPE >
	int cmd_data cmd_data_alloc( struct cmd_data &command, int narg );
	void cmd_data_free( struct cmd_data &command );
}

#endif
	


