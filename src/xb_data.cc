//implementation of the xb_data class

#include "xb_data.h"


namespace XB{
	//----------------------------------------------------------------------------
	//XB::data implementation
	data::_xb_data_structure() { buf = NULL; }
	data::_xb_data_structure( unsigned int the_n, unsigned int the_id ):
		sum_e( 0 ),
		in_beta( 0 ),
		empty_t( true ),
		empty_pt( true ),
		empty_e( true ),
		empty_he( true ),
		empty_sum_e( true ),
		empty_in_beta( true )
	{
		n = the_n;
		evnt = the_id;
		tpat = 0;
		make_buf();
	}

	data::_xb_data_structure( const _xb_data_structure& given ):
		sum_e( given.sum_e ),
		in_beta( given.in_beta ),
		empty_t( given.empty_t ),
		empty_pt( given.empty_pt ),
		empty_e( given.empty_e ),
		empty_he( given.empty_he ),
		empty_sum_e( given.empty_sum_e ),
		empty_in_beta( given.empty_in_beta )
	{
		n = given.n;
		evnt = given.evnt;
		tpat = given.tpat;
		make_buf();
		
		memcpy( buf, given.buf, 4*n*sizeof(float) + n*sizeof(unsigned int) ); //copy the buffer
	}

	data::~_xb_data_structure(){ free( buf ); t = NULL; pt = NULL; e = NULL; he = NULL, i = NULL; }
	
	//the memory allocator
	void data::make_buf(){
		buf = (void*)calloc( 1, 4*n*sizeof(float) + n*sizeof(unsigned int) );
		t = (float*)buf;
		pt = (float*)buf + n;
		e = (float*)buf + 2*n;
		he = (float*)buf + 3*n;
		i = (unsigned int*)((float*)buf + 4*n);
	}
	
	//this function has the task of pruning out the non-number constant that are stuck inside
	//the data
	void data::probe_for_crap(){
		for( int i=0; i < n; ++i ){
			if( !isnan( t[i] ) && !isinf( t[i] ) ) empty_t = false;
			else t[i] = 0;
			if( !isnan( pt[i] ) && !isinf( pt[i] ) ) empty_pt = false;
			else pt[i] = 0;
			if( !isnan( e[i] ) && !isinf( e[i] ) ) empty_e = false;
			else e[i] = 0;
			if( !isnan( he[i] ) && !isinf( he[i] ) ) empty_he = false;
			else he[i] = 0;
		}
		if( !isnan( sum_e ) ) empty_sum_e = false;
		else sum_e = 0;
		if( !isnan( in_beta ) ) empty_in_beta = false;
		else in_beta = 1;
	}

	
	//assignment operator (!!!)
	data &data::operator=( const data &given ){
		//copy all the things
		n = given.n;
		evnt = given.evnt;
		tpat = given.tpat;
		sum_e = given.sum_e;
		in_beta = given.in_beta;
		empty_t = given.empty_t;
		empty_pt = given.empty_pt;
		empty_e = given.empty_e;
		empty_he = given.empty_he;
		empty_sum_e = given.empty_sum_e;
		empty_in_beta = given.empty_in_beta;
		
		//free the current buffer
		if( this->buf != NULL ) free( this->buf );
		
		//make another one
		this->make_buf();
		
		//copy it
		memcpy( buf, given.buf, 4*n*sizeof(float) + n*sizeof(unsigned int) );
		
		//done
		return *this;
	}

	//----------------------------------------------------------------------------
	//XB::track_info implementation
	track_info::_xb_track_data_structure() { buf = NULL; }
	track_info::_xb_track_data_structure( unsigned int n_frags, unsigned int the_id ) {
		n = n_frags;
		evnt = the_id;
		make_buf();
	}

	track_info::_xb_track_data_structure( const _xb_track_data_structure& given ):
		in_beta( given.in_beta ),
		beta_0( given.beta_0 ),
		in_Z( given.in_Z ),
		in_A_on_Z( given.in_A_on_Z )
	{
		n = given.n;
		evnt = given.evnt;
		make_buf();
		
		memcpy( buf, given.buf, 3*n*sizeof(float) + 2*n*sizeof(versor) ); //copy the buffer
	}

	track_info::~_xb_track_data_structure(){
		free( buf );
		fragment_A = NULL;
		fragment_Z = NULL;
		fragment_beta = NULL;
		incoming = NULL;
		outgoing = NULL;
	}
	
	//the memory allocator
	void track_info::make_buf(){
		//allocate the memory buffer
		buf = calloc( 1, 3*n*sizeof(float) + 2*n*sizeof(versor) );
		
		//link the pointers
		fragment_A = (float*)buf;
		fragment_Z = (float*)buf + n;
		fragment_beta = (float*)buf + 2*n;
		incoming = (versor*)((float*)buf + 3*n);
		outgoing = incoming + n;
	}
	
	//assignment operator (!!!)
	track_info &track_info::operator=( const track_info &given ){
		n = given.n;
		evnt = given.evnt;
		tpat = given.tpat;
		in_beta = given.in_beta;
		beta_0 = given.beta_0;
		in_Z = given.in_Z;
		in_A_on_Z = given.in_A_on_Z;
		
		this->make_buf();
		memcpy( buf, given.buf, 3*n*sizeof(float) + 2*n*sizeof(versor) ); //copy the buffer
		
		return *this;
	}

	//----------------------------------------------------------------------------
	//comparison operators.
	
	//-------------------------
	//for the versors
	bool operator==( const versor &one, const versor &two ){
		return one.i == two.i && one.j == two.j && one.k == two.k;
	}
	
	bool operator!=( const versor &one, const versor &two ){
		return !( one == two );
	}
	
	
	//-------------------------
	//for track_info
	bool operator==( const track_info &one, const track_info &two ){
		if( one.n != two.n ) return false;
		if( one.evnt != two.evnt ) return false;
		if( one.tpat != two.tpat ) return false;
		if( one.in_beta != two.in_beta ) return false;
		if( one.beta_0 != two.beta_0 ) return false;
		if( one.in_Z != two.in_Z ) return false;
		if( one.in_A_on_Z != two.in_A_on_Z ) return false;
		
		for( int i=0; i < one.n; ++i ){
			if( one.fragment_A[i] != two.fragment_A[i] ) return false;
			if( one.fragment_Z[i] != two.fragment_Z[i] ) return false;
			if( one.fragment_beta[i] != two.fragment_beta[i] ) return false;
			if( one.incoming[i] != two.incoming[i] ) return false;
			if( one.outgoing[i] != two.outgoing[i] ) return false;
		}
		
		return true;
	}
	
	bool operator!=( const track_info &one, const track_info &two ){
		return !( one == two );
	}
	
	
	//-------------------------
	//for data
	bool operator==( const data &one, const data &two ){
		if( one.n != two.n ) return false;
		if( one.evnt != two.evnt ) return false;
		if( one.tpat != two.tpat ) return false;
	
		for( int i=0; i < one.n; ++i ){
			if( one.t[i] != two.t[i] ) return false;
			if( one.pt[i] != two.pt[i] ) return false;
			if( one.e[i] != two.e[i] ) return false;
			if( one.he[i] != two.he[i] ) return false;
			if( one.i[i] != two.i[i] ) return false;
		}
		
		if( one.sum_e != two.sum_e ) return false;
		if( one.in_beta != two.in_beta ) return false;
	
		return true;
	}

	bool operator!=( const data &one, const data &two ){
		return !( one == two );
	}

}
