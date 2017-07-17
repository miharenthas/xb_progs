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
		source_profiles(ii) = cctop_parse_sp( cc_settings.sp_list{ii} ); %TODO
	end
	
	%organize runs by source profile
	%so that runs with same source profile (and orientation)
	%contribute to the same statistics pool
	%NOTE: this means that you should feed this script
	%      things that can be combined together, so you
	%      should be sure that there's no drift.
	[data, source_profiles] = cctop_combine_data( data, source_profile ); %TODO
	
	%BIG loop on the crystals
	for rr=1:numel( data )
		
	
	
end
