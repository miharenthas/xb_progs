//this makes a XB cluster-level rootfile

#include "xb_root_writer.h"

void XB::rwrite(char* f_root_out, std::vector<XB::clusterZ> &event_klZ ){

	unsigned int Xbevnt=0;
	unsigned int Xbcmult=0;
	unsigned int Xbcn[160],Xbci[160];
	float Xbcalt[160],Xbcaz[160],Xbsume[160];

	TFile *fout = TFile::Open(f_root_out, "recreate"); //output file
	TTree *hxbc = new TTree("hxbc", "An XB_cluster TTree"); //the XB cluster tree
	//branches
	hxbc->Branch("Xbevnt",&Xbevnt); //event number
	hxbc->Branch("Xbcmult",&Xbcmult); //number of clusters in an event
	hxbc->Branch("Xbcn",&Xbcn,"Xbcn[Xbcmult]/i"); //number of crystals in a cluster
	hxbc->Branch("Xbci",&Xbci,"Xbci[Xbcmult]/i"); //index of the centroid
	hxbc->Branch("Xbcalt",&Xbcalt,"Xbcalt[Xbcmult]/F"); //centroid altitude
	hxbc->Branch("Xbcaz",&Xbcaz,"Xbcaz/F"); //centroid azimuth
	hxbc->Branch("Xbsume",&Xbsume,"Xbsume/F"); //gamma sum energy of cluster
	
	//start eventloop
	for( int i=0; i < event_klZ.size(); ++i ){
		Xbevnt=event_klZ[i].evnt;	
		Xbcmult=event_klZ[i].n;
		if (Xbcmult>0){ //are there any clusters?
			for( int k=0; k < event_klZ[i].n; ++k ){//loop over the clusters
				Xbcn[k]=event_klZ[i].clusters[k].n;
				Xbci[k]=event_klZ[i].clusters[k].centroid_id;
				Xbcalt[k]=event_klZ[i].clusters[k].c_altitude;
				Xbcaz[k]=event_klZ[i].clusters[k].c_azimuth;
				Xbsume[k]=event_klZ[i].clusters[k].sum_e;
			}
		}
	hxbc->Fill();
	}//end eventloop
	hxbc->Write();
	fout->Close();	
}

