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
	char in_bin_name[256], out_root_name[256];
	bool in_flag = false, out_flag = false, verbose = false;	

	//get ONE filename from the command line.
	if (argc<2) {
		puts("No options provided, I will die now since I have no purpose");
		return 1;
	}

        if( argv[1][0] != '-' && strlen( argv[1] ) < 256 ){
                strcpy( in_bin_name, argv[1] );
                in_flag = true;
        }
	//input parsing
	char iota = 0;
        while( (iota = getopt( argc, argv, "i:o:v") ) != -1 )
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
                        case 'v' : //be verbose
                                verbose = true;
                                break;
	                default:
                                printf( "-%c is not a valid option.\n", optopt );
                                printf( "usage: xb_converter [-i[FILE]|-o[FILE]|-v]\n" );
                                printf( "if -i isn't specified, stdin is used\n" );
                                exit( 1 );
                }
	if( !out_flag ) {
		printf( "Please give me a name for the output\n" );
		return 1;
	}
	//hello
	if( verbose ) printf( "*** I will try to make you a rootfile ***\n" );
	if( verbose && in_flag ) printf( "Reading from: %s...\n", in_bin_name );
	else if( verbose && !in_flag ) printf( "Reading from STDIN...\n" );

	//read in clustered data
	std::vector<XB::clusterZ> klz;
	if( in_flag ) XB::load( in_bin_name, klz );
	else XB::load( stdin, klz );



return 0;
}
