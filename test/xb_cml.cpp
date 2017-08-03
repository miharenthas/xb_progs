//a tiny test program to check that the command line for xb_make_spc is
//behaving properly.

#include <stdio.h>
#include <string.h>

#include "xb_make_spc/xb_make_spc.h"
#include "xb_make_spc/cmd_line.h"

void print_settings( FILE *target, p_opts &settings );

int main( int argc, char **argv ){
	p_opts settings;
	memset( &settings, 0, sizeof( p_opts ) ); //make sure it's 0-flat.
	settings.drone_flag = 1;

	puts( "Command line test program. Knock yourself out." );
	
	int breaker = DO_EXECUTE;
	while( breaker != DO_EXIT ){
		print_settings( stdout, settings );
		__HERE__:
		try{
			breaker = XB::cml_loop_prompt( stdin, settings );
		} catch( XB::error e ){
			puts( e.what() );
			goto __HERE__;
		}
	}
	
	puts( "DO_EXIT set, bye." );
	return 0;
}

void print_settings( FILE *target, p_opts &settings ){
	for( int i=0; i < settings.in_f_count; ++i )
		fputs( settings.in_fname[i], target );
	fputs( settings.out_fname, target );
	if( settings.in_flag ) fputs( "in_flag\n", target );
	if( settings.out_flag ) fputs( "out_flag\n", target );
	if( settings.draw_flag ) fputs( "draw_flag\n", target );
	if( settings.verbose ) fputs( "verbose\n", target );
	if( settings.interactive ) fputs( "interactive\n", target );
	fprintf( target, "in_f_count: %u\n", settings.in_f_count );
	fprintf( target, "num_bins: %u\n", settings.num_bins );
	fprintf( target, "target_mul: %u\n", settings.target_mul );
	fprintf( target, "target_ctr: %u\n", settings.target_ctr );
	fprintf( target, "target_cry: %u\n", settings.target_cry );
	fprintf( target, "target_alt: %u\n", settings.target_alt );
	fprintf( target, "target_azi: %u\n", settings.target_azi );
	fprintf( target, "target_nrg: %u\n", settings.target_nrg );
	fprintf( target, "mol_mul: %d\n", settings.mol_mul );
	fprintf( target, "mol_ctr: %d\n", settings.mol_ctr );
	fprintf( target, "mol_cry: %d\n", settings.mol_cry );
	fprintf( target, "mol_alt: %d\n", settings.mol_alt );
	fprintf( target, "mol_azi: %d\n", settings.mol_azi );
	fprintf( target, "mol_nrg: %d\n", settings.mol_nrg );
	fprintf( target, "range: %f:%f\n", settings.range[0], settings.range[1] );
	fprintf( target, "histo_mode: %d\n", settings.histo_mode );
	
	fprintf( target, "gp_opt.term %d\n", settings.gp_opt.term );
	if( settings.gp_opt.is_log ) fputs( "gp_opt.is_log\n", target );
	fputs( settings.gp_opt.title, target );
	fputs( settings.gp_opt.x_label, target );
	fputs( settings.gp_opt.y_label, target );
	fputs( settings.gp_opt.out_fname, target );
	
	fputs( settings.drone.instream, target ); fputc( '\n', target );
	fputs( settings.drone.outstream, target ); fputc( '\n', target );
	fputc( settings.drone.in_pof, target ); fputc( '\n', target );
	fputc( settings.drone.out_pof, target ); fputc( '\n', target );
}
	
