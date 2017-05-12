%This function finds the peaks in a spectrum, based on a Savitzky-Golay filter
%See the documentation of xb_multigaus_find for more details.
%
% [p_val, p_idx] = cc_find_peaks( energy_spc )
% [p_val, p_idx] = cc_find_peaks( energy_spc, settings )
%
%
% -- energy_spc: contains the energy spectrum of the crystal
%                energy_spc = [binZ;hst];
% -- settings: a struct with at least the fields:
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number
%              optionally, it can also contain:
%              -- trg: the minimum aplitude of a peak
%              -- sg_len: the length of the Savitzky-Golay (sg) filter
%              -- sg_smt: number of passes of the smoother
%              -- sg_ord: order of the sg filter

function [p_val, p_idx] = cc_find_peaks( energy_spc, varargin )
	%parse the evtl. options
	if isempty( varargin )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = 0;
		%these are the defaults fot xb_multigaus_find
		%look up the lines 21 through 24.
		settings.trg = 10; %except this
		settings.sg_len = 3;
		settings.sg_smt = 5;
		settings.sg_ord = 2;
	elseif isstruct( varargin{1} )
		settings.ax_lb = varargin{1}.ax_lb;
		settings.ax_ub = varargin{1}.ax_ub;
		settings.crys_nb = varargin{1}.crys_nb;
		if isfield( varargin{1}, 'trg' ) settings.trg = varargin{1}.trg;
		else settings.trg = 10; end
		if isfield( varargin{1}, 'sg_len' ) settings.sg_len = varargin{1}.sg_len;
		else settings.sg_len = 3; end
		if isfield( varargin{1}, 'sg_smt' ) settings.sg_smt = varargin{1}.sg_smt;
		else settings.sg_smt = 5; end
		if isfield( varargin{1}, 'sg_ord' ) settings.sg_ord = varargin{1}.sg_ord;
		else settings.sg_ord = 2; end
	elseif isscalar( varargin{1} )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		%these are the defaults fot xb_multigaus_find
		%look up the lines 21 through 24.
		settings.trg = 10;
		settings.sg_len = 3;
		settings.sg_smt = 5;
		settings.sg_ord = 2;
		settings.crys_nb = varargin{1};
	end

	go_on = true; rcl = true;
	fig = figure( 'position', [100, 100, 1600, 1200] );
	while go_on
		%find the peaks
		if rcl
			[p_val, p_idx] = xb_multigaus_find( energy_spc(2,:), ...
			                                    'triglevel', settings.trg, ...
			                                    'sgolaylength', settings.sg_len, ...
			                                    'sgolayorder', settings.sg_ord, ...
			                                    'smoothpasses', settings.sg_smt );
		end
		%display
		figure( fig );
		stairs( energy_spc(1,:), energy_spc(2,:), 'linewidth', 2 );

		%add the peak markers
		hold on;
		if ~isempty( p_idx ) %if there are some
			for ii=1:length( p_idx )
				plot( energy_spc(1,p_idx(ii)), p_val(ii), 'x', 'linewidth', 5 );
			end
		end
		hold off;
		set( gca, 'linewidth', 2, 'fontsize', 24 );
		axis( [settings.ax_lb, settings.ax_ub] );
		ylabel( 'counts/KeV-ish' );
		xlabel( 'KeV-ish' );
		title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum, peaks'] );
		grid on;
		
		%build the legend
		leg = { ['Crystal #', num2str( settings.crys_nb )] };
		for ii=1:length( p_idx )
			leg = [leg; ['Peak at ', num2str( energy_spc(1,p_idx(ii)) ), ...
			             ' amp: ', num2str( p_val(ii) ) ] ];
		end
		lg = legend( leg );
		set( lg, 'fontsize', 24 );
		
		%ask for user's opinion
		disp( "cc_find_peaks: is this OK?" );
		payload.p_val = p_val;
		payload.p_idx = p_idx;
		[go_on, rcl, settings, p_val, p_idx] = cc_find_peaks_prompt( fig, settings, payload );
	end
	close( fig );
end

%this function's command line
%TODO: some editing of the peak maxima might be useful here...
function [go_on, rcl, settings, p_val, p_idx] = cc_find_peaks_prompt( fig, old_settings, payload )
	go_on = true; rcl = true;
	settings = old_settings;
	p_val = payload.p_val;
	p_idx = payload.p_idx;

	user_says = input( 'cc> ', 'S' );
	if ~user_says
		return;
	end
	
	[cmd, opts] = cc_parse_cmd( user_says );
	
	switch( cmd )
		case 'ok'
			go_on = false;
		case 'axis'
			if numel( opts ) == 2
				settings.ax_lb = str2num( opts{1} );
				settings.ax_ub = str2num( opts{2} );
			else
				disp( 'command "axis" requires 2 arguments.' );
				continue;
			end
		case 'crys'
			if numel( opts ) == 1
				settings.crys_nb = str2num( opts{1} );
			else
				disp( 'command "crys" requires 1 argument.' );
			end
		case 'trg'
			if numel( opts ) == 1
				settings.trg = str2num( opts{1} );
			else
				disp( 'command "trg" requires 1 argument.' );
			end
		case 'sglen'
			if numel( opts ) == 1
				settings.sg_len = str2num( opts{1} );
			else
				disp( 'command "sglen" requires 1 argument.' );
			end
		case 'sgord'
			if numel( opts ) == 1
				settings.sg_ord = str2num( opts{1} );
			else
				disp( 'command "sgord" requires 1 argument.' );
			end
		case 'smoothps'
			if numel( opts ) == 1
				settings.sg_smt = str2num( opts{1} ); 
			else
				disp( 'command "smoothps" requires 1 argument.' );
			end
		case 'rm'
			if numel( opts ) == 1
				idx = 1:length( payload.p_idx );
				idx = find( idx ~= str2num( opts{1} ) );
				p_idx = payload.p_idx(idx);
				p_val = payload.p_val(idx);
				rcl = false;
			else
				disp( 'command "rm" supports one only argument.' );
			end
		case 'reset'
			settings = old_settings;
		case 'rcl'
			rcl = true;
		case 'save'
			if ~isempty( opts )
				name = opts{1};
			else
				name = [ num2str( settings.crys_nb ), '_peaks_', ...
				         num2str( settings.bin ), '_from_', ...
				         num2str( settings.ax_lb ), '_to_', ...
				         num2str( settings.ax_ub ) ];
				hgsave( fig, name );
			end
		otherwise
			disp( ['"', cmd, '" is not a valid command.'] );
	end
end
