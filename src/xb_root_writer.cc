//this makes a XB cluster-level rootfile

#include "xb_root_writer.h"
unsigned int default_beam_out = 81; 

//this one just writes an XBc rootfile
void XB::rwrite(char* f_root_out, std::vector<XB::clusterZ> &event_klZ ){
	xb_ball the_cb;
	//event
	unsigned int Xbevnt=0;
	unsigned int Xbcmult=0;
	//clusters in event
	unsigned int Xbcn[162],Xbci[162];
	float Xbcth[162],Xbcsume[162];

	unsigned int default_beam_out = 81; //the default beam in
	//sort the xb cluster data by event id before writing
	std::sort( event_klZ.begin(), event_klZ.end(), evnt_id_comparison );

	TFile *fout = TFile::Open(f_root_out, "recreate"); //output file
	TTree *hxbc = new TTree("hxbc", "An XB_cluster TTree"); //the XB cluster tree
	//branches
	hxbc->Branch("Xbevnt",&Xbevnt); //event number
	hxbc->Branch("Xbcmult",&Xbcmult); //number of clusters in an event
	hxbc->Branch("Xbcn",&Xbcn,"Xbcn[Xbcmult]/i"); //number of crystals in a cluster
	hxbc->Branch("Xbci",&Xbci,"Xbci[Xbcmult]/i"); //index of the centroid
	hxbc->Branch("Xbcth",&Xbcth,"Xbcth[Xbcmult]/F"); //centroid theta ( inclination towards beam line )
	hxbc->Branch("Xbcsume",&Xbcsume,"Xbcsume[Xbcmult]/F"); //gamma sum energy of cluster 

	//start eventloop
	for( unsigned int i=0; i < event_klZ.size(); ++i ){
		Xbevnt=event_klZ[i].evnt;	
		Xbcmult=event_klZ[i].n;
		if (Xbcmult>0){ //are there any clusters?
			for ( unsigned int k=0; k < event_klZ[i].n; ++k ){//loop over the clusters
				Xbcn[k]=event_klZ[i].clusters[k].n;
				Xbci[k]=event_klZ[i].clusters[k].centroid_id;
				Xbcth[k]=angular_distance( the_cb.at( default_beam_out ).altitude, the_cb.at( default_beam_out ).azimuth, 
						event_klZ[i].clusters[k].c_altitude, event_klZ[i].clusters[k].c_azimuth );
				Xbcsume[k]=event_klZ[i].clusters[k].sum_e;
			}
		}
	hxbc->Fill();
	}//end eventloop
	hxbc->Write();
	fout->Close();	
}

// this one will stitch
void XB::rwrite(char* f_root_out, char* stch_r_file, std::vector<XB::clusterZ> &event_klZ, bool v_flag){
	xb_ball the_cb;
        unsigned int Xbcmult=0;
        unsigned int Xbcn[162],Xbci[162];
        float Xbcth[162],Xbcsume[162];
	float Xbct[162];
	//crystals in cluster
	unsigned int Xbii[162]={0}; 

	unsigned int default_beam_out = 81; //the default beam in

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
	unsigned int Xbn;
	unsigned int Xbi[162];
	float Xbe[162];
	float Xbt[162];
	TBranch *b_Evnt;
	TBranch *b_Xbn;
	TBranch *b_Xbi;
	TBranch *b_Xbe;
	TBranch *b_Xbt;

	data_tree->SetBranchAddress("Evnt",&Evnt, &b_Evnt);	
	data_tree->SetBranchAddress("Xbn",&Xbn, &b_Xbn);	
	data_tree->SetBranchAddress("Xbi",&Xbi, &b_Xbi);	
	data_tree->SetBranchAddress("Xbe",&Xbe, &b_Xbe);	
	data_tree->SetBranchAddress("Xbt",&Xbt, &b_Xbt);	

	//clone the tree into a new root file	
	TFile* fout = new TFile( f_root_out, "recreate" );
	TTree *newtr = data_tree->CloneTree(0);
	//add new branches
        TBranch *bxbcmult = newtr->Branch("Xbcmult",&Xbcmult); //number of clusters in an event
        TBranch *bxbcn = newtr->Branch("Xbcn",&Xbcn,"Xbcn[Xbcmult]/i"); //number of crystals in a cluster
        TBranch *bxbci = newtr->Branch("Xbci",&Xbci,"Xbci[Xbcmult]/i"); //index of the centroid
        TBranch *bxbth = newtr->Branch("Xbcth",&Xbcth,"Xbcth[Xbcmult]/F"); //centroid altitude
        TBranch *bxbcsume = newtr->Branch("Xbcsume",&Xbcsume,"Xbcsume[Xbcmult]/F"); //gamma sum energy of cluster
        TBranch *bxbct = newtr->Branch("Xbct",&Xbct,"Xbct[Xbcmult]/F"); //time of cluster
	TBranch *bxbii = newtr->Branch("Xbii",&Xbii,"Xbii[Xbn]/i");//cluster assignment of crystal

	//number of events
	int nevents=data_tree->GetEntries();

	//guess the ordering
	int ord=0;
	srand(nevents);
	for ( int chk=0;chk<10;chk++){  
		int ri=rand()%(nevents-1);	
		data_tree->GetEvent( ri );unsigned int ev1=Evnt;
		data_tree->GetEvent( ri+1 );unsigned int ev2=Evnt;	
		if ( ev1 > ev2 ) ord--;
		else ord++;
	}

	if ( ord==-10 && v_flag ) puts( "Rootfile has reverse ordering by event id " );
	else if ( ord==10 && v_flag ) puts( "Rootfile is sorted by event id " );
	else if ( v_flag ) puts ( "Rootfile is not ordered, this will be slow "); 

	//sort the xb cluster data by event id (it's probably sorted already) 
	std::sort( event_klZ.begin(), event_klZ.end(), evnt_id_comparison );
	//reverse if root file is sorted oppositely (shouldn't be the case, but anyway...)
	if ( ord==-10 ) std::reverse( event_klZ.begin(), event_klZ.end() );

	//iterator
	unsigned int cl_i = 0; 
	unsigned int v_cl_i=cl_i; 

	//loop over events 
	int debug=0;
	int debug_total=0;
	for (int jentry=0;jentry<nevents;jentry++){
		data_tree->GetEvent(jentry);	
		if ( v_flag && (float)(jentry/25000.)==(int)(jentry/25000.) ) printf( "%i %% done, entry %i \n", (int)(100.*jentry/nevents), cl_i );  
		for (unsigned int sx=0;sx<Xbn;sx++) {
			//if (!isnan(Xbe[sx])&&!isinf(Xbe[sx])) debug_total++;
			debug_total++;
			Xbii[sx]=0;
		}
		bool match = false;
		//find the match in the cluster data
		while ( !match && cl_i >= 0 && cl_i < event_klZ.size() ) {
			if ( event_klZ[cl_i].evnt ==  Evnt  ) match = true;
			else cl_i++;
		}//end of search over cluster events
		//reset counter to last entry found
		if ( match&&abs(ord)==10 ) v_cl_i=cl_i;
		else if ( abs(ord)==10 ) cl_i=v_cl_i;
		else cl_i=0;
		if (match){
                	Xbcmult=event_klZ[cl_i].n;
                	if (Xbcmult>0){ //are there any clusters?
                        	for( unsigned int k=0; k < event_klZ[cl_i].n; ++k ){//loop over the clusters
                                	Xbcn[k]=event_klZ[cl_i].clusters[k].n;
                                	Xbci[k]=event_klZ[cl_i].clusters[k].centroid_id;
                                	Xbcth[k]=angular_distance( the_cb.at( default_beam_out ).altitude, the_cb.at( default_beam_out ).azimuth,
                                                event_klZ[cl_i].clusters[k].c_altitude, event_klZ[cl_i].clusters[k].c_azimuth );
                                	Xbcsume[k]=event_klZ[cl_i].clusters[k].sum_e;
					Xbct[k]=0;
					for ( unsigned int cr=0; cr < event_klZ[cl_i].clusters[k].n; cr++ ){//loop over all crystals in cluster
						for ( unsigned int scr=0;scr<Xbn;scr++ ){//loop over all crystals fired
							if ( Xbi[scr]==event_klZ[cl_i].clusters[k].crys.at(cr) && Xbi[scr]>0 && Xbi[scr]<163 ) {
							//	if (!isnan(Xbe[scr]) && !isinf(Xbe[scr])) {
									debug++;
									Xbii[scr]=event_klZ[cl_i].clusters[k].centroid_id;
									if (Xbct[k]==0) Xbct[k]=Xbt[scr];
							//	}
							}
						}
					} 
				}
                	}
		}
		else Xbcmult=0;
		//Fill the tree
		newtr->Fill();
	}//end of eventloop (stitch file)
	if ( v_flag ) Printf("Assigned crystals: %f %\n",(100.*debug)/debug_total);
	newtr->Write();
	delete newtr;
	fout->Close();
}
//debugging purposes, just write out what was read in from the original rootfile

void XB::rwrite( char* f_root_out, char* stch_r_file, std::vector<XB::data> &xb_book ){
	
	unsigned int Xbn_debug;
	float Inz_debug;
	unsigned int Xbi_debug[162];
	float Xbe_debug[162];
	float Xbt_debug[162];

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
        unsigned int Xbn;
        float Xbe[162];
        TBranch *b_Evnt;
        TBranch *b_Xbn;
        TBranch *b_Xbe;

        data_tree->SetBranchAddress("Evnt",&Evnt, &b_Evnt);
        data_tree->SetBranchAddress("Xbn",&Xbn, &b_Xbn);
        data_tree->SetBranchAddress("Xbe",&Xbe, &b_Xbe);

        //clone the tree into a new root file   
        TFile* fout = new TFile( f_root_out, "recreate" );
        TTree *newtr = data_tree->CloneTree(0);
        //add new branches
	TBranch *bxbn_debug = newtr->Branch("Xbn_debug",&Xbn_debug);
	TBranch *binz_debug = newtr->Branch("Inz_debug",&Inz_debug);
	TBranch *bxbi_debug = newtr->Branch("Xbi_debug",&Xbi_debug,"Xbi_debug[Xbn_debug]/i");
	TBranch *bxbe_debug = newtr->Branch("Xbe_debug",&Xbe_debug,"Xbe_debug[Xbn_debug]/F");
	TBranch *bxbt_debug = newtr->Branch("Xbt_debug",&Xbt_debug,"Xbt_debug[Xbn_debug]/F");

        //number of events
        int nevents=data_tree->GetEntries();

        //guess the ordering
        int ord=0;
        srand(nevents);
        for ( int chk=0;chk<10;chk++){
                int ri=rand()%(nevents-1);
                data_tree->GetEvent( ri );unsigned int ev1=Evnt;
                data_tree->GetEvent( ri+1 );unsigned int ev2=Evnt;
                if ( ev1 > ev2 ) ord--;
                else ord++;
        }

        if ( ord==-10 ) puts( "Rootfile has reverse ordering by event id " );
        else if ( ord==10 ) puts( "Rootfile is sorted by event id " );
        else  puts ( "Rootfile is not ordered, this will be slow ");

        //sort the xb data by event id (it's probably sorted already) 
        std::sort( xb_book.begin(), xb_book.end(), evnt_id_comparison );
        //reverse if root file is sorted oppositely (shouldn't be the case, but anyway...)
        if ( ord==-10 ) std::reverse( xb_book.begin(), xb_book.end() );

        //iterator
        unsigned int ci = 0;
        unsigned int v_ci=ci;
	//declare the energy list
        XB::oed *energy_list;

	unsigned int debug=0;

        //loop over events 
        for (int jentry=0;jentry<nevents;jentry++){
                data_tree->GetEvent(jentry);
		Xbn_debug=0;
		Inz_debug=0;
		for (int rst=0;rst<162;rst++) {Xbi_debug[rst]=0,Xbe_debug[rst]=0,Xbt_debug[rst]=0;}
                if ( (float)(jentry/25000.)==(int)(jentry/25000.) ) printf( "%i %% done \n", (int)(100.*jentry/nevents) );
                //find the match in the data
		bool match = false;
                while ( !match && ci >= 0 && ci < xb_book.size() ) {
                        if ( xb_book[ci].evnt ==  Evnt  ) match = true;
                        else ci++;
                }//end of search over events
                //reset counter to last entry found
                if ( match&&abs(ord)==10 ) v_ci=ci;
                else if ( abs(ord)==10 ) ci=v_ci;
                else ci=0;
		if ( match ){
			Inz_debug=xb_book[ci].in_Z;
			Xbn_debug=xb_book[ci].n;	
			//energy_list = XB::make_energy_list( xb_book[ci] );
			for (unsigned int j=0;j<Xbn_debug;j++){
				Xbi_debug[j]=xb_book[ci].i[j];
				Xbe_debug[j]=xb_book[ci].e[j];
				Xbt_debug[j]=xb_book[ci].t[j];
				//Xbi_debug[j]=energy_list[j].i;
				//Xbe_debug[j]=energy_list[j].e;
				//Xbt_debug[j]=energy_list[j].t;
			}
		}
		else if ( !match && Xbn>0) { debug++;}
		newtr->Fill();
	}
	newtr->Write();
        delete newtr;
        fout->Close();
	Printf( "Couldn't find a match in %i %% of cases\n", (100*debug)/nevents );
}

//the comparison utility
//for the event holder structure
bool evnt_id_comparison( const XB::event_holder &one, const XB::event_holder &two ){
        return one.evnt < two.evnt;
}

