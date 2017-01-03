//test the various cuts
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "xb_cut_typedefs.h"

#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

int main( int argc, char **argv ){
	//----------------------------------------------------------------------------
	//test the 1D cut
	printf( "------------\n" );
	XB::cut_segment segment_cut( 0, 2.99 );
	
	int rsp = 0;
	
	srand( time( NULL ) );
	
	float guess = 0;
	
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess = rand()%7;
		rsp += (segment_cut.contains( guess ) ? 1 : 0);
	}
	
	printf( "%d random numbers from 0 to 6: %d in.\n", atoi( argv[1] ), rsp );
	printf( "ratio: %f\n", (float)rsp/atof( argv[1] ) );
	
	//----------------------------------------------------------------------------
	//test the circular cut (by calculating pi!)
	printf( "------------\n" );
	double centre[] = {1, 1};
	XB::cut_circle circ_cut( centre, 1. );
	rsp = 0;
	double guess_pt[] = {0, 0};
	
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess_pt[0] = 2.*(double)rand();
		guess_pt[1] = 2.*(double)rand();
		guess_pt[0] /= RAND_MAX;
		guess_pt[1] /= RAND_MAX;
		rsp += (circ_cut.contains( guess_pt ) ? 1 : 0);
		
	}
	
	printf( "%d random numbers from (0,0) to (2,2): %d in.\n", atoi( argv[1] ), rsp );
	printf( "PI estimated at: %f\n", 4.*float(rsp)/atof( argv[1] ) );
	
	//----------------------------------------------------------------------------
	//test the ellipse cut (again by calculating pi!)
	printf( "------------\n" );
	double semiaxes[] = { 0.5, 1.5 };
	centre[0] = 5.;
	centre[1] = 5.;
	
	rsp = 0;
	XB::cut_ellipse ell_cut( centre, semiaxes, .3 );
	
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess_pt[0] = 10.*(double)rand();
		guess_pt[1] = 10.*(double)rand();
		guess_pt[0] /= RAND_MAX;
		guess_pt[1] /= RAND_MAX;
		rsp += (ell_cut.contains( guess_pt ) ? 1 : 0);
	}
	
	double p_est = 100.*((float)rsp/atof( argv[1] ))/0.75;
	printf( "%d random numbers from (0,0) to (10,10): %d in.\n", atoi( argv[1] ), rsp );
	printf( "PI estimated at: %f\n", p_est );
	
	//----------------------------------------------------------------------------
	//test the polygonal cut
	printf( "------------\n" );
	double vertices[10];
	
	//generate a regular pentagon of radius 1 and centered in (1,1);
	for( int i=0; i < 5; ++i ){
		vertices[2*i] = cos( 2./5.*_XB_PI*i )+1.;
		vertices[2*i+1] = sin( 2./5.*_XB_PI*i )+1.;
	}
	
	XB::cut_polygon pent_cut( vertices, 5 );
	printf( "The deduced centroid is: (%f,%f)\n", pent_cut.C_1(), pent_cut.C_2() );
	
	//apply a transformation
	gsl_matrix *rot = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( rot );
	
	//add a rotation
	double rotation = .7;
	gsl_matrix_set( rot, 0, 0, cos( rotation ) );
	gsl_matrix_set( rot, 1, 1, cos( rotation ) );
	gsl_matrix_set( rot, 0, 1, -1*sin( rotation ) );
	gsl_matrix_set( rot, 1, 0, sin( rotation ) );
	
	//add a shear
	gsl_matrix *shear = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( shear );
	gsl_matrix_set( shear, 0, 1, 3 );
	
	//add a translation to (3, 1)
	gsl_matrix *trans = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( trans );
	gsl_matrix_set( trans, 0, 2, 3 );
	gsl_matrix_set( trans, 1, 2, 1 );
	
	//stack the matrices (rot*shear*trans ---> first trans, then shear, then rot )
	//NOTE: it's a bit cluncky but blas doesn't work in-place.
	gsl_matrix *trf, *trf_buf;
	trf = gsl_matrix_alloc( 3, 3 );
	trf_buf = gsl_matrix_alloc( 3, 3 );
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, rot, shear, 0, trf_buf );
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, trf_buf, trans, 0, trf );
	
	printf( "The transformation:\n" );
	for( int i=0; i < 3; ++i ){
		for( int j=0; j < 3; ++j ){
			printf( "\t%f", gsl_matrix_get( trf, i, j ) );
		}
		printf( "\n" );
	}
	
	//apply the transformation
	pent_cut.transform( trf );
	
	//check out the vertices
	printf( "Vertices:\n" );
	for( int i=0; i < 5; ++i ){
		printf( "\t(%f, %f)\n", pent_cut.vertices()[2*i], pent_cut.vertices()[2*i+1] );
	}
	printf( "Centroid is: (%f,%f)\n", pent_cut.C_1(), pent_cut.C_2() );
	
	//calculate the determinant of the 2*2 transformation
	double det = gsl_matrix_get( trf, 0 , 0 )*gsl_matrix_get( trf, 1, 1 )
	             - gsl_matrix_get( trf, 0, 1 )*gsl_matrix_get( trf, 1, 0 );
	
	printf( "The determinant of the transformation is: %f\n", det );
	
	//generate the random numbers in a 50*50 square (that hopefully
	//contains the transformed pentagon)
	rsp = 0;
	#pragma omp parallel for private( guess_pt ) shared( rsp, pent_cut )
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess_pt[0] = 50.*(double)rand();
		guess_pt[1] = 50.*(double)rand();
		guess_pt[0] /= RAND_MAX;
		guess_pt[1] /= RAND_MAX;
		rsp += (pent_cut.contains( guess_pt ) ? 1 : 0);
	}
	
	double exp_area = 5./4.*sqrt( (5. + sqrt( 5. ))/2. )*det;
	
	printf( "%d random numbers from (0,0) to (50,50): %d in.\n", atoi( argv[1] ), rsp );
	printf( "The area appears to be %f. Expected %f.\n",
	        2500*(float)rsp/atof( argv[1] ), exp_area );
	
	//----------------------------------------------------------------------------
	//regular polygon cut
	printf( "------------\n" );
	centre[0] = 2.;
	centre[1] = 2.;
	
	XB::cut_regular_polygon hept_cut( centre, 3.25, 7, 0.53 );
	
	//check out the vertices
	printf( "Vertices:\n" );
	for( int i=0; i < 7; ++i ){
		printf( "\t(%f, %f)\n", hept_cut.vertices()[2*i], hept_cut.vertices()[2*i+1] );
	}
	printf( "Centroid is: (%f,%f)\n", hept_cut.C_1(), hept_cut.C_2() );
	
	//set up a scale, a translation and a rotation
	gsl_matrix_set( trans, 0, 2, 5. );
	gsl_matrix_set( trans, 1, 2, 7. );
	
	gsl_matrix_set( rot, 0, 0, cos( 0.35 ) );
	gsl_matrix_set( rot, 1, 1, cos( 0.35 ) );
	gsl_matrix_set( rot, 0, 1, -1*sin( 0.35 ) );
	gsl_matrix_set( rot, 1, 0, sin( 0.35 ) );
	
	gsl_matrix *scale = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( scale );
	gsl_matrix_set( scale, 0, 0, 1.25 );
	gsl_matrix_set( scale, 1, 1, 1.25 );
		
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, scale, trans, 0, trf_buf );
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, rot, trf_buf, 0, trf );
	
	printf( "The transformation:\n" );
	for( int i=0; i < 3; ++i ){
		for( int j=0; j < 3; ++j ){
			printf( "\t%f", gsl_matrix_get( trf, i, j ) );
		}
		printf( "\n" );
	}
	
	//get the determinant of the transformation
	det = gsl_matrix_get( trf, 0 , 0 )*gsl_matrix_get( trf, 1, 1 )
	      - gsl_matrix_get( trf, 0, 1 )*gsl_matrix_get( trf, 1, 0 );
	
	printf( "The determinant of the transformation is: %f\n", det );
	
	hept_cut.transform( trf );
	
	//check out the vertices
	printf( "Vertices:\n" );
	for( int i=0; i < 7; ++i ){
		printf( "\t(%f, %f)\n", hept_cut.vertices()[2*i], hept_cut.vertices()[2*i+1] );
	}
	printf( "Centroid is: (%f,%f)\n", hept_cut.C_1(), hept_cut.C_2() );
	
	//generate the random numbers in a 50*50 square (that hopefully
	//contains the transformed heptagon)
	rsp = 0;
	//#pragma omp parallel for private( guess_pt ) shared( rsp, hept_cut )
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess_pt[0] = 50.*(double)rand();
		guess_pt[1] = 50.*(double)rand();
		guess_pt[0] /= RAND_MAX;
		guess_pt[1] /= RAND_MAX;
		guess_pt[0] -= 25;
		guess_pt[1] -= 25;
		rsp += (hept_cut.contains( guess_pt ) ? 1 : 0);
	}
	
	//calc the expected area
	const double *v = hept_cut.vertices();
	double side_sq = pow( v[0]-v[2], 2 ) + pow( v[1]-v[3], 2 );
	exp_area = 7./4.*side_sq/tan( _XB_PI/7. );
	
	printf( "Side length: %f\n", sqrt( side_sq ) );
	
	printf( "%d random numbers from (-25,-25) to (25,25): %d in.\n", atoi( argv[1] ), rsp );
	printf( "The area appears to be %f. Expected %f.\n",
	        2500*(float)rsp/atof( argv[1] ), exp_area );
	
	//----------------------------------------------------------------------------
	//test a square cut
	printf( "------------\n" );
	centre[0] = 2.;
	centre[1] = 2.;
	
	//make a square
	XB::cut_square sq_cut( centre, 3., 0.1 );
		
	//check out the vertices
	printf( "Vertices:\n" );
	for( int i=0; i < 4; ++i ){
		printf( "\t(%f, %f)\n", sq_cut.vertices()[2*i], sq_cut.vertices()[2*i+1] );
	}
	
	//no need to test transform, since it's essentially the same as for the regular polygon
	//moving on to the pip test
	rsp = 0;
	for( int i=0; i < atoi( argv[1] ); ++i ){
		guess_pt[0] = 10.*(double)rand();
		guess_pt[1] = 10.*(double)rand();
		guess_pt[0] /= RAND_MAX;
		guess_pt[1] /= RAND_MAX;
		guess_pt[0] -= 5;
		guess_pt[1] -= 5;
		rsp += (sq_cut.contains( guess_pt ) ? 1 : 0);
	}
	
	exp_area = 9.;
	
	printf( "%d random numbers from (-5,-5) to (5,5): %d in.\n", atoi( argv[1] ), rsp );
	printf( "The area appears to be %f. Expected %f.\n",
	        100*(float)rsp/atof( argv[1] ), exp_area );
	
	return 0;
}
