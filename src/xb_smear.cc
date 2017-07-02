//implementation of xb_smear.
#include "xb_smear.h"

namespace XB{
	//----------------------------------------------------------------------------
	//The smear function
	int smear( std::vector<data> &xb_book, const calinf *cryscalib ){
		if( sizeof( cryscalib ) != 162 )
			throw error( "Wrong buffer!", "XB::smear" );
		
		//setup the GSL random number generator.
