//this is the error class declaration for the xb_explorer suite
#ifndef XB_ERROR__H
#define XB_ERROR__H

#include <string.h>

namespace XB{
	//an exception structure
	typedef class _xb_error{
		public:
			_xb_error( const char* message, const char* from );
			_xb_error( const _xb_error &given );
			~_xb_error(){ delete what; };
			char *what;
	} error;
}
#endif
