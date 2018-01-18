#ifndef XB_RWRITER__H
#define XB_RWRITER__H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TKey.h"
#include "TObject.h"
#include "TClonesArray.h"

#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 99, 99 )
        #include "RtypesCore.h"
#endif

#include "xb_cluster.h"
#include "xb_doppc.h"

namespace XB{
        //writing rootfile for cluster data
	void rwrite( char* f_root_out, std::vector<XB::clusterZ> &event_klZ);
	//write and stitch 
	void rwrite( char* f_root_out, char* stch_r_file, std::vector<XB::clusterZ> &event_klZ, bool v_flag=false);
}

#endif
