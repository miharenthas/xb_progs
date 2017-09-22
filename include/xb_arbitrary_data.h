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

//some defines
#define XB_ADATA_NB_FIELDS 256 //maximum number of fields supported
                               //NOTE: keep it small, but that means
                               //      hash collisions will be a problem
//the hash table
//NOTE: apparently, there's a way to generate one that won't produce collisions
//      on a given set of words. This is NOT it yet, it's just a random one.
//TODO: get a proper one for the 153 fields in the land02 tree.
#define XB_PAERSON_HASH_TABLE { 171, 104, 228, 208, 188, 42, 152, 244, 137, 117, 173, 255, 201, 215, 204, 41, 74, 45, 246, 249, 91, 184, 227, 59, 64, 133, 114, 220, 122, 155, 192, 212, 43, 105, 46, 16, 77, 156, 98, 126, 191, 12, 190, 75, 55, 169, 13, 106, 84, 36, 33, 170, 61, 194, 144, 136, 178, 164, 29, 161, 108, 206, 121, 123, 129, 135, 47, 6, 146, 10, 250, 185, 239, 51, 89, 15, 49, 30, 94, 128, 193, 32, 181, 183, 142, 210, 163, 34, 67, 3, 143, 100, 230, 216, 23, 19, 97, 93, 159, 22, 124, 76, 237, 226, 162, 139, 200, 221, 252, 145, 125, 199, 70, 229, 20, 182, 154, 219, 18, 96, 119, 179, 5, 217, 160, 35, 231, 37, 56, 132, 82, 81, 115, 116, 223, 92, 65, 112, 158, 180, 2, 153, 168, 113, 209, 165, 134, 8, 167, 120, 225, 101, 148, 253, 176, 243, 205, 207, 21, 54, 247, 44, 235, 202, 172, 4, 147, 73, 40, 83, 213, 102, 196, 85, 189, 80, 53, 242, 66, 157, 109, 195, 31, 78, 48, 69, 57, 86, 186, 38, 218, 110, 79, 211, 187, 203, 0, 127, 1, 233, 9, 224, 14, 87, 7, 254, 95, 150, 88, 248, 25, 27, 107, 63, 149, 138, 50, 238, 240, 198, 24, 251, 17, 118, 140, 166, 68, 90, 99, 197, 111, 130, 234, 174, 245, 232, 62, 28, 60, 214, 151, 175, 222, 52, 72, 11, 58, 39, 177, 131, 71, 236, 26, 103, 241, 141 }

//a define to get the header size of the linear buffer
#define nf2hdr_size( nf ) ((nf)+2)*sizeof(int) + (nf)*sizeof(adata_field)

namespace XB{
	//----------------------------------------------------------------------------
	//a data structure representing the field,
	typedef struct _xb_arb_data_field {
		char name[16];
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
			_xb_arbitrary_data &operator=( const _xb_arbitrary_data &right );
			//get data from field, by name.
			//use fsize( char *name ) to ge the returned buffer size
			void *operator()( const char *name ) const;
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
			void clear(); //remove everything.
			//for size-1 fields of a specified type
			//you can use this themplate mehtod, too
			template< class T >
			T getfield( const char *name ) const {
				void *head = _fld_ptr[phash8( name )];
				if( !head ) throw error( "Not a field!", "XB::adata::getfield" );
				head = (int*)head + 1;
				return *(T*)head;
			};
			//list the fields
			std::vector< adata_field > lsfields() const { return _fields; };
			int fsize( const char *name ) const;
			
			//a couple of friends, for I/O ops
			friend int adata_getlbuf( void **linbuf, const _xb_arbitrary_data &given );
			friend int adata_fromlbuf( _xb_arbitrary_data &here, const void *buf );
			//TODO: and one for merge
			/*friend _xb_arbitrary_data adata_merge( const _xb_arbitrary_data &one,
			                                       const _xb_arbitrary_data &two );*/
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
			unsigned char phash8( const char *name ) const;
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
	int adata_getlbuf( void **linbuf, const _xb_arbitrary_data &given );
	int adata_fromlbuf( _xb_arbitrary_data &here, const void *buf );
	/*_xb_arbitrary_data adata_merge( const _xb_arbitrary_data &one,
	                                const _xb_arbitrary_data &two );*/
		
}
#endif
