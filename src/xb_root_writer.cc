//this makes a XB cluster-level rootfile

#include "xb_root_writer.h"

//this one just writes an XBc rootfile
void XB::rwrite(char* f_root_out, std::vector<XB::clusterZ> &event_klZ ){
	//event
	unsigned int Xbevnt=0;
	unsigned int Xbcmult=0;
	//clusters in event
	unsigned int Xbcn[162],Xbci[162];
	float Xbcalt[162],Xbcaz[162],Xbcsume[162];
	//crystals in cluster
	unsigned int Xbcii[162][162];

	TFile *fout = TFile::Open(f_root_out, "recreate"); //output file
	TTree *hxbc = new TTree("hxbc", "An XB_cluster TTree"); //the XB cluster tree
	//branches
	hxbc->Branch("Xbevnt",&Xbevnt); //event number
	hxbc->Branch("Xbcmult",&Xbcmult); //number of clusters in an event
	hxbc->Branch("Xbcn",&Xbcn,"Xbcn[Xbcmult]/i"); //number of crystals in a cluster
	hxbc->Branch("Xbci",&Xbci,"Xbci[Xbcmult]/i"); //index of the centroid
	hxbc->Branch("Xbcalt",&Xbcalt,"Xbcalt[Xbcmult]/F"); //centroid altitude
	hxbc->Branch("Xbcaz",&Xbcaz,"Xbcaz[Xbcmult]/F"); //centroid azimuth
	hxbc->Branch("Xbcsume",&Xbcsume,"Xbcsume[Xbcmult]/F"); //gamma sum energy of cluster 
	hxbc->Branch("Xbcii",&Xbcii,"Xbcii[Xbcmult][162]/i");//indices of paricipating crystals, organised clusterwise

	//start eventloop
	for( unsigned int i=0; i < event_klZ.size(); ++i ){
		Xbevnt=event_klZ[i].evnt;	
		Xbcmult=event_klZ[i].n;
		if (Xbcmult>0){ //are there any clusters?
			for ( unsigned int k=0; k < event_klZ[i].n; ++k ){//loop over the clusters
				Xbcn[k]=event_klZ[i].clusters[k].n;
				Xbci[k]=event_klZ[i].clusters[k].centroid_id;
				Xbcalt[k]=event_klZ[i].clusters[k].c_altitude;
				Xbcaz[k]=event_klZ[i].clusters[k].c_azimuth;
				Xbcsume[k]=event_klZ[i].clusters[k].sum_e;
				for ( unsigned int cr=0; cr < event_klZ[i].clusters[k].n; cr++ ){//loop over crystals in cluster
					Xbcii[k][cr]=event_klZ[i].clusters[k].crys.at(cr);
				} 
			}
		}
	hxbc->Fill();
	}//end eventloop
	hxbc->Write();
	fout->Close();	
}

// this one will stitch
void XB::rwrite(char* f_root_out, char* stch_r_file, std::vector<XB::clusterZ> &event_klZ, bool v_flag){

        unsigned int Xbcmult=0;
        unsigned int Xbcn[162],Xbci[162];
        float Xbcalt[162],Xbcaz[162],Xbcsume[162];
	//crystals in cluster
	unsigned int Xbcii[162][162];

	TFile stitch_file( stch_r_file );
	if( stitch_file.IsZombie() ){
                throw XB::error( "File error!", "XB::rwrite" );
        }

	//relating to the root file we stitch the XB data to
	//get the name of the TTree
	TIter nextkey( stitch_file.GetListOfKeys() );
        TKey *key;
        TObject *obj;
        TTree *data_tree;
        while( (key = (TKey*)nextkey()) ){
                obj = key->ReadObj();
                if( obj->IsA()->InheritsFrom( TTree::Class() ) ){
                        data_tree = (TTree*)obj;
                        break;
                }
        }

	//we need the event id from the tree
	int Evnt;
	TBranch *b_Evnt;

	data_tree->SetBranchAddress("Evnt",&Evnt, &b_Evnt);	

	//clone the tree into a new root file	
	TFile* fout = new TFile( f_root_out, "recreate" );
	TTree *newtr = data_tree->CloneTree();
	//add new branches
        TBranch *bxbcmult = newtr->Branch("Xbcmult",&Xbcmult); //number of clusters in an event
        TBranch *bxbcn = newtr->Branch("Xbcn",&Xbcn,"Xbcn[Xbcmult]/i"); //number of crystals in a cluster
        TBranch *bxbci = newtr->Branch("Xbci",&Xbci,"Xbci[Xbcmult]/i"); //index of the centroid
        TBranch *bxbcalt = newtr->Branch("Xbcalt",&Xbcalt,"Xbcalt[Xbcmult]/F"); //centroid altitude
        TBranch *bxbcaz = newtr->Branch("Xbcaz",&Xbcaz,"Xbcaz[Xbcmult]/F"); //centroid azimuth
        TBranch *bxbcsume = newtr->Branch("Xbcsume",&Xbcsume,"Xbcsume[Xbcmult]/F"); //gamma sum energy of cluster
	TBranch *bxbcii = newtr->Branch("Xbcii",&Xbcii,"Xbcii[Xbcmult][162]/i");//indices of paricipating crystals, organised clusterwise

	//guess the ordering
	bool st_lth = true, cl_lth = true;
	if ( event_klZ[1].evnt > event_klZ[2].evnt ) cl_lth = false;	
	newtr->GetEvent(1);unsigned int ev1=Evnt;
	newtr->GetEvent(2);unsigned int ev2=Evnt;	
	if ( ev1 > ev2 ) st_lth = false;	

	//iterator
	unsigned int cl_i = event_klZ.size() - 1; 
	if ( st_lth && cl_lth ) cl_i = 0; 
	unsigned int v_cl_i=cl_i;

	//loop over events 
	for (int jentry=0;jentry<(int)newtr->GetEntries();jentry++){
		newtr->GetEvent(jentry);	
		if ( v_flag && (float)(jentry/25000.)==(int)(jentry/25000.) ) printf( "%i %% done \n", (int)(100.*jentry/((int)newtr->GetEntries())) );  
		bool match = false;
		//find the match in the cluster data
		while ( !match && cl_i >= 0 && cl_i < event_klZ.size() ) {
			//fix the screw up in event id uint/int between rootfile and xb data
			unsigned int tmp_id;
			memcpy( &tmp_id, &Evnt, sizeof(unsigned int));
			if ( st_lth && cl_lth ) { //same ordering
				if ( event_klZ[cl_i].evnt == tmp_id  ) match = true;
				else cl_i++;
			}
			else {	//opposite ordering
				if (  event_klZ[cl_i].evnt == tmp_id ) match = true;
                                else cl_i--;
			}
		}//end of search over cluster events
		//reset counter to last entry found
		if (match) v_cl_i=cl_i;
		else {cl_i=v_cl_i;continue;}
                Xbcmult=event_klZ[cl_i].n;
                if (Xbcmult>0){ //are there any clusters?
                        for( unsigned int k=0; k < event_klZ[cl_i].n; ++k ){//loop over the clusters
                                Xbcn[k]=event_klZ[cl_i].clusters[k].n;
                                Xbci[k]=event_klZ[cl_i].clusters[k].centroid_id;
                                Xbcalt[k]=event_klZ[cl_i].clusters[k].c_altitude;
                                Xbcaz[k]=event_klZ[cl_i].clusters[k].c_azimuth;
                                Xbcsume[k]=event_klZ[cl_i].clusters[k].sum_e;
				for ( unsigned int cr=0; cr < event_klZ[cl_i].clusters[k].n; cr++ ){//loop over all crystals in cluster
					Xbcii[k][cr]=event_klZ[cl_i].clusters[k].crys.at(cr);
				} 
                        }
                }
		//Fill the new branches
		bxbcmult->Fill();
		bxbcn->Fill();
		bxbci->Fill();
		bxbcalt->Fill();
		bxbcaz->Fill();
		bxbcsume->Fill();
		bxbcii->Fill();
	}//end of eventloop (stitch file)
	newtr->Write();
	delete newtr;
	fout->Close();
}


