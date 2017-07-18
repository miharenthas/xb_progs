%This is the main script (finally under development).
%It's largely a sequential call of the various routines on each crystal
%Also, it has the job of loading the calibration data (both in
%pre loaded and in raw form --preloaded needed for Mac).
%
% cryscalib
% cryscalib( run_file_list, source_profile_list )

function cryscalib( varargin )
	%plan:
	%0) parse files/input
	%   --determine how many calibration runs,
	%     how many sources (and where they are)
	%1) Do the calibration for every (half of the CB)
	%   run provided and for every crystal:
	%   --cc_do_spectrum
	%   --cc_do_cutoff
	%   --cc_do_fitting
	%   --cc_do_calib
	%2) Combine the various calibration information of the
	%   various runs (thus reducing the relative error?)
	%3) --cc_print
	
	%make the settings for the program
	%NOTE: probably, more stuff is gonna be into it
	cc_settings = struct( 'rf_list', {}, 'sp_list', {} );
	
	if ~numel( varargin )
		%use a prompt
	elseif numel( varargin ) == 2
		rf_list = varargin{1};
		sp_list = varargin{2};
		
		%check in bulk
		ck_str = @( p ) if ~ischar( p ) error( 'Not a string' ); end;
		cellfun( ck_str, rf_list );
		cellfun( ck_str, sp_list );
		
		%save it into cc_settings.
		cc_settings.rf_list = rf_list;
		cc_settings.sp_list = sp_list;
	else
		error( 'Inconsistent number of argument. Require two cells of strings.' );
	end
	
	%open the files and load the data (s)
	data = {};
	source_profiles = {};
	for ii=1:numel( cc_settings.rf_list )
		data(ii) = xb_load_data( cc_settings.rf_list{ii} );
		source_profiles(ii) = cctop_parse_sp( cc_settings.sp_list{ii} );
	end
	data = cctop_split_leftright( data ); %TODO
	
	%organize runs by source profile
	%so that runs with same source profile (and orientation)
	%contribute to the same statistics pool
	%NOTE: this means that you should feed this script
	%      things that can be combined together, so you
	%      should be sure that there's no drift.
	[data, source_profiles, nb_runZ] = cctop_combine_data( data, source_profile );
	
	%BIG loop on the crystals
	%collect the results
	lore = struct( 'hst', cell( nb_runZ, 162 ), ...
	               'binZ', cell( nb_runZ, 162 ), ...
	               'gfit_p', cell( nb_runZ, 162 ), ...
	               'gfit_e', cell( nb_runZ, 162 ), ...
	               'cutoff', cell( nb_runZ, 162 ) );
	global settings; %settings for the various functions, it's globbal.
	settings.ax_lb = 0;
	settings.ax_ub = 3e3;
	settings.bin = 10;
	%left processing
	for rr=1:2:nb_runZ
		for cc=1:82
			%do the energy spectrum from the data.
			settings.crys_nb = cc;
			[lore.hst(rr,cc), lore.binZ(rr,cc)] = cc_do_spectrum( data{rr} );
			
			%do the cutoff (no settings update necessary)
			lore.cutoff(rr,cc) = cc_do_cutoff( [lore.hst{rr,cc}; lore.binZ{rr,cc}] );
			
			%do the fitting (big thing!)
			[lore.gfit_p(rr,cc), lore.gfit_e(rr,cc)] = ...
				cc_do_fitting( [lore.hst{rr,cc}; lore.binZ{rr,cc}] );
		end
	end
	%right processing
	for rr=2:2:nb_runZ
		for cc=83:162
			%do the energy spectrum from the data.
			settings.crys_nb = cc;
			[lore.hst(rr,cc), lore.binZ(rr,cc)] = cc_do_spectrum( data{rr} );
			
			%do the cutoff (no settings update necessary)
			lore.cutoff(rr,cc) = cc_do_cutoff( [lore.hst{rr,cc}; lore.binZ{rr,cc}] );
			
			%do the fitting (big thing!)
			[lore.gfit_p(rr,cc), lore.gfit_e(rr,cc)] = ...
				cc_do_fitting( [lore.hst{rr,cc}; lore.binZ{rr,cc}] );
		end
	end
	
	%now that we did the accounts for every run and every crystal
	%we should use the whole data for every single crystal calibration.
	calinf = cctop_merge_lores( lore, source_profiles ); %TODO
	calfile = fopen( 'calinf.dat', 'a' );
	for cc=1:162
		%do the calibration
		[calinf.cal_p{cc}, calinf.cal_e{cc}, calinf.cal{cc}] = ...
			cc_do_calib( calinf.gfit_p{cc}, calinf.gfit_e{cc}, ...
			             calinf.sp );
		
		%do the energy resolution
		calinf.dE_E(cc) = ...
			cc_do_eres( calinf.gfit_p{cc}, calinf.gfit_e{cc}, calinf.cal{cc} );
		
		%and print
		cc_print( calfile, 'a', cc, calinf.cutoff(cc), calinf.cal_p(cc), ...
		          calinf.cal_e{cc}, calinf.dE_E{cc} );
	end
	
	%save the data from this execution.
	save( '-float-binary', 'datalore', 'lore', 'calinf' );
	
	%and that was it.
end
