//this is the implementation for xb_error
#include "xb_error.h"

namespace XB{

	//error class implementation
	error::_xb_error( const char *message, const char* from ){
		what = new char[strlen( message ) + strlen( from ) + 1];
		strcpy( what, message );
		strcat( what, from );
	}
}
