//this function reads data of the XB out of a .root file generated with paw_ntuple.
//WARNING, this is the only piece of the XB namespace that still dialogates directl with ROOT
//use at your own peril
#ifndef XB_READER__H
#define XB_READER__H

#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>

#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TObject.h"
#include "TClonesArray.h"

#include "R3BXBallCrystalHitSim.h"
#include "R3BXBallPoint.h"

//a very ugly check on ROOT version
//because the "machine independent types" are defined
//in a header only starting from ROOT 6.xx.xx
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 99, 99 )
	#include "RtypesCore.h"
#endif

#include "xb_data.h"
#include "xb_arbitrary_data.h"
#include "xb_error.h"

#define CK_NULL( ptr, what, who ) if( !ptr ) throw XB::error( what, who );

namespace XB{
	//read the data
	void reader( std::vector<XB::data> &xb_book, std::string f_name );
	void reader( std::vector<XB::data> &xb_book, const char *f_name );
	
	//read the tracker output
	void reader( std::vector<XB::track_info> &xb_book, std::string f_name );
	void reader( std::vector<XB::track_info> &xb_book, const char *f_name );

	//read the simulation data
	void sim_reader( std::vector<XB::data> &xb_book, std::string f_name );
	void sim_reader( std::vector<XB::data> &xb_book, const char *f_name );

	//read the simulation track (for now: dummy)
	void sim_reader( std::vector<XB::track_info> &xb_book, std::string f_name );
	void sim_reader( std::vector<XB::track_info> &xb_book, const char *f_name );

	//read arbitrary data
	//The big new part is the fields argument
	//which is a null terminated array of fields.
	//NOTE: For now, the first field will be the
	//      size info for the other ones.
	/*****************************************************************************
	EXAMPLE:
	adata_field fields[] = {
		{ "Xbn", 4 }, --> Xbn is 4 bytes long (float and int),
		                  will be stored in .n
		{ "Xbpt", 4 }, --> Xbpt is 4 by the _value_ of Xbn long,
		                   Will be stored in its own field
		{ NULL, 0 } --> Is the end of the array.
	};
	*****************************************************************************/
	void arb_reader( std::vector<XB::adata> &xb_book,
	                 std::string f_name, const adata_field *fields );
	void arb_reader( std::vector<XB::adata> &xb_book,
	                 const char *f_name, const adata_field *fields );
}
#endif
