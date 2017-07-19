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

	%organize runs by source profile
	%so that runs with same source profile (and orientation)
	%contribute to the same statistics pool
	%NOTE: this means that you should feed this script
	%      things that can be combined together, so you
	%      should be sure that there's no drift.
	[data, source_profiles] = cctop_combine_data( data, source_profile );
	
	%BIG loop on the crystals
	%collect the results
	lore = struct( 'hst', cell( 1, 162 ), ...
	               'binZ', cell( 1, 162 ), ...
	               'gfit_p', cell( 1, 162 ), ...
	               'gfit_e', cell( 1, 162 ), ...
	               'cutoff', cell( 1, 162 ), ...
	               'cal_p', cell( 1, 162 ), ...
	               'cal_e', cell( 1, 162 ), ...
	               'dE_E', cell( 1, 162 ) );
	global settings; %settings for the various functions, it's globbal.
	settings.ax_lb = 0;
	settings.ax_ub = 3e3;
	settings.bin = 10;
	%left processing
	for cc=1:162
		%do the energy spectrum from the data.
		settings.crys_nb = cc;
		[lore.hst(cc), lore.binZ(cc)] = cc_do_spectrum( data );
	
		%do the cutoff (no settings update necessary)
		lore.cutoff(cc) = cc_do_cutoff( [lore.hst{cc}; lore.binZ{cc}] );
	
		%do the fitting (big thing!)
		[lore.gfit_p(cc), lore.gfit_e(cc)] =
			cc_do_fitting( [lore.hst{cc}; lore.binZ{cc}] );
	end
	
	%now that we did the accounts for every run and every crystal
	%we should use the whole data for every single crystal calibration.
	calfile = fopen( 'lore.dat', 'a' );
	for cc=1:162
		%do the calibration
		[lore.cal_p{cc}, lore.cal_e{cc}, lore.cal{cc}] = ...
			cc_do_calib( lore.gfit_p{cc}, lore.gfit_e{cc}, ...
			             lore.sp );
		
		%do the energy resolution
		lore.dE_E(cc) = ...
			cc_do_eres( lore.gfit_p{cc}, lore.gfit_e{cc}, lore.cal{cc} );
		
		%and print
		cc_print( calfile, 'a', cc, lore.cutoff(cc), lore.cal_p(cc), ...
		          lore.cal_e{cc}, lore.dE_E{cc} );
	end
	
	%save the data from this execution.
	save( '-float-binary', 'lore.oct', 'lore' );
	
	%and that was it.
end
