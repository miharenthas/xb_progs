//this program goes back to root
//input needs to be clusters XB data

#include <iostream>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "xb_io.h"
#include "xb_root_writer.h"

using namespace std;

int main( int argc, char **argv ){
	char in_bin_name[256], out_root_name[256], stitch_f_name[256];
	bool in_flag = false, out_flag = false, verbose = false, stitch=false, debug=false;	

	//get ONE filename from the command line.
	if (argc<2) {
		puts("No options provided, I will die now since I have no purpose");
		exit( 1 );
	}

        if( argv[1][0] != '-' && strlen( argv[1] ) < 256 ){
                strcpy( in_bin_name, argv[1] );
                in_flag = true;
        }
	//input parsing
	char iota = 0;
        while( (iota = getopt( argc, argv, "i:o:s:dv") ) != -1 )
                switch( iota ){
                        case 'i' : //set an input file
                                if( strlen( optarg ) < 256 ){
                                        strcpy( in_bin_name, optarg );
                                        in_flag = true;
                                }
                                break;
                        case 'o' : //set an output file
                                if( strlen( optarg ) < 256 ){
                                        strcpy( out_root_name, optarg );
                                        out_flag = true;
                                }
                                break;
                        case 's' : //set an output file
                                if( strlen( optarg ) < 256 ){
                                        strcpy( stitch_f_name, optarg );
                                        stitch = true;
                                }
                                break;
                        case 'd' : //debug mode
                                debug = true;
                                break;
                        case 'v' : //be verbose
                                verbose = true;
                                break;
	                default:
                                printf( "-%c is not a valid option.\n", optopt );
                                printf( "usage: xb_converter [-i[FILE]|-o[FILE]|-s[FILE]|-d|-v]\n" );
                                printf( "if -i isn't specified, stdin is used\n" );
                                exit( 1 );
                }
	if( !out_flag ) {
		printf( "Please give me a name for the output root file \n" );
		exit( 1 );
	}

	if ( debug && !stitch ){
		printf( "Invalid option combo. Can't run debug mode without a file to stitch to \n" );
		exit( 1 );
	}

	//hello
	if( verbose ) printf( "*** I will try to make you a rootfile ***\n" );
	if( verbose && in_flag ) printf( "Reading from: %s...\n", in_bin_name );
	else if( verbose && !in_flag ) printf( "Reading from STDIN...\n" );

	//read in data
	std::vector<XB::data> xb_book;
	std::vector<XB::clusterZ> klz;

	if ( debug ){
		if( in_flag ) XB::load( in_bin_name, xb_book );
		else XB::load( stdin, xb_book );	
	}
	else {	
		if( in_flag ) XB::load( in_bin_name, klz );
		else XB::load( stdin, klz );
	}
	if ( verbose ) printf( "Writing file: %s \n",out_root_name);
	//write a rootfile, no stitching
	if ( !stitch ) {		
		XB::rwrite( out_root_name, klz);
	}
	else {
		if ( verbose ) printf( "Stitching to root file: %s \n",stitch_f_name );
		if ( debug ) XB::rwrite( out_root_name, stitch_f_name, xb_book );
		else XB::rwrite( out_root_name, stitch_f_name, klz, verbose );
	}
return 0;
}
