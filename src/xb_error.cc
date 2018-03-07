//this is the implementation for xb_error
#include "xb_error.h"

namespace XB{

	//error class implementation
	error::_xb_error( const char *message, const char* from ){
		_what = new char[strlen( message ) + strlen( from ) + 1];
		strcpy( _what, message );
		strcat( _what, from );
	}

	error::_xb_error( const _xb_error &given ){
		_what = new char[strlen( given._what )+1];
		strcpy( _what, given._what );
	}

	const char *error::what() const noexcept {
		return _what;
	}
}	
