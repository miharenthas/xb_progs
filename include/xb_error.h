//this is the error class declaration for the xb_explorer suite
#ifndef XB_ERROR__H
#define XB_ERROR__H

#include <string.h>

namespace XB{
	//an exception structure
	typedef class _xb_error{
		public:
			_xb_error( const char* message, const char* from );
			char *what;
	} error;
}
#endif
