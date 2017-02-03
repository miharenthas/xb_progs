//implementation of xb_reader
#include "xb_reader.h"

//--------------------------------------------------------------------------
//the reader bit implementation
void XB::reader( std::vector<XB::data> &xb_book, char* f_name ){
	//first thing first, check that the datatypes are allright
	if( !(sizeof(unsigned int) == sizeof(UInt_t)) ||
            !(sizeof(float) == sizeof(Float_t)) ){
		throw XB::error( "Quirky types!", "XB::reader" );
	}

	//open the usual file
	TFile f( f_name );

	if( f.IsZombie() ){
		throw XB::error( "File error!", "XB::reader" );
	}
	
	//work out what is the name of the tree
	TIter nextkey( f.GetListOfKeys() );
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
	
	//let's get ready to read stuff
	//thats utter braindamage
	TBranch *evnt = data_tree->GetBranch( "Evnt" );
	TBranch *xbn = data_tree->GetBranch( "Xbn" );
	TBranch *xbi = data_tree->GetBranch( "Xbi" );
	TBranch *xbt = data_tree->GetBranch( "Xbt" );
	TBranch *xbpt = data_tree->GetBranch( "Xbpt" );
	TBranch *xbe = data_tree->GetBranch( "Xbe" );
	TBranch *xbhe = data_tree->GetBranch( "Xbhe" );
	TBranch *xbsume = data_tree->GetBranch( "Xbsume" );
	TBranch *inbeta = data_tree->GetBranch( "Inbeta" );
	
	
	//reader loop
	unsigned int n_on_this = 0;
	int evnt_id = 0;
	int n_frags = 0;
	//associate the tbranch to the number of cative crystals
	//and the event numebr and the fragment number
	xbn->SetAddress( (UInt_t*)&n_on_this );
	evnt->SetAddress( (Int_t*)&evnt_id );
	for( int i=0; i < (int)data_tree->GetEntries(); ++i ){
		xbn->GetEntry( i );
		//check that it's not 0
		if( n_on_this == 0 ) continue;
		
		//get the things into the two indices
		evnt->GetEntry( i );
		xbn->GetEntry( i );
		
		//create the new book page (haha, funny)
		xb_book.push_back( XB::data( n_on_this, (unsigned int)evnt_id ) );
		
		//fill it:
		//associate the branches
		xbi->SetAddress( (UInt_t*)xb_book.back().i );
		xbt->SetAddress( (Float_t*)xb_book.back().t );
		xbpt->SetAddress( (Float_t*)xb_book.back().pt );
		xbe->SetAddress( (Float_t*)xb_book.back().e );
		xbhe->SetAddress( (Float_t*)xb_book.back().he );
		xbsume->SetAddress( (Float_t*)&xb_book.back().sum_e );
		inbeta->SetAddress( (Float_t*)&xb_book.back().in_beta );
		
		//issue the copy order
		xbi->GetEntry( i );
		xbt->GetEntry( i );
		xbpt->GetEntry( i );
		xbe->GetEntry( i );
		xbhe->GetEntry( i );
		xbsume->GetEntry( i );
		inbeta->GetEntry( i );
		
		//probe for nans
		xb_book.back().probe_for_crap();
	}
	
	//close the file
	f.Close();
}

//std::string interface...
void XB::reader( std::vector<XB::data> &xb_book, std::string f_name ){
	XB::reader( xb_book, f_name.c_str() );
}

//--------------------------------------------------------------------------
//the reader bit implementation
void XB::reader( std::vector<XB::track_info> &xb_book, char* f_name ){
	//first thing first, check that the datatypes are allright
	if( !(sizeof(unsigned int) == sizeof(UInt_t)) ||
            !(sizeof(float) == sizeof(Float_t)) ){
		throw XB::error( "Quirky types!", "XB::reader" );
	}

	//open the usual file
	TFile f( f_name );

	if( f.IsZombie() ){
		throw XB::error( "File error!", "XB::reader" );
	}

	//work out what is the name of the tree
	TIter nextkey( f.GetListOfKeys() );
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
	
	//let's get ready to read stuff
	//thats utter braindamage
	TBranch *evnt = data_tree->GetBranch( "Evnt" );
	TBranch *inbeta = data_tree->GetBranch( "Inbeta" );
	TBranch *inbeta0 = data_tree->GetBranch( "Inbeta0" );
	TBranch *inz = data_tree->GetBranch( "Inz" );
	TBranch *inaoverz = data_tree->GetBranch( "Inaoverz" );
	TBranch *framul = data_tree->GetBranch( "fra_mul" );
	TBranch *fraa = data_tree->GetBranch( "fra_A" );
	TBranch *fraz = data_tree->GetBranch( "fra_Z" );
	TBranch *frabeta = data_tree->GetBranch( "fra_beta" );
	TBranch *in_dir[] = { data_tree->GetBranch( "Indx" ),
	                      data_tree->GetBranch( "Indy" ),
	                      data_tree->GetBranch( "Indz" ) };
	TBranch *out_dir[] = { data_tree->GetBranch( "fra_dx" ),
	                       data_tree->GetBranch( "fra_dy" ),
	                       data_tree->GetBranch( "fra_dz" ) };
	
	
	//reader loop
	int evnt_id = 0;
	int n_frags = 0;
	float *dir_in, *dir_out; //two buffers where to save all the directions of the fragments
	//associate the tbranch to the number of cative crystals
	//and the event numebr and the fragment number
	evnt->SetAddress( (Int_t*)&evnt_id );
	framul->SetAddress( (Int_t*)&n_frags );
	for( int i=0; i < (int)data_tree->GetEntries(); ++i ){
		//get the things into the two indices
		evnt->GetEntry( i );
		framul->GetEntry( i );
		
		//check that it's not 0
		if( n_frags == 0 ) continue;
		
		//allocate the buffer for the directions
		dir_in = new float[3*n_frags];
		dir_out = new float[3*n_frags];
		
		//create the new book page (haha, funny)
		xb_book.push_back( XB::track_info( n_frags, (unsigned int)evnt_id ) );
		
		//fill it:
		//associate the branches
		inbeta->SetAddress( (Float_t*)&xb_book.back().in_beta );
		inbeta0->SetAddress( (Float_t*)&xb_book.back().beta_0 );
		inz->SetAddress( (Float_t*)&xb_book.back().in_Z );
		inaoverz->SetAddress( (Float_t*)&xb_book.back().in_A_on_Z );
		fraa->SetAddress( (Float_t*)xb_book.back().fragment_A );
		fraz->SetAddress( (Float_t*)xb_book.back().fragment_Z );
		frabeta->SetAddress( (Float_t*)xb_book.back().fragment_beta );
		for( int j=0; j < 3; ++j ){
			in_dir[j]->SetAddress( (Float_t*)&dir_in[j*n_frags] );
			out_dir[j]->SetAddress( (Float_t*)&dir_out[j*n_frags] );
		}
		
		
		//issue the copy order
		inbeta->GetEntry( i );
		inbeta0->GetEntry( i );
		inz->GetEntry( i );
		inaoverz->GetEntry( i );
		fraa->GetEntry( i );
		fraz->GetEntry( i );
		frabeta->GetEntry( i );
		for( int j=0; j < 3; ++j ){
			in_dir[j]->GetEntry( i );
			out_dir[j]->GetEntry( i );
		}
		
		//actually copy the directions
		for( int f=0; f < n_frags; ++f ){
			xb_book.back().incoming[f].i = dir_in[f];
			xb_book.back().incoming[f].j = dir_in[f+n_frags];
			xb_book.back().incoming[f].k = dir_in[f+n_frags<<1];
			xb_book.back().outgoing[f].i = dir_out[f];
			xb_book.back().outgoing[f].j = dir_out[f+n_frags];
			xb_book.back().outgoing[f].k = dir_out[f+n_frags<<1];
		}
		
		//cleanup
		delete[] dir_in;
		delete[] dir_out;
	}
	
	//close the file
	f.Close();
}

//std::string interface...
void XB::reader( std::vector<XB::track_info> &xb_book, std::string f_name ){
	XB::reader( xb_book, f_name.c_str() );
}

//------------------------------------------------------------------------------------
//the simulation reader implementation
void XB::sim_reader( std::vector<XB::data> &xb_book, char *f_name ){
	//first thing first, check that the datatypes are allright
	if( !(sizeof(unsigned int) == sizeof(UInt_t)) ||
            !(sizeof(float) == sizeof(Float_t)) ){
		throw XB::error( "Quirky types!", "XB::sim_reader" );
	}

	//open the usual file
	TFile f( f_name );

	if( f.IsZombie() ){
		throw XB::error( "File error!", "XB::sim_reader" );
	}

	//work out what is the name of the tree
	TIter nextkey( f.GetListOfKeys() );
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

	//get the number of events and the number of tracks for the first entry
	int n_events = data_tree->GetEntries(), n_tracks = 0;
	if( !n_events ) throw XB::error( "Empty tree!", "XB::sim_reader" ); //check for non
	                                                                    //emptiness

	//associate the target branch to a pointer
	TClonesArray buf( "R3BXBallCrystalHitSim", 4096 ), *p_buf; //the clones array buffer
	                                                           //and a pointer
	p_buf = &buf;
	int rc = data_tree->SetBranchAddress( "XBCrystalHitSim", &p_buf );
	if( rc ) throw XB::error( "Branch not found!", "XB::sim_reader" );

	//a place to handle the data
	R3BXBallCrystalHitSim *p_data;
	
	//loop on the data
	for( int i=0; i < n_events; ++i ){
		data_tree->GetBranch( "XBCrystalHitSim" )->GetEntry( i ); //retrieve the entry
		
		n_tracks = buf.GetEntries();
		xb_book.push_back( XB::data( n_tracks, i ) );
		
		for( int t=0; t < n_tracks; ++t ){
			p_data = (R3BXBallCrystalHitSim*)buf.At( t );
			
			xb_book.back().i[t] = p_data->GetCrystalNumber();
			xb_book.back().t[t] = p_data->GetTime();
			xb_book.back().e[t] = 1e6*p_data->GetEnergy(); //GeV to KeV
		}
		
		//this basically sets the flags correctly, here
		xb_book.back().probe_for_crap();
	}
	
	f.Close();
}

//std::string interface...
void XB::sim_reader( std::vector<XB::data> &xb_book, std::string f_name ){
	XB::sim_reader( xb_book, f_name.c_str() );
}

//------------------------------------------------------------------------------------
//dummy track readers
void XB::sim_reader( std::vector<XB::track_info> &xb_book, std::string f_name ){ return; }
void XB::sim_reader( std::vector<XB::track_info> &xb_book, char *f_name ){ return; }
		
