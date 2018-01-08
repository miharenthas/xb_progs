#ifndef XB_RWRITER__H
#define XB_RWRITER__H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "TFile.h"
#include "TTree.h"

#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 99, 99 )
        #include "RtypesCore.h"
#endif

#include "xb_cluster.h"

namespace XB{
        //writing rootfile for cluster data
	void rwrite( TFile* f_root_out, std::vector<XB::clusterZ> &event_klZ);
}

#endif
