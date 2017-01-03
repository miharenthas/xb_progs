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
#include "xb_error.h"

namespace XB{
	//read the data
	void reader( std::vector<XB::data*> &xb_book, std::string f_name );
	void reader( std::vector<XB::data*> &xb_book, char *f_name );
	
	//read the tracker output
	void reader( std::vector<XB::track_info*> &xb_book, std::string f_name );
	void reader( std::vector<XB::track_info*> &xb_book, char *f_name );

	//read the simulation data
	void sim_reader( std::vector<XB::data*> &xb_book, std::string f_name );
	void sim_reader( std::vector<XB::data*> &xb_book, char *f_name );
}
#endif
