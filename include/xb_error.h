//this is the error class declaration for the xb_explorer suite
#ifndef XB_ERROR__H
#define XB_ERROR__H

#include <string.h>

#include <exception>

namespace XB{
	//an exception structure
	typedef class _xb_error : public std::exception {
		public:
			_xb_error( const char* message, const char* from );
			_xb_error( const _xb_error &given );
			~_xb_error() noexcept { delete _what; };

			virtual const char *what() const noexcept;

			char *_what;
	} error;
}
#endif
