//this files contains the definitions of the types for the cut programs
//TODO: implement the remaining classes.
//TODO: the _deformed flag may be obsolete: consider getting rid of it.
#ifndef XB_CUT_TYPEDEFS__H
#define XB_CUT_TYPEDEFS__H

//stl includes
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//GSL includes
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

//CGAL includes
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2_algorithms.h>

//toolkit includes
#include "xb_error.h"

#define _XB_PI 3.141592653589793
#define _XB_TAU 6.283185307179586

namespace XB{
	//----------------------------------------------------------------------------
	//some useful types:
	//cut types (1D cuts are trivial)
	typedef enum _xb_1D_cut_primitive_type{
		CUT_SEGMENT = 0
	} cut_primitive_1D;
	
	//2D cuts
	typedef enum _xb_2D_cut_primitive_type{
		CUT_NOTHING = 0,
		CUT_SQUARE,
		CUT_CIRCLE,
		CUT_ELLIPSE,
		CUT_REGULAR_POLYGON,
		CUT_POLYGON
	} cut_primitive_2D;
	
	/*
	//3D cuts -- probably not implemented
	typedef enum _xb_3D_cut_primitive_type{
		CUT_CUBE = 0,
		CUT_PRISM,
		CUT_BSPLNE_PRISM,
		CUT_SPHERE,
		CUT_ELLIPSOID,
		CUT_TESSELLATED
	} cut_primitive_3D;
	*/
	
	/*
	//ND cuts -- probably unnecessary
	typedef enum _xb_ND_cut_primitive_type{
		CUT_HYPERCUBE = 0,
		CUT_HYPERPRISM,
		CUT_HYPERSPHERE,
		CUT_HYPERELLIPSOID,
		CUT_HYPERTESSELLATED
	} cut_primitive_ND;
	*/
	
	//----------------------------------------------------------------------------
	//the type family (and yes, this time I'm usign a full blown class tree)
	
	//1D primitive stands on its own
	typedef class _xb_1D_cut_primitive{
		public:
			//ctors, dtor
			_xb_1D_cut_primitive();
			_xb_1D_cut_primitive( double a, double b );
			_xb_1D_cut_primitive( _xb_1D_cut_primitive &given );
			~_xb_1D_cut_primitive();
			
			//test whether a point is in
			 bool contains( double pt );
			
			//extremes manipulation
			 double &a();
			 double &b();

			_xb_1D_cut_primitive &operator=( _xb_1D_cut_primitive &right );
		private:
			double _extremes[2];
	} cut_segment;
	
	//2D primitives base class
	//it's more of a conceptual thing so far
	class _xb_2D_cut_primitive{
		public:
			//default constructor and destructor
			_xb_2D_cut_primitive();
			_xb_2D_cut_primitive( cut_primitive_2D identity );
			~_xb_2D_cut_primitive();
			
			//identity check
			cut_primitive_2D type();
			
			//virtual method that makes this class incomplete
			virtual bool contains( double *pt ) =0;
			
			//centroid manipulation methods
			double &C_1();
			double &C_2();
			
			//transform method:
			//it is supposed to take a 3*3 matrix as input
			//and apply a complete transofrmation to the
			//relevant descriptors
			virtual void transform( gsl_matrix *trf ) =0;
		protected:
			void realloc_descriptors( const unsigned int how_many );
					
			double *_descriptors; //an array of descriptors
			                      //according to the kind
			cut_primitive_2D _identity; //the identity of this cut
			
			//basis change toolkit
			virtual void do_basis();
			void do_change_basis( gsl_matrix *trf );
			void do_change_basis_to_point( double *pt );
			
			gsl_vector *_basis[2]; //describes the current basis the
			                       //cut lives in. For regular shapes,
			                       //that have been ferociously transformed
			                       //it can mean optimization-ish
			gsl_matrix *_base_change; //the matrix for the change of basis...
			bool _deformed; //a flag to be set in case of transformation.
	};
	
	//2D primitives classes:
	//polygon
	typedef class _xb_cut_polygon : public _xb_2D_cut_primitive {
		public:
			//some CGAL typedefs:
			//the kernel for using doubles (and tollerating
			//their numerical noise)
			//and the 2D point object associated with it
			typedef CGAL::Exact_predicates_inexact_constructions_kernel cgal_K;
			typedef cgal_K::Point_2 cgal_pt;
			
			_xb_cut_polygon();
			_xb_cut_polygon( double *vertices, unsigned int n_of_sides );
			_xb_cut_polygon( double *centroid, double *vertices, unsigned int n_of_sides );
			_xb_cut_polygon( _xb_cut_polygon &given );
			
			~_xb_cut_polygon();
			
			//test whether a point is in
			virtual  bool contains( double *pt );
			
			//manipulation members
			double &nb_of_sides();
			
			//get the vertices
			double *vertices();
			long double vertex( unsigned int n );
			
			//transform method
			virtual void transform( gsl_matrix *trf );
			
			_xb_cut_polygon &operator=( _xb_cut_polygon &right );
		protected:
			void do_centroid();
			double *_vertices;
	} cut_polygon;
	
	//regular polygon, which is a polygon
	typedef class _xb_cut_regular_polygon : public _xb_cut_polygon {
		public:
			_xb_cut_regular_polygon();
			_xb_cut_regular_polygon( double radius, double n_fo_sides );
			_xb_cut_regular_polygon( double *centroid, double radius,
			                         double n_of_sides );
			_xb_cut_regular_polygon( double *centroid, double radius,
			                         double n_of_sides, double rotation );
			_xb_cut_regular_polygon( _xb_cut_regular_polygon &given );
			
			~_xb_cut_regular_polygon();
			
			//test whether a point is in
			virtual bool contains( double *pt );
			virtual void transform( gsl_matrix *trf );
			
			//manipulation members
			double &radius();
			double &rotation();
			
			_xb_cut_regular_polygon &operator=( _xb_cut_regular_polygon &right );
		protected:
			void do_init_transform(); //a routine to make the initial placement
			                          //in terms of the rotation and translation.
	} cut_regular_polygon;
	
	//square, which is a regular polygon
	typedef class _xb_cut_square : public _xb_cut_regular_polygon {
		public:
			//ctors, dtor
			_xb_cut_square();
			_xb_cut_square( double side );
			_xb_cut_square( double *centroid, double side, double rotation=0 );
			_xb_cut_square( _xb_cut_square &given );
			
			~_xb_cut_square();
			
			//transofrm overload
			void transform( gsl_matrix *trf );
			
			//test whether a point is in
			bool contains( double *pt );
		
			//manipulation members
			double &side();
			
			_xb_cut_square &operator=( _xb_cut_square &right );
		private:
			void do_square_from_side( double side );
	} cut_square;
	
		//circle
	typedef class _xb_cut_circle : public _xb_2D_cut_primitive {
		public:
			_xb_cut_circle();
			_xb_cut_circle( double radius );
			_xb_cut_circle( double *centre, double radius );
			_xb_cut_circle( _xb_cut_circle &given );
			
			~_xb_cut_circle();
			
			//test whether a point is in
			virtual bool contains( double *pt );
			
			//manipulation members
			double &radius();
			
			//transform method
			//NOTE: whatever happens, this stays a circle!
			//      if you want some flexibility, use an ellipse
			//      if you want utmost flexibility, use a polygon
			virtual void transform( gsl_matrix *trf );
			
			//get a polygon representing this cut
			virtual _xb_cut_polygon polygonise( unsigned int nb_vertices ){};
			
			//operators
			_xb_cut_circle &operator=( _xb_cut_circle &right );
	} cut_circle;
	
	//ellipse
	typedef class _xb_cut_ellipse : public _xb_cut_circle {
		public:
			_xb_cut_ellipse();
			_xb_cut_ellipse( double *semiaxes );
			_xb_cut_ellipse( double *semiaxes, double rotation );
			_xb_cut_ellipse( double *centroid, double *semiaxes, double rotation );
			_xb_cut_ellipse( _xb_cut_ellipse &given );
			
			~_xb_cut_ellipse();
			
			//test whether a point is in
			bool contains( double *pt );
			
			//manipulation members
			double &a();
			double &b();
			double &rotation();
			
			//transformation method
			//NOTE: whatever happens, this stays an ellipse!
			void transform( gsl_matrix *trf );

			//get a polygon representing this cut
			_xb_cut_polygon polygonise( unsigned int nb_vertices ){};
			
			_xb_cut_ellipse &operator=( _xb_cut_ellipse &right );
	} cut_ellipse;
	
	//----------------------------------------------------------------------------
	//N-dimensional cuts (including 3D cuts)
	//coming soon
	//...		
}

#endif
