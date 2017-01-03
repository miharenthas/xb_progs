//This file contains the implementations of the cut primitives

#include "xb_cut_typedefs.h"

namespace XB{
	//----------------------------------------------------------------------------------
	//1-dimensional primitive (just a segment)
	//constructors:
	//default:
	_xb_1D_cut_primitive::_xb_1D_cut_primitive(){} //do nothing
	
	//with extremes:
	_xb_1D_cut_primitive::_xb_1D_cut_primitive( double a, double b ){
		_extremes[0] = a;
		_extremes[1] = b;
	}
	
	//copy:
	_xb_1D_cut_primitive::_xb_1D_cut_primitive( _xb_1D_cut_primitive &given ){
		memcpy( _extremes, given._extremes, 2*sizeof(double) );
	}
	
	//dtor:
	_xb_1D_cut_primitive::~_xb_1D_cut_primitive(){}
	
	//methods:
	//pertinence test:
	bool _xb_1D_cut_primitive::contains( double pt ){
		return pt >= _extremes[0] && pt <= _extremes[1];
	}
	
	//manipulation:
	//first extreme:
	double &_xb_1D_cut_primitive::a(){
		return _extremes[0];
	}
	
	//second extreme:
	double &_xb_1D_cut_primitive::b(){
		return _extremes[1];
	}
	
	
	//assignmet operator
	_xb_1D_cut_primitive &_xb_1D_cut_primitive::operator=( _xb_1D_cut_primitive &right ){
		memcpy( _extremes, right._extremes, 2*sizeof(double) );
		
		return *this;
	}

	//----------------------------------------------------------------------------------
	//2-dimesntional primitives (there are a few, and here's fun)
	//----------------------------------------------------------------------------------
	//base class:
	//constructors:
	//default
	_xb_2D_cut_primitive::_xb_2D_cut_primitive():
		_identity( CUT_NOTHING ),
		_deformed( false )
	{
		_descriptors = (double*)malloc( 2*sizeof(double) );
		_basis[0] = gsl_vector_alloc( 2 );
		_basis[1] = gsl_vector_alloc( 2 );
		_base_change = gsl_matrix_alloc( 2, 2 );
		
		do_basis();
	}
	
	//with identity
	_xb_2D_cut_primitive::_xb_2D_cut_primitive( cut_primitive_2D identity ):
		_identity( identity ),
		_deformed( false )
	{
		_descriptors = (double*)malloc( 2*sizeof(double) );
		_basis[0] = gsl_vector_alloc( 2 );
		_basis[1] = gsl_vector_alloc( 2 );
		_base_change = gsl_matrix_alloc( 2, 2 );
		
		do_basis();
	}
	
	//destructor
	_xb_2D_cut_primitive::~_xb_2D_cut_primitive(){
		free( _descriptors );
		gsl_vector_free( _basis[0] );
		gsl_vector_free( _basis[1] );
		gsl_matrix_free( _base_change );
	}
	
	//non-abstract methods:
	//access to the first centroid coordinate
	double &_xb_2D_cut_primitive::C_1(){
		return _descriptors[0];
	}
	
	//access the second centroid coordinate
	double &_xb_2D_cut_primitive::C_2(){
		return _descriptors[1];
	}
	
	//get the identity
	cut_primitive_2D _xb_2D_cut_primitive::type(){
		return _identity;
	}

	//reallocate (typically, enlarge) the descriptors array
	void _xb_2D_cut_primitive::realloc_descriptors( const unsigned int how_many ){
		_descriptors = (double*)realloc( _descriptors, how_many*sizeof(double) );
		if( _descriptors == NULL ){
			fprintf( stderr, "MEMORY ERROR in 2D primitive cut!\nABORT.\n" );
			exit( 1 );
		}
		memset( _descriptors, 0, how_many*sizeof(double) );
	}
	
	//make the basis
	void _xb_2D_cut_primitive::do_basis(){
		gsl_vector_set( _basis[0], 0, 1 );
		gsl_vector_set( _basis[0], 1, 0 );
		gsl_vector_set( _basis[1], 0, 0 );
		gsl_vector_set( _basis[1], 1, 1 );
		
		gsl_matrix_set_identity( _base_change );
	}
	
	//make _base_change from the basis vectors
	void _xb_2D_cut_primitive::do_change_basis( gsl_matrix *trf ){
		//transform the basis, disregarding translations, which do
		//not affect the thing --isolate the "roto-scale-sheer" part
		//of the affine transformation (it always works)
		gsl_matrix *trf_core = gsl_matrix_alloc( 2, 2 );
		for( int i=0; i < 2; ++i )
			for( int j=0; j < 2; ++j )
				gsl_matrix_set( trf_core, i, j, gsl_matrix_get( trf, i, j ) );
		
		//transform the basis
		gsl_vector *res = gsl_vector_alloc( 2 );
		for( int i=0; i < 2; ++i ){
			gsl_blas_dgemv( CblasNoTrans, 1, trf_core, _basis[i], 0, res );
			gsl_vector_memcpy( _basis[i], res );
		}
		
		//cleanup
		gsl_vector_free( res );
		gsl_matrix_free( trf_core );
		
		//do a base change to matrix
		gsl_matrix_set( _base_change, 0, 0, _basis[1]->data[1] );
		gsl_matrix_set( _base_change, 0, 1, -_basis[1]->data[0] );
		gsl_matrix_set( _base_change, 1, 0, -_basis[0]->data[1] );
		gsl_matrix_set( _base_change, 1, 1, _basis[0]->data[0] );
		
		double det = gsl_vector_get( _basis[0], 0 )*gsl_vector_get( _basis[1], 1 )
		             - gsl_vector_get( _basis[1], 0 )*gsl_vector_get( _basis[0], 1 );
		
		gsl_matrix_scale( _base_change, 1./det );
	}
	
	//change the base to a point
	void _xb_2D_cut_primitive::do_change_basis_to_point( double *pt ){
		gsl_vector_view pt_vv = gsl_vector_view_array( pt, 2 );
		gsl_vector *res = gsl_vector_alloc( 2 );
		
		gsl_blas_dgemv( CblasNoTrans, 1, _base_change, &pt_vv.vector, 0, res );
		
		memcpy( pt, res->data, 2*sizeof(double) );
		
		gsl_vector_free( res );
	}
	
	//---------------------------------------
	//circle class:
	//constructors:
	//default:
	_xb_cut_circle::_xb_cut_circle():
		_xb_2D_cut_primitive( CUT_CIRCLE )
	{
		//(re)allocate the descriptor buffer.
		realloc_descriptors( 3 );
	}
	
	//just radius:
	_xb_cut_circle::_xb_cut_circle( double radius ):
		_xb_2D_cut_primitive( CUT_CIRCLE )
	{
		realloc_descriptors( 3 );
		_descriptors[2] = radius;
	}
	
	//radius and centre:
	_xb_cut_circle::_xb_cut_circle( double *centre, double radius ):
		_xb_2D_cut_primitive( CUT_CIRCLE )
	{
		realloc_descriptors( 3 );
		memcpy( _descriptors, centre, 2*sizeof(double) );
		_descriptors[2] = radius;
	}
	
	//copy:
	_xb_cut_circle::_xb_cut_circle( _xb_cut_circle &given ):
		_xb_2D_cut_primitive( CUT_CIRCLE )
	{
		realloc_descriptors( 3 );
		memcpy( _descriptors, given._descriptors, 3*sizeof(double) );
		
		_deformed = given._deformed;
		gsl_vector_memcpy( _basis[0], given._basis[0] );
		gsl_vector_memcpy( _basis[1], given._basis[1] );
		gsl_matrix_memcpy( _base_change, given._base_change );
	}
	
	//destructor (do nothing)
	_xb_cut_circle::~_xb_cut_circle(){}
	
	//methods:
	//access radius
	double &_xb_cut_circle::radius(){
		return _descriptors[2];
	}
	
	//check if a point is in the circle
	bool _xb_cut_circle::contains( double *pt ){
		double distance[2];
		
		double pt_prime[] = { pt[0], pt[1] };
		
		//bear in mind that:
		//_descriptors[0,1] == this->C_1,2()
		//_descriptors[2] == this->radius()
		distance[0] = pt_prime[0] - _descriptors[0];
		distance[1] = pt_prime[1] - _descriptors[1];
		
		//build a vector view
		gsl_vector_view dist_vec = gsl_vector_view_array( distance, 2 );
		
		//return whether the norm of the distance is less or equal the radius
		return gsl_blas_dnrm2( &dist_vec.vector ) <= _descriptors[2];
	}
	
	//transform:
	void _xb_cut_circle::transform( gsl_matrix *trf ){
		//check the size
		if( trf->size1 != 3 || trf->size2 != 3 )
			throw error( "Matrix is wrongly sized!", "XB::cut_circle::transform" );
		
		//allocate the vector
		gsl_vector *centre_vec = gsl_vector_calloc( 3 );
		gsl_vector *res = gsl_vector_alloc( 3 ); //result buffer
		
		//write inside it the centre coordinates
		memcpy( centre_vec->data, _descriptors, 2*sizeof(double) );
		centre_vec->data[2] = 1; //and the trailing one
		
		//blas multiply them
		gsl_blas_dgemv( CblasNoTrans, 1, trf, centre_vec, 0, res );
		
		//write the new centre coordinates
		memcpy( _descriptors, res->data, 2*sizeof(double) );
		
		//free the gsl vector
		gsl_vector_free( centre_vec );
		gsl_vector_free( res );
	}
	
	//assignment operator
	_xb_cut_circle &_xb_cut_circle::operator=( _xb_cut_circle &right ){
		memcpy( _descriptors, right._descriptors, 3*sizeof(double) );
		
		gsl_vector_memcpy( _basis[0], right._basis[0] );
		gsl_vector_memcpy( _basis[1], right._basis[1] );
		gsl_matrix_memcpy( _base_change, right._base_change );
		
		_identity = right._identity;
		_deformed = right._deformed;
		
		return *this;
	}
	
	//----------------------------------------------------------------------------------
	//class ellipse:
	//constructors:
	//default:
	_xb_cut_ellipse::_xb_cut_ellipse()
	{
		_identity = CUT_ELLIPSE;
		
		//_descriptors[0] ==> C_0
		//_descriptors[1] ==> C_1
		//_descriptors[2] ==> a (x//semiaxis)
		//_descriptors[3] ==> b
		//_descriptors[4] ==> rotation (radians)
		realloc_descriptors( 5 );
	}
	
	//with semiaxes
	_xb_cut_ellipse::_xb_cut_ellipse( double *semiaxes ){
		_identity = CUT_ELLIPSE;
	
		//copy the semiaxes data
		realloc_descriptors( 5 );
		memcpy( _descriptors+2, semiaxes, 2*sizeof(double) );
	}
	
	//with centroid and semiaxes
	_xb_cut_ellipse::_xb_cut_ellipse( double *semiaxes, double rotation ){
		_identity = CUT_ELLIPSE;
		
		realloc_descriptors( 5 );
		
		//copy the semiaxes data
		memcpy( _descriptors+2, semiaxes, 2*sizeof(double) );

		//copy the rotation
		_descriptors[4] = rotation;
	}
	
	//with centroid, semiaxes and rotation
	_xb_cut_ellipse::_xb_cut_ellipse( double *centroid, double *semiaxes, double rotation ){
		_identity = CUT_ELLIPSE;
		
		realloc_descriptors( 5 );
		
		//copy the semiaxes data
		memcpy( _descriptors+2, semiaxes, 2*sizeof(double) );

		//copy the rotation
		_descriptors[4] = rotation;

		//copy the centroid
		memcpy( _descriptors, centroid, 2*sizeof(double) );
	}
	
	//copy constructor
	_xb_cut_ellipse::_xb_cut_ellipse( _xb_cut_ellipse &given ){
		_identity = CUT_ELLIPSE;
		realloc_descriptors( 5 );
		memcpy( _descriptors, given._descriptors, 5*sizeof(double) );
	
		//copy the base information
		_deformed = given._deformed;
		gsl_vector_memcpy( _basis[0], given._basis[0] );
		gsl_vector_memcpy( _basis[1], given._basis[1] );
		gsl_matrix_memcpy( _base_change, given._base_change );
	}
	
	//dtor
	_xb_cut_ellipse::~_xb_cut_ellipse(){}
	
	//manip methods
	double &_xb_cut_ellipse::a(){ return _descriptors[2]; } //vertical semiaxis
	double &_xb_cut_ellipse::b(){ return _descriptors[3]; } //horizontal semiaxis
	double &_xb_cut_ellipse::rotation(){ return _descriptors[4]; } //rotation
	
	//pertinence test
	bool _xb_cut_ellipse::contains( double *pt ){
		double pt_prime[] = { pt[0], pt[1] };
		
		//based on the ellipse equation
		double c = cos( _descriptors[4] );
		double s = sin( _descriptors[4] );
		double d[] = { pt_prime[0] - _descriptors[0], pt_prime[1] - _descriptors[1] };
		double &A = _descriptors[2];
		double &B = _descriptors[3];
		
		double test = pow( c*d[0] + s*d[1], 2 )/pow( A, 2 )
		              + pow( -1*s*d[0] + c*d[1], 2 )/pow( B, 2 ); 
		
		return ( test <= 1 )? true : false;
	}
	
	//transform:
	void _xb_cut_ellipse::transform( gsl_matrix *trf ){
		//check the size
		if( trf->size1 != 3 || trf->size2 != 3 )
			throw error( "Matrix is wrongly sized!", "XB::cut_ellipse::transform" );
		
		//allocate the vectors to be transformed
		gsl_vector *centre_vec = gsl_vector_calloc( 3 );
		gsl_vector *a_guidepoint = gsl_vector_calloc( 3 );
		gsl_vector *b_guidepoint = gsl_vector_calloc( 3 );
		gsl_vector *res = gsl_vector_alloc( 3 ); //a buffer where the result goes
		
		//populate them:
		//the center
		memcpy( centre_vec->data, _descriptors, 2*sizeof(double) );
		centre_vec->data[2] = 1; //and the trailing one
		
		//the A guidepoint
		a_guidepoint->data[0] = _descriptors[2];
		a_guidepoint->data[2] = 1;
		
		//the B guidepoint
		b_guidepoint->data[1] = _descriptors[3];
		b_guidepoint->data[2] = 1;
		
		//apply the native rotation and translation:
		//allocate and init
		gsl_matrix *native_trans = gsl_matrix_alloc( 3, 3 ); //native translation
		gsl_matrix *native_rot = gsl_matrix_alloc( 3, 3 ); //native rotation
		gsl_matrix *native_trf = gsl_matrix_alloc( 3, 3 ); //native transformation
		gsl_matrix_set_identity( native_trans );
		gsl_matrix_set_identity( native_rot );
		
		//populate
		gsl_matrix_set( native_trans, 0, 2, _descriptors[0] );
		gsl_matrix_set( native_trans, 1, 2, _descriptors[1] );
		
		gsl_matrix_set( native_rot, 0, 0, cos( _descriptors[4] ) );
		gsl_matrix_set( native_rot, 0, 1, -sin( _descriptors[4] ) );
		gsl_matrix_set( native_rot, 1, 0, sin( _descriptors[4] ) );
		gsl_matrix_set( native_rot, 1, 1, cos( _descriptors[4] ) );
		
		//create the native transformation
		gsl_blas_dgemm( CblasNoTrans,
		                CblasNoTrans, 
		                1., native_trans, 
		                native_rot, 0., 
		                native_trf );
		
		//apply this transformation to the guidepoints
		gsl_blas_dgemv( CblasNoTrans, 1., native_trf, a_guidepoint, 0., res );
		gsl_vector_memcpy( a_guidepoint, res );
		
		gsl_blas_dgemv( CblasNoTrans, 1., native_trf, b_guidepoint, 0., res );
		gsl_vector_memcpy( b_guidepoint, res ); 
		
		//now, apply the given transformation
		//blas multiply them
		gsl_blas_dgemv( CblasNoTrans, 1, trf, centre_vec, 0, res );
		gsl_vector_memcpy( centre_vec, res );
		
		gsl_blas_dgemv( CblasNoTrans, 1, trf, a_guidepoint, 0, res );
		gsl_vector_memcpy( a_guidepoint, res );
		
		gsl_blas_dgemv( CblasNoTrans, 1, trf, b_guidepoint, 0, res );
		gsl_vector_memcpy( b_guidepoint, res );
		
		//calculate the length of the new semiaxes
		gsl_vector_sub( a_guidepoint, centre_vec );
		gsl_vector_sub( b_guidepoint, centre_vec );
		_descriptors[2] = gsl_blas_dnrm2( a_guidepoint );
		_descriptors[3] = gsl_blas_dnrm2( b_guidepoint );
		
		//last thing: calculate the new rotation:
		_descriptors[4] = atan2( a_guidepoint->data[1], a_guidepoint->data[0] );
		
		//write the new centre coordinates
		memcpy( _descriptors, centre_vec->data, 2*sizeof(double) );
		
		//free the gsl vector
		gsl_vector_free( centre_vec );
	}

	//assignment operator
	_xb_cut_ellipse &_xb_cut_ellipse::operator=( _xb_cut_ellipse &right ){
		memcpy( _descriptors, right._descriptors, 5*sizeof(double) );
		
		gsl_vector_memcpy( _basis[0], right._basis[0] );
		gsl_vector_memcpy( _basis[1], right._basis[1] );
		
		_identity = right._identity;
		_deformed = right._deformed;
		
		return *this;
	}	
	
	//----------------------------------------------------------------------------------
	//class polygon:
	//constructors:
	//default:
	_xb_cut_polygon::_xb_cut_polygon():
		_xb_2D_cut_primitive( CUT_POLYGON )
	{
		//descriptors[0] ==> C_1
		//descriptors[1] ==> C_2
		//descriptosr[2] ==> n_of_sides
		realloc_descriptors( 3 );
	}
	
	//vertices and number of sides:
	_xb_cut_polygon::_xb_cut_polygon( double *vertices, unsigned int n_of_sides ):
		_xb_2D_cut_primitive( CUT_POLYGON )
	{
		realloc_descriptors( 3 );
		_vertices = (double*)malloc( 2*n_of_sides*sizeof(double) );
		memcpy( _vertices, vertices, 2*n_of_sides*sizeof(double) );
		
		_descriptors[2] = n_of_sides;
		
		do_centroid();
	}
	
	//centroid, vertices and number of sides:
	_xb_cut_polygon::_xb_cut_polygon( double *centroid, double *vertices, unsigned int n_of_sides ):
		_xb_2D_cut_primitive( CUT_POLYGON )
	{
		realloc_descriptors( 3 );
		_vertices = (double*)malloc( 2*n_of_sides*sizeof(double) );
		memcpy( _vertices, vertices, 2*n_of_sides*sizeof(double) );
		
		_descriptors[2] = n_of_sides;		
		
		memcpy( _descriptors, centroid, 2*sizeof(double) );
	}
	
	//destructor
	_xb_cut_polygon::~_xb_cut_polygon(){ free( _vertices ); }
	
	//manip methods
	double &_xb_cut_polygon::nb_of_sides(){ return _descriptors[2]; }
	double *_xb_cut_polygon::vertices(){ return _vertices; }
	
	//since we're 2Ding, this works. It's a ludicrous hack.
	long double _xb_cut_polygon::vertex( unsigned int n ){
		long double packed_vertex = 0.;
		memcpy( &packed_vertex, _vertices + (n % (int)_descriptors[2]), 2*sizeof(double) );
		return packed_vertex;
	}
	
	//transformation method
	//modifies the object!
	void _xb_cut_polygon::transform( gsl_matrix *trf ){
		if( trf->size1 != 3 || trf->size2 != 3 )
			throw error( "Matrix is wrongly sized!", "XB::cut_polygon::transform" );
			
		//do a gsl vector for matrix multiplication
		gsl_vector *cv = gsl_vector_calloc( 3 );
		gsl_vector *res = gsl_vector_calloc( 3 );
		cv->data[2] = 1;

		//do the multiplication for all the vertices
		for( int i=0; i < _descriptors[2]; ++i ){
			memcpy( cv->data, _vertices + 2*i, 2*sizeof(double) );
			gsl_blas_dgemv( CblasNoTrans, 1, trf, cv, 0, res ); //blas matrix mulitplication
			memcpy( _vertices + 2*i, res->data, 2*sizeof(double) );
		}
		
		do_centroid();
	}

	//pertinency check
	bool _xb_cut_polygon::contains( double *pt ){
		//check if the point is in the polygon, with CGAL
		//1) create the CGAL polygon
		cgal_pt *points = (cgal_pt*)calloc( _descriptors[2], sizeof(cgal_pt) );
		for( int i=0; i < _descriptors[2]; ++i ){
			points[i] = cgal_pt( _vertices[2*i], _vertices[2*i+1] );
		}

		//2) check if the point is in the polygon:
		//if the point is inside or on the boundary of the polygon
		//return true, else return false.
		cgal_pt the_pt( pt[0], pt[1] );
		bool truth = false;
		switch( CGAL::bounded_side_2( points, points+(int)_descriptors[2], the_pt, cgal_K() ) ){
			case CGAL::ON_BOUNDED_SIDE: case CGAL::ON_BOUNDARY : truth = true; break;
			case CGAL::ON_UNBOUNDED_SIDE: truth = false; break;
		}

		free( points );
		return truth;
	}
	
	//do centroid (basically, calc the CoM  of the thing
	//NOTE: it's protected, just utlis
	void _xb_cut_polygon::do_centroid(){
		double CoM[] = {0, 0};
		
		for( int i=0; i < _descriptors[2]; ++i ){
			CoM[0] += _vertices[2*i];
			CoM[1] += _vertices[2*i+1];
		}
		
		CoM[0] /= _descriptors[2];
		CoM[1] /= _descriptors[2];
		
		_descriptors[0] = CoM[0];
		_descriptors[1] = CoM[1];
	}
	
	//assignment operator
	_xb_cut_polygon &_xb_cut_polygon::operator=( _xb_cut_polygon &right ){
		memcpy( _descriptors, right._descriptors, 3*sizeof(double) );
		memcpy( _vertices, right._vertices, _descriptors[2]*sizeof(double) );
		
		_identity = right._identity;
				
		return *this;
	}
	
	//----------------------------------------------------------------------------------
	//class regular polygon
	//constructors:
	//default:
	_xb_cut_regular_polygon::_xb_cut_regular_polygon(){
		_identity = CUT_REGULAR_POLYGON;
		//descriptors[0] ==> C_1
		//descriptors[1] ==> C_2
		//descriptosr[2] ==> n_of_sides
		//descriptors[3] ==> radius -- the radius of the
		//                             circumscribed circle
		//descriptors[4] ==> rotation
		realloc_descriptors( 5 );
	}
	
	//parametric: radius and number of sides
	_xb_cut_regular_polygon::_xb_cut_regular_polygon( double radius, double n_of_sides ){
		realloc_descriptors( 5 );
		_vertices = (double*)calloc( 2*n_of_sides, sizeof(double) );
		
		//store radius and number of sides
		_descriptors[0] = _descriptors[1] = 0; //set the centriod to (0,0)
		_descriptors[2] = n_of_sides;
		_descriptors[3] = radius;
		
		//calculate the vertices:
		//NOTE: this calculation *always* puts the first vertex
		//      at (0,radius) and proceeds clockwise
		const double ANGLE_STEP = _XB_TAU/_descriptors[2];
		for( int v=0; v < _descriptors[2]; ++v ){
			_vertices[2*v] = radius*sin( ANGLE_STEP*v );
			_vertices[2*v+1] = radius*cos( ANGLE_STEP*v );
		}
	}
	
	//parametric: radius, n of sides and centroid
	_xb_cut_regular_polygon::_xb_cut_regular_polygon( double *centroid, double radius,
	                                                  double n_of_sides ){
		realloc_descriptors( 5 );
		_vertices = (double*)calloc( 2*n_of_sides, sizeof(double) );
		
		//store radius and number of sides
		_descriptors[2] = n_of_sides;
		_descriptors[3] = radius;
		
		//calculate the vertices:
		//NOTE: this calculation *always* puts the first vertex
		//      at (0,radius) and proceeds clockwise
		const double ANGLE_STEP = _XB_TAU/_descriptors[2];
		for( int v=0; v < _descriptors[2]; ++v ){
			_vertices[2*v] = radius*sin( ANGLE_STEP*v );
			_vertices[2*v+1] = radius*cos( ANGLE_STEP*v );
		}
		
		//set the centroid
		_descriptors[0] = centroid[0];
		_descriptors[1] = centroid[1];

		//translate the vertices
		for( int v=0; v < _descriptors[2]; ++v ){
			_vertices[2*v] += _descriptors[0];
			_vertices[2*v+1] += _descriptors[1];
		}
	}
	
	//parametric: with rotation
	//NOTE: the rotation gets applied in the constructor,
	//      it is *not* undestood as something that is already
	//      an attribute of the cut! (This may change)
	_xb_cut_regular_polygon::_xb_cut_regular_polygon( double *centroid, double radius,
	                                                  double n_of_sides, double rotation ){
		realloc_descriptors( 5 );
		_vertices = (double*)calloc( 2*n_of_sides, sizeof(double) );
		
		//store radius and number of sides
		_descriptors[2] = n_of_sides;
		_descriptors[3] = radius;
		
		//calculate the vertices:
		//NOTE: this calculation *always* puts the first vertex
		//      at (0,radius) and proceeds clockwise
		const double ANGLE_STEP = _XB_TAU/_descriptors[2];
		for( int v=0; v < _descriptors[2]; ++v ){
			_vertices[2*v] = radius*sin( ANGLE_STEP*v );
			_vertices[2*v+1] = radius*cos( ANGLE_STEP*v );
		}
		
		//set the centroid and rotation
		_descriptors[0] = centroid[0];
		_descriptors[1] = centroid[1];
		_descriptors[4] = rotation;
		
		//place the polygon
		do_init_transform();
	}

	//copy
	_xb_cut_regular_polygon::_xb_cut_regular_polygon( _xb_cut_regular_polygon &given ){
		realloc_descriptors( 5 );
		memcpy( _descriptors, given._descriptors, 5*sizeof(double) );
		
		if( _vertices == NULL ) _vertices = (double*)malloc( given._descriptors[2]*sizeof(double) );
		else _vertices = (double*)realloc( _vertices, 2*given._descriptors[2]*sizeof(double) );
		memcpy( _vertices, given._vertices, 5*sizeof(double) );
		_deformed = given._deformed;
	}
	
	//destructor
	_xb_cut_regular_polygon::~_xb_cut_regular_polygon(){} //do nothing
	
	//make the initial transformation and apply it
	void _xb_cut_regular_polygon::do_init_transform(){
		//some aliasing
		double *centroid = _descriptors;
		double &rotation = _descriptors[4];
		
		//make the transformation
		gsl_matrix *trf = gsl_matrix_alloc( 3, 3 );
		gsl_matrix *rot = gsl_matrix_alloc( 3, 3 );
		gsl_matrix *trans = gsl_matrix_alloc( 3, 3 );
		
		gsl_matrix_set_identity( rot );
		gsl_matrix_set_identity( trans );
		gsl_matrix_set_identity( trf );
		
		//rotation
		gsl_matrix_set( rot, 0, 0, cos( rotation ) );
		gsl_matrix_set( rot, 1, 1, cos( rotation ) );
		gsl_matrix_set( rot, 0, 1, -1*sin( rotation ) );
		gsl_matrix_set( rot, 1, 0, sin( rotation ) );
		
		//affine translation (for centroid)
		gsl_matrix_set( trans, 0, 2, centroid[0] );
		gsl_matrix_set( trans, 1, 2, centroid[1] );
		
		//compose the two ( trans*rot ---> first rotation, then translation )
		gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, trans, rot, 0, trf );
		
		//apply the transformation using own's method
		transform( trf );
		
		gsl_matrix_free( trf );
		gsl_matrix_free( rot );
		gsl_matrix_free( trans );
	}
	
	//manip methods
	double &_xb_cut_regular_polygon::radius(){ return _descriptors[3]; }
	double &_xb_cut_regular_polygon::rotation(){ return _descriptors[4]; }
	
	//transformation method
	void _xb_cut_regular_polygon::transform( gsl_matrix *trf ){
		//check if the transformation is anti- or symmetric
		if( gsl_matrix_get( trf, 0, 0 ) != gsl_matrix_get( trf, 1, 1 )
		    || abs( gsl_matrix_get( trf, 0, 1 ) ) != abs( gsl_matrix_get( trf, 1, 0 ) ) ){
			_deformed = true;
		}
		
		_xb_cut_polygon::transform( trf ); //call the parent's method
		do_change_basis( trf ); //change the basis
	}
	
	//pip test
	bool _xb_cut_regular_polygon::contains( double *pt ){
		//if this polygon is deformed, change the basis to the given point
		//and then proceed as usual.
		double pt_prime[] = { pt[0], pt[1] };
		pt_prime[0] -= _descriptors[0];
		pt_prime[1] -= _descriptors[1];
		do_change_basis_to_point( pt_prime );
		
		//first: if it's further away than the radius from the
		//centrioid, it's out (no holes here)
		double dfc = sqrt( pow( pt_prime[0], 2 ) + pow( pt_prime[1], 2 ) ); //norm of vector distance
		
		if( dfc > _descriptors[3] ) return false;
		
		//if we came here, it's worth to check if it's in the
		//inscribed circumference -- in which case, it's in
		if( dfc <= _descriptors[3]*cos( _XB_PI/_descriptors[2] ) ) return true;
		
		//if we didn't get it yet, desperation: ray casting.
		//NOTE: with the untouched point, since the basis doesn't enter into it
		return _xb_cut_polygon::contains( pt );
	}
	
	//assignment operator
	_xb_cut_regular_polygon &_xb_cut_regular_polygon::operator=( _xb_cut_regular_polygon &right ){
		memcpy( _descriptors, right._descriptors, 5*sizeof(double) );
		memcpy( _vertices, right._vertices, _descriptors[2]*sizeof(double) );
		
		_identity = right._identity;
		_deformed = right._deformed;
		
		return *this;
	}
	
	//----------------------------------------------------------------------------------
	//class square (inherits from regular polygon):
	//constructors:
	//default:
	//NOTE: the correct identity is already set by the default constructor
	//      in the parent class (no CUT_SQUARE type!)
	_xb_cut_square::_xb_cut_square(){
		_identity = CUT_SQUARE;
		//descriptors[0] ==> C_1
		//descriptors[1] ==> C_2
		//descriptosr[2] ==> n_of_sides
		//descriptors[3] ==> radius -- the radius of the
		//                             circumscribed circle
		//descriptors[4] ==> rotation
		//descriptors[5] ==> side
		realloc_descriptors( 6 );
		_vertices = (double*)calloc( 8, sizeof(double) );
		
		_descriptors[2] = 4;
	}
	
	//parametric: side
	_xb_cut_square::_xb_cut_square( double side ){
		_identity = CUT_SQUARE;
		realloc_descriptors( 6 );
		_vertices = (double*)calloc( 8, sizeof(double) );
		
		_descriptors[2] = 4;
		_descriptors[5] = side;
		
		do_square_from_side( side );
	}
	
	//parmetric: full
	_xb_cut_square::_xb_cut_square( double *centroid, double side, double rotation ){
		_identity = CUT_SQUARE;
		realloc_descriptors( 6 );
		_vertices = (double*)calloc( 8, sizeof(double) );
		
		_descriptors[2] = 4;
		_descriptors[5] = side;
		
		do_square_from_side( side );
		
		//save the missing bits
		_descriptors[0] = centroid[0];
		_descriptors[1] = centroid[1];
		_descriptors[4] = rotation;
		
		do_init_transform();
	}
	
	//copy:
	_xb_cut_square::_xb_cut_square( _xb_cut_square &given ){
		_identity = CUT_SQUARE;
		realloc_descriptors( 6 );
		_vertices = (double*)calloc( 8, sizeof(double) );
		
		memcpy( _descriptors, given._descriptors, 6*sizeof(double) );
		memcpy( _vertices, given._vertices, 8*sizeof(double) );
		
		_deformed = given._deformed;
		gsl_vector_memcpy( _basis[0], given._basis[0] );
		gsl_vector_memcpy( _basis[1], given._basis[1] );
		gsl_matrix_memcpy( _base_change, given._base_change );
	}
	
	//destructor
	_xb_cut_square::~_xb_cut_square(){ gsl_matrix_free( _base_change ); }
	
	//util to make the square from the side
	void _xb_cut_square::do_square_from_side( double side ){
		side /= 2;
		_vertices[0] = side; //bottom right
		_vertices[1] = -side;
		_vertices[2] = side; //top right
		_vertices[3] = side;
		_vertices[4] = -side; //top left
		_vertices[5] = side;
		_vertices[6] = -side; //bottom left
		_vertices[7] = -side;
	}
	
	//manip methods
	double &_xb_cut_square::side(){ return _descriptors[5]; }
	
	//the usual transform overload
	void _xb_cut_square::transform( gsl_matrix *trf ){
		//call the parent's transform method
		_xb_cut_regular_polygon::transform( trf );
	}
	
	//pip test
	bool _xb_cut_square::contains( double *pt ){
		//if there's a transformation, proceed with the general test with the base change.
		//NOTE: the centroid is kept up to date
		//      in the grandparent class' transform
		//      method.
		double pt_prime[] = { pt[0], pt[1] };
		pt_prime[0] -= _descriptors[0]; //translate it back in the centroid's origin
		pt_prime[1] -= _descriptors[1];
		do_change_basis_to_point( pt_prime );
		
		double h_side = _descriptors[5]*.5;
		
		if( pt_prime[0] > h_side || pt_prime[0] < -h_side
		    || pt_prime[1] > h_side || pt_prime[1] < -h_side ) return false;
		else return true;
		
	}
	
	//assignment operator
	_xb_cut_square &_xb_cut_square::operator=( _xb_cut_square &right ){
		memcpy( _descriptors, right._descriptors, 3*sizeof(double) );
		memcpy( _vertices, right._vertices, 4*sizeof(double) );
		
		gsl_vector_memcpy( _basis[0], right._basis[0] );
		gsl_vector_memcpy( _basis[1], right._basis[1] );
		gsl_matrix_memcpy( _base_change, right._base_change );
		
		_identity = right._identity;
		_deformed = right._deformed;
		
		return *this;
	}
	
	//TODO: more classes to come!
	//...
	
} //namespace
