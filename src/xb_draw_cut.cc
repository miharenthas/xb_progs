//implementation of the utilities delared in xb_draw_cut.

#include "xb_draw_cut.h"

namespace XB{
	//------------------------------------------------------------------------------------
	//utilities:
	//TODO: make the routines smart enough so that a deformed cut can be drawn

	//------------------------------------------------------------------------------------
	//draws a circular cut.
	//NOTE: May fail if some interesting transformation has been applied
	void draw_cut_circle( gnuplot_ctrl *gp_h, cut_circle& the_cut ){
		//let's declare some buffers to build the command
		char gp_cmd[128], centre_bit[32], radius_bit[32], color_bit[64], str_buf[64];
	
		strcpy( gp_cmd, "set object 100 ellipse " );
	
		strcpy( str_buf, "center %f, %f " );
		sprintf( centre_bit, str_buf, the_cut.C_1(), the_cut.C_2() );
		strcat( gp_cmd, centre_bit );
	
		strcpy( str_buf, "size %f, %f " );
		sprintf( radius_bit, str_buf, 2*the_cut.radius(), 2*the_cut.radius() );
		strcat( gp_cmd, radius_bit );
	
		strcpy( color_bit, "units xy fc rgb \"violet\" linewidth 3 " );
		strcat( gp_cmd, color_bit );
	
		gnuplot_cmd( gp_h, gp_cmd );
		gnuplot_cmd( gp_h, "set object 100 front" );
	}

	//------------------------------------------------------------------------------------
	//draws a square cut.
	void draw_cut_square( gnuplot_ctrl *gp_h, cut_square& the_cut ){
		XB::draw_cut_polygon( gp_h, (cut_polygon&)the_cut );
	}

	//------------------------------------------------------------------------------------
	//draws an elliptical cut.
	//NOTE: May fail if some interesting transformation has been applied
	void draw_cut_ellipse( gnuplot_ctrl *gp_h, cut_ellipse& the_cut ){
		//let's declare some buffers to build the command
		char gp_cmd[160], centre_bit[32], semiaxes_bit[32];
		char angle_bit[32], color_bit[64], str_buf[64];
	
		strcpy( gp_cmd, "set object 102 ellipse " );
	
		strcpy( str_buf, "center %f, %f " );
		sprintf( centre_bit, str_buf, the_cut.C_1(), the_cut.C_2() );
		strcat( gp_cmd, centre_bit );
	
		strcpy( str_buf, "size %f, %f " );
		sprintf( semiaxes_bit, str_buf, 2*the_cut.a(), 2*the_cut.b() );
		strcat( gp_cmd, semiaxes_bit );
		
		strcpy( str_buf, "angle %f " );
		sprintf( angle_bit, str_buf, 360*the_cut.rotation()/_XB_TAU );
		strcat( gp_cmd, angle_bit );

		strcpy( color_bit, "units xy fc rgb \"violet\" linewidth 3 " );
		strcat( gp_cmd, color_bit );
	
		gnuplot_cmd( gp_h, gp_cmd );
		gnuplot_cmd( gp_h, "set object 102 front" );
	}

	//------------------------------------------------------------------------------------
	//draws a polygonal cut.
	void draw_cut_polygon( gnuplot_ctrl *gp_h, cut_polygon &the_cut ){
		//let's declare some buffers to build the command
		char *gp_cmd, size_bit[64], vertex_bit[64], color_bit[128], str_buf[128];
	
		//count the number of vertices in this polygon
		//and allocate consequently
		gp_cmd = (char*)malloc( 128 + 64*the_cut.nb_of_sides() );
		if( gp_cmd == NULL ){
			throw error( "Out of memory!", "XB::draw_cut_polygon" );
		}
	
		strcpy( gp_cmd, "set object 103 polygon " );
	
		//add all the vertices
		double *vertices = the_cut.vertices(); //aliasing!
		int n_vertices = the_cut.nb_of_sides();
		
		strcpy( str_buf, "from %f,%f " );
		sprintf( vertex_bit, str_buf, vertices[0], vertices[1] );
		strcat( gp_cmd, vertex_bit );
		for( int i=1; i <= n_vertices; ++i ){
			strcpy( str_buf, "to %f,%f " );
			sprintf( vertex_bit, str_buf,
                                 vertices[2*(i%n_vertices)],
                                 vertices[2*(i%n_vertices)+1] );
			strcat( gp_cmd, vertex_bit );
		}
	
	
		strcpy( color_bit, "fs empty border rgb \"violet\" linewidth 3 " );
		strcat( gp_cmd, color_bit );
	
		gnuplot_cmd( gp_h, gp_cmd );
		gnuplot_cmd( gp_h, "set object 103 front" );
	}

	//------------------------------------------------------------------------------------
	//draws a polygonal cut (no difference from the polygon one)
	void draw_cut_regular_polygon( gnuplot_ctrl *gp_h, cut_regular_polygon &the_cut ){
		XB::draw_cut_polygon( gp_h, (cut_polygon&)the_cut );
	}

}
