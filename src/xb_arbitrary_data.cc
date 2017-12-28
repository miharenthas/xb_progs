//implementation of xb_arbitrary_data.h

#include "xb_arbitrary_data.h"
#include <stdio.h>
namespace XB{
	//----------------------------------------------------------------------------
	//constructors:
	adata::_xb_arbitrary_data():
		_buf( NULL ),
		_buf_sz( 0 ),
		_fields( 0 )
	{
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
		_buf_sz( 0 ),
		_fields( 0 )
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
		_buf( NULL ),
		_buf_sz( given._buf_sz ),
		_fields( given._fields )
	{
		n = given.n;
		evnt = given.evnt;
		tpat = given.tpat;
		in_Z = given.in_Z;
		in_A_on_Z = given.in_A_on_Z;
		
		//copy the buffer
		_buf = malloc( given._buf_sz );
		if( !_buf ) throw error( "Memory error!", "XB::adata::assign" );
		memcpy( _buf, given._buf, given._buf_sz );
		
		//copy the pointers
		int dist = 0;
		char *p_curr = NULL;
		const char *p_head = (char*)given._buf;

		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !given._fld_ptr[i] ){ _fld_ptr[i] = NULL; continue; }
			
			p_curr = (char*)given._fld_ptr[i];
			dist = p_curr - p_head;
			_fld_ptr[i] = (char*)_buf + dist;
		}
	}
	
	//dtor:
	adata::~_xb_arbitrary_data(){
		if( _buf ) free( _buf );
	}
	
	//----------------------------------------------------------------------------
	//utils:
	//----------------------------------------------------------------------------
	//now the fun begins: the hash function
	unsigned char adata::phash8( const char *name ) const {
		int len = strlen( name );
		unsigned short h = 0;
		
		for( int i=0; i < len; ++i ) h = adata_pT[ h ^ name[i] ];
		
		return h;
	}
	
	//----------------------------------------------------------------------------
	//safe realloc (with moving around the pointers in _fld_ptr
	//***works***
	void adata::safe_buf_realloc( size_t size ){

		if( !_buf ){_buf = malloc( size+sizeof(int) ); return; }
	
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
		n = given.n;
		evnt = given.evnt;
		tpat = given.tpat;
		in_Z = given.in_Z;
		in_A_on_Z = given.in_A_on_Z;
		
		_buf_sz = given._buf_sz;
		_fields = given._fields;
		
		//copy the buffer
		if( _buf ) free( _buf );
		_buf = malloc( given._buf_sz );
		if( !_buf ) throw error( "Memory error!", "XB::adata::assign" );
		memcpy( _buf, given._buf, given._buf_sz );
		
		//copy the pointers
		int dist = 0;
		char *p_curr = NULL;
		const char *p_head = (char*)given._buf;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ) _fld_ptr[i] = NULL;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ){
			if( !given._fld_ptr[i] ) continue;
			
			p_curr = (char*)given._fld_ptr[i];
			dist = p_curr - p_head;
			_fld_ptr[i] = (char*)_buf + dist;
		}
		
		return *this;
	}
	
	//----------------------------------------------------------------------------
	//comparison ops
	bool adata::operator==( const adata &right ) const {
		if( n != right.n) return false;
		if( evnt != right.evnt) return false;
		if( tpat != right.tpat) return false;
		if( in_Z != right.in_Z) return false;
		if( in_A_on_Z != right.in_A_on_Z ) return false;

		if( _buf_sz != right._buf_sz ) return false;
		if( _fields.size() != right._fields.size() ) return false;
		
		for( int i=0; i < _fields.size(); ++i ){
			if( strcmp( _fields[i].name, right._fields[i].name ) ||
			    _fields[i].size != right._fields[i].size ) return false;
		}
		
		if( memcmp( _buf, right._buf, _buf_sz ) ) return false;
		
		return true;
	}
	
	bool adata::operator!=( const adata &right ) const {
		return !( *this == right );
	}
	
	//----------------------------------------------------------------------------
	//accessors:
	
	//----------------------------------------------------------------------------
	//parenthesis operator
	void *adata::operator()( const char *name ) const {
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
	
	void adata::dofield( const char *name, short size, void *buf ){
		adata_field fld = { "", size };
		strcpy( fld.name, name );
		dofield( fld, buf );
	}
	
	//----------------------------------------------------------------------------
	//get the size of a field (in bytes)
	int adata::fsize( const char *name ) const {
		void *head = _fld_ptr[phash8( name )];
		if( !head ) return 0;
		return *(int*)head;
		return 0;
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
	
	//----------------------------------------------------------------------------
	//clear the structure
	void adata::clear(){
		n = 0;
		tpat = 0;
		in_Z = 0;
		in_A_on_Z = 0;
		if( _buf ){ free( _buf ); _buf = NULL; }
		_buf_sz = 0;
		for( int i=0; i < XB_ADATA_NB_FIELDS; ++i ) _fld_ptr[i] = NULL;
		_fields.clear();
	}
	
	//============================================================================
	//the two friend functions.

	//----------------------------------------------------------------------------
	//make the linearized buffer:
	//[event_holder| # fields|field list|field pointer deltas|data size|data buffer]
	int adata_getlbuf( void **linbuf, const adata &given ){
		int nf = given._fields.size();
		
		//calculate the deltas
		int *deltas = (int*)calloc( nf, sizeof(int) ), *d_indirect;
		d_indirect = deltas;
		if( !deltas ) throw error( "Memory error!", "XB::adata_getlbuf" );
		for( int i=0; i < nf; ++i ){
			*d_indirect = (char*)given._fld_ptr[given.phash8( given._fields[i].name )] -
			              (char*)given._buf;
			++d_indirect;
		}
		
		//allocate the linear buffer
		int bsize = sizeof(event_holder) + (nf+2)*sizeof(int) +
		            nf*sizeof(adata_field) + given._buf_sz;
		*linbuf = malloc( bsize );
		
		//do the copying
		void *head = *linbuf;
		memcpy( head, &given.n, sizeof(event_holder) ); //the event holder
		head = (event_holder*)head + 1;
		*(int*)head = nf; //# fields
		head = (int*)head + 1;
		memcpy( head, &given._fields[0], nf*sizeof(adata_field) ); //field list
		head = (adata_field*)head + nf;
		memcpy( head, deltas, nf*sizeof(int) ); //deltas
		head = (int*)head + nf;
		*(int*)head = given._buf_sz; //data size
		head = (int*)head + 1;
		memcpy( head, given._buf, given._buf_sz ); //data
		
		free( deltas );
		
		return bsize;
	}
	
	//----------------------------------------------------------------------------
	//now from the linear buffer to the structure
	int adata_fromlbuf( adata &given, const void *buffer ){
		void *hdr = (char*)buffer + sizeof(event_holder);
		int nf = *(int*)hdr;
		hdr = (int*)hdr + 1;
		int hdr_sz = nf2hdr_size( nf ) -1*sizeof(int);
		
		void *data = (char*)buffer + hdr_sz;
		int data_sz = *(int*)data;
		data = (int*)data + 1;
		
		//clear the structure
		given.clear();
		
		//copy the event holder
		memcpy( &given.n, buffer, sizeof(event_holder) );
		
		//copy the data
		given._buf_sz = data_sz;
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
		
		
