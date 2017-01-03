//a test program for the cut interface and drawing routines

#include <time.h>
#include <stdlib.h>

#include <gsl/gsl_matrix.h>

#include "xb_draw_cut.h"
#include "xb_cut.h"
#include "xb_cut_typedefs.h"
#include "xb_data.h"

extern "C" {
	#include "gnuplot_i.h"
}

#define N_OF_POINTS 10000

//------------------------------------------------------------------------------------
//implementation of the point extractor
class ext : public XB::cut_data_2D< XB::event_holder > {
	public:
		ext( std::vector< XB::event_holder* > &data_array ):
			XB::cut_data_2D< XB::event_holder >( data_array ) {};
	
		XB::pt_holder operator()( unsigned int i ){
			double a = _data_array[i]->n;
			double b = _data_array[i]->evnt;
			return XB::pt_holder( a, b );
	}
};

//------------------------------------------------------------------------------------
//main:
int main( int argc, char **argv ){
	//first: create the test set
	std::vector< XB::event_holder* > some_data;
	
	srand( time(NULL) );
	for( int i=0; i < N_OF_POINTS; ++i ){
		some_data.push_back( new XB::event_holder );
		some_data[i]->n = rand()%1000;
		some_data[i]->evnt = rand()%1500;
	}
	
	ext point_extractor( some_data );
	
	//second: apply the cuts (and visualize them) in sequence
	//----------------------------
	//a circle:
	double c[] = { 130., 240. }, radius = 100;
	XB::cut_circle circ( c, radius );
	
	//apply it:
	std::vector<bool> flags( N_OF_POINTS );
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = circ.contains( (double*)point_extractor( i ) );
	}
	
	//draw it
	gnuplot_ctrl *gp_h = XB::draw_cut< XB::event_holder >( &circ, point_extractor, flags, "try", "this" );
	
	char ch = getchar();
	gnuplot_close( gp_h );
	
	//----------------------------
	//an ellipse:
	double sem[] = { 30., 300. };
	c[0] = 0;
	c[1] = 300;
	XB::cut_ellipse ell( c, sem, 0. );
	
	//do a transform (just for the sake of it)
	gsl_matrix *trf = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( trf );
	gsl_matrix_set( trf, 0, 0, cos( _XB_PI*0.25 ) );
	gsl_matrix_set( trf, 0, 1, 1*sin( _XB_PI*0.25 ) );
	gsl_matrix_set( trf, 1, 0, -1*sin( _XB_PI*0.25 ) );
	gsl_matrix_set( trf, 1, 1, cos( _XB_PI*0.25 ) );
	
	ell.transform( trf );
	
	//apply it:
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = ell.contains( (double*)point_extractor( i ) );
	}
	
	//draw it
	gp_h = XB::draw_cut< XB::event_holder >( &ell, point_extractor, flags, "try", "this" );
	
	ch = getchar();
	gnuplot_close( gp_h );
	
	//----------------------------
	//a polygon
	double vertices[] = { 0, 0,
	                      300, 200,
	                      100, 500,
	                      -200, 600,
	                      -100, 500 };
	XB::cut_polygon pol( vertices, 5 );
	
	//make a sheer
	gsl_matrix *shear = gsl_matrix_alloc( 3, 3 );
	gsl_matrix_set_identity( shear );
	gsl_matrix_set( shear, 0, 1, 1.5 );
	
	gsl_matrix *res = gsl_matrix_alloc( 3, 3 );
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, shear, trf, 0, res );
	gsl_matrix_memcpy( trf, res );
	
	//transform the polygon
	pol.transform( trf );
	
	//apply it:
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = pol.contains( (double*)point_extractor( i ) );
	}
	
	gp_h = XB::draw_cut< XB::event_holder >( &pol, point_extractor, flags, "try", "this" );
	
	ch = getchar();
	gnuplot_close( gp_h );
	
	//----------------------------
	//a regular polygon
	XB::cut_regular_polygon rpo( c, 150, 7 );
	
	//transform it
	rpo.transform( trf );
	
	//apply it:
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = rpo.contains( (double*)point_extractor( i ) );
	}

	//display it
	gp_h = XB::draw_cut< XB::event_holder >( &rpo, point_extractor, flags, "try", "this" );
	
	ch = getchar();
	gnuplot_close( gp_h );
	
	//----------------------------
	//a square
	XB::cut_square sq( c, 300., 0. );

	//transform it
	sq.transform( shear );
	
	//apply it:
	for( int i=0; i < N_OF_POINTS; ++i ){
		flags[i] = sq.contains( (double*)point_extractor( i ) );
	}
	
	//draw it
	gp_h = XB::draw_cut< XB::event_holder >( &sq, point_extractor, flags, "try", "this" );

	ch = getchar();
	gnuplot_close( gp_h );
	
	//third: happy thoughts
	return 0;
}
