//this file defines a sort-of-associatve array, which will be used to store
//data that aren't really crystal ball data but support.
//The structure will inherit from the event_holder.

#ifndef XB_ARBITRARY_DATA__H
#define XB_ARBITRARY_DATA__H

#include <stdlib.h>
#include <string.h>
#include <vector>

#include "xb_error.h"
#include "xb_data.h"

#define XB_ADATA_NB_FIELDS 256

namespace XB{
	//----------------------------------------------------------------------------
	//a data structure representing the field,
	typedef struct _xb_arb_data_field {
		char name[64];
		size_t size;
	} adata_field;

	//----------------------------------------------------------------------------
	//the main thing, the class
	typedef class _xb_arbitrary_data : public event_holder {
		public:
			//ctors, dtor
			_xb_arbitrary_data();
			_xb_arbitrary_data( const adata_field *fld_array, size_t n_fld );
			_xb_arbitrary_data( const _xb_arbitrary_data &given );
			~_xb_arbitrary_data();
			
			//important operators
			_xb_arbitrary_data &operator=( _xb_arbitrary_data &right );
			//get data from field, by name.
			//use fsize( char *name ) to ge the returned buffer size
			void *operator()( const char *name );
			bool operator==( const _xb_arbitrary_data &right ) const;
			bool operator!=( const _xb_arbitrary_data &right ) const;
			
			//accessing methods:
			//create/write field
			//if *buf is NULL, the field is just created
			void dofield( const char *name, size_t size, void *buf );
			void dofield( const adata_field &fld, void *buf );
			//get the size of a field
			//remove a field
			void rmfield( const char *name );
			//for size-1 fields of a specified type
			//you can use this themplate mehtod, too
			template< class T >
			T getfield( const char *name ){
				void *head = _fld_ptr[phash8( name )];
				if( !head ) throw error( "Not a field!", "XB::adata::getfield" );
				head = (int*)head + 1;
				return *(T*)head;
			};
			//list the fields
			std::vector<adata_field> lsfields(){ return _fields } const;
			
			friend int adata_getlbuf( void **buf, _xb_arbitrary_data &given );
			friend int adata_fromlbuf( _xb_arbitrary_data &here, void *buf );
		private:
			//the data buffer
			//data is stored [int size|data]
			//the field pointer point to the size!!!
			//can contain any type.
			int _buf_sz;
			void *_buf;
			void *_fld_ptr[XB_ADATA_NB_FIELDS]; //support 256 fields (more than enough)
			std::vector< adata_field > _fields; //keep track of the fields.
			
			//an utility to has a field name
			//pearson's has, 8 bits.
			unsigned char phash8( const char *name );
			const unsigned char _pT[256]; //LUT for the pearson's hash
			
			//an utility to realloc the buffer, safely
			void safe_buf_realloc( size_t size );
	} adata;
	
	//----------------------------------------------------------------------------
	//Two functions to be used in read/write operations (friended by the class)
	//they convert it in and from a linear buffer, ready to be written to file.
	//adata_getlbuf: get the linear buffer in void *buffer and return the size.
	//               buffer will be allocated. An error is thrown if it's already
	//               allocated.
	int adata_getlbuf( void **buffer, const adata &given );
	int adata_frombuf( adata &given, const void *buffer );
	int nf2hdr_size( int nf ){ return (nf+2)*sizeof(int) + nf*sizeof(adata_field); }
}

#endif
