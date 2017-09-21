//implementation of xb_arbitrary_data.h

#include "xb_arbitrary_data.h"

namespace XB{
	//----------------------------------------------------------------------------
	//constructors:
	adata::_xb_arbitrary_data(): _buf( NULL ), _pT XB_PAERSON_HASH_TABLE {
		//just banally init the event_holder members
		n = 0;
		evnt = 0;
		tpat = 0;
		in_Z = 0;
		in_A_on_Z = 0;
		
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ) _fld_ptr[i] = NULL;
	}
	
	adata::_xb_arbitrary_data( const adata_field *fld_array, size_t n_fld ):
		_buf( NULL ),
		_pT XB_PAERSON_HASH_TABLE
	{
		n = 0;
		evnt = 0;
		tpat = 0;
		in_Z = 0;
		in_A_on_Z = 0;
		
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ) _fld_ptr[i] = NULL;
		for( int i=0; i < n_fld; ++i ) dofield( fld_array[i], NULL );
	}
	
	adata::_xb_arbitrary_data( const adata &given ):
		_pT XB_PAERSON_HASH_TABLE
	{
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ) _fld_ptr[i] = NULL;
		*this = given; //since it's exactly the same code...
	}
	
	//dtor:
	adata::~_xb_arbitrary_data(){
		free( _buf );
	}
	
	//----------------------------------------------------------------------------
	//utils:
	//----------------------------------------------------------------------------
	//now the fun begins: the hash function
	unsigned char adata::phash8( const char *name ){
		int len = strlen( name );
		unsigned short h = 0;
		
		for( int i=0; i < len; ++i ) h = _pT[ h ^ name[i] ];
		
		return h;
	}
	
	//----------------------------------------------------------------------------
	//safe realloc (with moving around the pointers in _fld_ptr
	void adata::safe_buf_realloc( size_t size ){
		if( !_buf ){ _buf = malloc( size+sizeof(int) ); return; }
	
		void *old_buf = _buf;
		_buf = realloc( _buf, size+sizeof(int) );
		if( !_buf ) throw error( "Memory error!", "adata::safe_buf_realloc" );
		
		if( old_buf == _buf ) return; //nothing to do
		
		int delta;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !_fld_ptr[i] ) continue;
			
			delta = (char*)_fld_ptr[i] - (char*)old_buf; //D bytes;
			_fld_ptr[i] = (char*)_buf + delta;
		}
	}
	
	//----------------------------------------------------------------------------
	//operators:
	
	//----------------------------------------------------------------------------
	//assignment operator
	adata &adata::operator=( const adata &given ){
		_buf_sz = given._buf_sz;
		_fields = given._fields;
		
		//copy the buffer
		_buf = malloc( given._buf_sz );
		if( !_buf ) throw error( "Memory error!", "XB::adata::assign" );
		memcpy( _buf, given._buf, given._buf_sz );
		
		//copy the pointers
		int dist = 0;
		char *p_curr;
		const char *p_head = (char*)given._buf;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !given._fld_ptr ){ _fld_ptr[i] = NULL; continue; }
			
			p_curr = (char*)given._fld_ptr[i];
			dist = p_curr - p_head;
			_fld_ptr[i] = (char*)_buf + dist;
		}
		
		return *this;
	}
	
	//----------------------------------------------------------------------------
	//comparison ops
	bool adata::operator==( const adata &right ) const {
		if( _buf_sz != right._buf_sz ) return false;
		if( _fields.size() != right._fields.size() ) return false;
		
		for( int i=0; i < _fields.size(); ++i ){
			if( !strcmp( _fields[i].name, right._fields[i].name ) &&
			    _fields[i].size != right._fields[i].size ) return false;
		}
		
		const char *b_ptr = (char*)_buf, *rb_ptr = (char*)right._buf;
		for( int i=0; i < _buf_sz; ++i )
			if( b_ptr[i] != rb_ptr[i] ) return false;
		
		return true;
	}
	
	bool adata::operator!=( const adata &right ) const {
		return !( *this == right );
	}
	
	//----------------------------------------------------------------------------
	//accessors:
	
	//----------------------------------------------------------------------------
	//parenthesis operator
	void *adata::operator()( const char *name ){
		void *head = _fld_ptr[phash8( name )];
		if( !head ) throw error( "Field is empty!", "XB::adata::()" );
		
		int fsize = *(int*)head; //in bytes!
		head = (int*)head + 1;
		
		void *payload = malloc( fsize );
		if( !payload ) throw error( "Memory error!", "XB::adata::()" );
		
		memcpy( payload, head, fsize );
		return payload;
	}

	//----------------------------------------------------------------------------
	//access in write a field denoted by adata_field
	void adata::dofield( const adata_field &fld, void *buf ){
		unsigned char i_fld = phash8( fld.name );
		void *head = _fld_ptr[i_fld];
		
		if( !head ){ //the field is empty, let's do it
			safe_buf_realloc( _buf_sz+fld.size );
			
			//save the new field poiner
			_fld_ptr[i_fld] = (char*)_buf + _buf_sz;
			head = _fld_ptr[i_fld]; //and put it in the head
			*(int*)head = fld.size; //write the size
			head = (int*)head + 1; //move the head
			
			//if there's something to copy, do it
			//else zero the memory (safer)
			if( buf ) memcpy( head, buf, fld.size );
			else memset( head, 0, fld.size );
			
			//finally, update the buffer size
			//and push the new field in the field list
			_buf_sz += fld.size + sizeof(int);
			_fields.push_back( fld );
		} else { //the field is populated
			if( !buf ) return; //do nothing
			if( fld.size != *(int*)head ) //freak out
				throw error( "Wrong field size!", "XB::adata::dofield" );
			
			head = (int*)head + 1; //move the head past the size
			memcpy( head, buf, fld.size ); //copy
		}
	}
	
	void adata::dofield( const char *name, size_t size, void *buf ){
		adata_field fld = { "", size };
		strcpy( fld.name, name );
		dofield( fld, buf );
	}
	
	//----------------------------------------------------------------------------
	//get the size of a field (in bytes)
	int adata::fsize( const char *name ){
		void *head = _fld_ptr[phash8( name )];
		if( !head ) return 0;
		return *(int*)head;
	}
	
	//----------------------------------------------------------------------------
	//remove a field (another interesting method)
	void adata::rmfield( const char *name ){
		void *head = _fld_ptr[phash8( name )];
		if( !head ) return; //nothing to do
		
		//work out where head is in the buffer
		int fsize = *(int*)head + sizeof(int);
		int from_front = (char*)head - (char*)_buf; //how far from the front
		int to_back = _buf_sz - fsize - from_front; //how far from the back
		
		//find the beginning from the head
		void *rest = (char*)head + fsize;
		memmove( head, rest, to_back );
		
		//move the field pointers
		_fld_ptr[phash8( name )] = NULL; //remove the one
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !_fld_ptr[i] || _fld_ptr[i] < rest ) continue;
			
			//move them back by fsize (the removed bit)
			_fld_ptr[i] = (char*)_fld_ptr[i] - fsize;
		}
		
		//resize the buffer
		_buf_sz -= fsize;
		safe_buf_realloc( _buf_sz );
		
		//drum out the field from the field list
		from_front = 0; //recycle
		while( strcmp( (_fields.begin()+from_front)->name, name ) ) ++from_front;
		_fields.erase( _fields.begin() + from_front );
	}
	
	//============================================================================
	//the two friend functions.

	//----------------------------------------------------------------------------
	//make the linearized buffer:
	//[# fields|field list|field pointer deltas|data size|data buffer]
	int adata_getlbuf( void **buffer, const adata &given ){
		if( *buffer ) throw error( "Buffer not empty!", "XB::adata_getlbuf" );
	
		int nf = given._fields.size();
		
		//calculate the deltas
		int *deltas = (int*)malloc( nf*sizeof(int) ), *d_indirect;
		d_indirect = deltas;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !given._fld_ptr[i] ) continue;
			*d_indirect = (char*)given._fld_ptr[i] - (char*)given._buf;
			++d_indirect;
		}
		
		//allocate the linear buffer
		int bsize = (nf+2)*sizeof(int) + nf*sizeof(adata_field) + given._buf_sz;
		*buffer = malloc( bsize );
		void *linbuf = *buffer;
		
		//do the copying
		*(int*)linbuf = nf; //# fields
		linbuf = (int*)linbuf + 1;
		memcpy( linbuf, &given._fields[0], nf*sizeof(adata_field) ); //field list
		linbuf = (adata_field*)linbuf + nf;
		memcpy( linbuf, deltas, nf*sizeof(int) ); //deltas
		linbuf = (int*)linbuf + nf;
		*(int*)linbuf = given._buf_sz; //data size
		linbuf = (int*)linbuf + 1;
		memcpy( linbuf, given._buf, given._buf_sz ); //data
		
		return bsize;
	}
	
	//----------------------------------------------------------------------------
	//now from the linear buffer to the structure
	int adata_fromlbuf( adata &given, const void *buffer ){
		int nf = *(int*)buffer;
		void *hdr = (int*)buffer + 1;
		int hdr_sz = nf2hdr_size( nf ) -1*sizeof(int);
		
		void *data = (char*)buffer + hdr_sz;
		int data_sz = *(int*)data;
		data = (int*)data + 1;
		
		//copy the data
		given._buf_sz = data_sz;
		if( given._buf ) free( given._buf );
		given._buf = malloc( data_sz );
		memcpy( given._buf, data, data_sz );
		
		//copy the field list
		std::vector<adata_field> fields( nf );
		memcpy( &fields[0], hdr, nf*sizeof(adata_field) );
		given._fields = fields;
		
		//reconstruct the pointer map
		hdr = (adata_field*)hdr + nf;
		for( int i=0; i < fields.size(); ++i ){
			given._fld_ptr[given.phash8( fields[i].name )] = //...
				(char*)given._buf + *((int*)hdr+i);
		}
		
		return nf; //useless...
	}
} //end of namespace
		
		
