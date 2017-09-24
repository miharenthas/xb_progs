//implementation of xb_reader
//NOTE: this code is ugly, because ROOT is ugly
#include "xb_reader.h"

//--------------------------------------------------------------------------
//the reader bit implementation
void XB::reader( std::vector<XB::data> &xb_book, const char* f_name ){
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
	TBranch *evnt = data_tree->GetBranch( "Evnt" ); CK_NULL( evnt, "no Evnt!", "XB::reader" );
	TBranch *xbtpat = data_tree->GetBranch( "Tpat" ); CK_NULL( evnt, "no Tpat!", "XB::reader" );
	TBranch *xbn = data_tree->GetBranch( "Xbn" ); CK_NULL( xbn, "no Xbn!", "XB::reader" );
	TBranch *xbi = data_tree->GetBranch( "Xbi" ); CK_NULL( xbi, "no Xbi!", "XB::reader" );
	TBranch *xbt = data_tree->GetBranch( "Xbt" ); CK_NULL( xbt, "no Xbt!", "XB::reader" );
	TBranch *xbpt = data_tree->GetBranch( "Xbpt" ); CK_NULL( xbpt, "no Xbpt!", "XB::reader" );
	TBranch *xbe = data_tree->GetBranch( "Xbe" ); CK_NULL( xbe, "no Xbe!", "XB::reader" );
	TBranch *xbhe = data_tree->GetBranch( "Xbhe" ); CK_NULL( xbhe, "no Xbhe!", "XB::reader" );
	TBranch *xbsume = data_tree->GetBranch( "Xbsume" ); CK_NULL( xbsume, "no Xbsume!",
                                                                     "XB::reader" );
	//These three fields are't provided in source runs, so they will be copied
	//and cheked on only in case they are provided.	
	TBranch *inbeta = data_tree->GetBranch( "Inbeta" );
	TBranch *inz = data_tree->GetBranch( "Inz" );
	TBranch *inaonz = data_tree->GetBranch( "Inaoverz" );	
	
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
		xbtpat->SetAddress( (Int_t*)&xb_book.back().tpat );
		xbi->SetAddress( (UInt_t*)xb_book.back().i );
		xbt->SetAddress( (Float_t*)xb_book.back().t );
		xbpt->SetAddress( (Float_t*)xb_book.back().pt );
		xbe->SetAddress( (Float_t*)xb_book.back().e );
		xbhe->SetAddress( (Float_t*)xb_book.back().he );
		xbsume->SetAddress( (Float_t*)&xb_book.back().sum_e );
		if( inbeta ) inbeta->SetAddress( (Float_t*)&xb_book.back().in_beta );
		if( inz ) inz->SetAddress( (Float_t*)&xb_book.back().in_Z );
		if( inaonz ) inaonz->SetAddress( (Float_t*)&xb_book.back().in_A_on_Z );
		
		//issue the copy order
		xbtpat->GetEntry( i );
		xbi->GetEntry( i );
		xbt->GetEntry( i );
		xbpt->GetEntry( i );
		xbe->GetEntry( i );
		xbhe->GetEntry( i );
		xbsume->GetEntry( i );
		if( inbeta ) inbeta->GetEntry( i );
		if( inz ) inz->GetEntry( i );
		if( inaonz ) inaonz->GetEntry( i );
		
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
void XB::reader( std::vector<XB::track_info> &xb_book, const char* f_name ){
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
	TBranch *tpat = data_tree->GetBranch( "Tpat" );
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
		tpat->SetAddress( (Int_t*)&xb_book.back().tpat );
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
		tpat->GetEntry( i );
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
void XB::arb_reader( std::vector<XB::adata> &xb_book,
                     const char *f_name, const XB::adata_field *fields ){
	//first thing first, check that the datatypes are allright
	if( !(sizeof(unsigned int) == sizeof(UInt_t)) ||
            !(sizeof(float) == sizeof(Float_t)) ){
		throw XB::error( "Quirky types!", "XB::sim_reader" );
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

	//standard branch retrieval
	TBranch *evnt = data_tree->GetBranch( "Evnt" ); CK_NULL( evnt, "no Evnt!", "XB::reader" );
	TBranch *tpat = data_tree->GetBranch( "Tpat" ); CK_NULL( evnt, "no Tpat!", "XB::reader" );
	//These three fields are't provided in source runs, so they will be copied
	//and cheked on only in case they are provided.	
	TBranch *inz = data_tree->GetBranch( "Inz" );
	TBranch *inaonz = data_tree->GetBranch( "Inaoverz" );
	
	//dynamic branch retrival
	int nf = 0; while( fields[nf].size ) ++nf;
	TBranch **branches = (TBranch**)malloc( nf*sizeof(TBranch**));
	for( int i=0; i < nf; ++i ){
		branches[i] = data_tree->GetBranch( fields[i].name );
		if( !branches[i] ) throw XB::error( "No field!", "XB::arb_reader" );
	}
	
	//do the various associations
	int nb_entries = data_tree->GetEntries(), field_sz;
	void *field_bf = malloc( 1 );
	for( int i=0; i < nb_entries; ++i ){
		xb_book.push_back( XB::adata() );
		
		//addressing
		evnt->SetAddress( (Int_t*)&xb_book.back().evnt );
		tpat->SetAddress( (Int_t*)&xb_book.back().tpat );
		if( inz ) inz->SetAddress( (Float_t*)&xb_book.back().in_Z );
		if( inaonz ) inaonz->SetAddress( (Float_t*)&xb_book.back().in_A_on_Z );
		branches[0]->SetAddress( (UInt_t*)&xb_book.back().n );

		//copying
		evnt->GetEntry( i );
		tpat->GetEntry( i );
		if( inz ) inz->GetEntry( i );
		if( inaonz ) inaonz->GetEntry( i );
		branches[0]->GetEntry( i );
		
		for( int f=1; f < nf; ++f ){
			field_sz = fields[f].size*xb_book.back().n;
			field_bf = realloc( field_bf, field_sz );
			branches[f]->SetAddress( (Float_t*)field_bf );
			branches[f]->GetEntry( i );
			xb_book.back().dofield( fields[f].name, field_sz, field_bf );
		}
	}		
	free( field_bf );
	
	f.Close();
}

//std::string interface...
void XB::arb_reader( std::vector<XB::adata> &xb_book,
                     std::string f_name, const XB::adata_field *fields ){
	XB::arb_reader( xb_book, f_name.c_str(), fields );
}			

//------------------------------------------------------------------------------------
//the simulation reader implementation
void XB::sim_reader( std::vector<XB::data> &xb_book, const char *f_name ){
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
void XB::sim_reader( std::vector<XB::track_info> &xb_book, const char *f_name ){ return; }
		
