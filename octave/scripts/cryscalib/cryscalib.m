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
	if ~numel( varargin )
		%use a prompt, TODO
		warning( 'No prompt available yet.' );
		return;
	elseif numel( varargin ) >= 2
		rf_list = varargin(1);
		sp_list = varargin(2);
		
		%check in bulk
		ck_str = @( p ) if ~ischar( p ) error( 'Not a string' ); end;
		cellfun( ck_str, rf_list );
		cellfun( ck_str, sp_list );
		
		%save it into cc_settings.
		cc_settings.rf_list = rf_list;
		cc_settings.sp_list = sp_list;
		if numel( varargin ) == 3
			if isvector( varargin{3} ) target_crys = varargin{3};
			else error( 'Target crystal must be an array of numbers' ); end
		else target_crys = [1:162]; end
	else
		error( 'Inconsistent number of argument.' );
	end
	
	%open the files and load the data (s)
	data = {};
	source_profiles = {};
	for ii=1:numel( cc_settings.rf_list )
		data(ii) = xb_load_data( cc_settings.rf_list{ii}, 1e6 );
		source_profiles(ii) = cctop_parse_sp( cc_settings.sp_list{ii} );
	end

	%organize runs by source profile
	%so that runs with same source profile (and orientation)
	%contribute to the same statistics pool
	%NOTE: this means that you should feed this script
	%      things that can be combined together, so you
	%      should be sure that there's no drift.
	[data, source_profiles] = cctop_combine_data( data, source_profiles );
	
	%BIG loop on the crystals
	%collect the results
	lore = struct( 'hst', cell( 1, 162 ), ...
	               'binZ', cell( 1, 162 ), ...
	               'gfit_p', cell( 1, 162 ), ...
	               'gfit_e', cell( 1, 162 ), ...
	               'cutoff', cell( 1, 162 ), ...
	               'cal_p', cell( 1, 162 ), ...
	               'cal_e', cell( 1, 162 ), ...
	               'calf', cell( 1, 162 ), ...
	               'csp', cell( 1, 162 ), ...
	               'dE_E', cell( 1, 162 ) );
	global settings; %settings for the various functions, it's globbal.
	settings.ax_lb = 0;
	settings.ax_ub = 3e3;
	settings.bin = 10;
	%left processinggit
	for cc=target_crys
		%do the energy spectrum from the data.
		settings.crys_nb = cc;
		oh = @( p ) p == cc;
		c_data = xb_data_cut_on_field( data, oh, 'i' );
		[lore(cc).hst, lore(cc).binZ] = cc_do_spectrum( [c_data.e] );
	
		%do the cutoff (no settings update necessary)
		lore(cc).cutoff = cc_do_cutoff( [lore(cc).binZ; lore(cc).hst] );
	
		%do the fitting (big thing!)
		[lore(cc).gfit_p, lore(cc).gfit_e] = ...
			cc_do_fitting( [lore(cc).binZ; lore(cc).hst] );
	end
	
	%now that we did the accounts for every run and every crystal
	%we should use the whole data for every single crystal calibration.
	calfile = fopen( 'lore.dat', 'a' );
	for cc=target_crys
		%do the calibration
		[lore(cc).cal_p, lore(cc).cal_e, lore(cc).calf, lore(cc).csp ] = ...
			cc_do_calib( lore(cc).gfit_p, lore(cc).gfit_e, ...
			             source_profiles );
		
		%do the energy resolution
		lore(cc).dE_E = ...
			cc_do_eres( lore(cc).gfit_p, lore(cc).gfit_e, lore(cc).calf );
		
		%and print
		f = cc_print( calfile, 'a', cc, lore(cc).cutoff, lore(cc).cal_p, ...
		              lore(cc).cal_e, lore(cc).dE_E );
	end
	fclose( f );
	
	%save the data from this execution.
	save( '-float-binary', 'lore.ofb', 'lore' );
	
	%and that was it.
end
