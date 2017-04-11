%This function makes a spectrum and returns the histogram output to the main program
%At every attempt, the function will ask the user's opinion, unless otherwise
%specified (TODO).
%
% [hst, binZ] = cc_do_spectrum( energy_data )
% [hst, binZ] = cc_do_spectrum( energy_data, settings )
%
% -- energy_data: contains the energy deposits from a crystal
%                 0-pruning will be repeated
% -- settings: a struct with at least the fields:
%              -- bin: the width of the bin
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number

function [hst, binZ] = cc_do_spectrum( energy_data, varargin )
	%parse the evtl. options
	if isempty( varargin )
		settings.bin = 10;
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = 0;
	elseif isstruct( varargin{1} )
		settings.bin = varargin{1}.bin;
		settings.ax_lb = varargin{1}.ax_lb;
		settings.ax_ub = varargin{1}.ax_ub;
		settings.crys_nb = varargin{1}.crys_nb;
	elseif isscalar( varargin{1} )
		settings.bin = 10;
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = varargin{1};
	end

	%do a spot of 0-pruning
	energy_data = energy_data( find( energy_data ) );

	go_on = true;
	fig = figure( 'position', [100, 100, 1600, 1200] );
	while go_on
		%do the histogram
		[hst, binZ] = hist( energy_data, max( energy_data )/settings.bin );
		
		%display
		figure( fig );
		stairs( binZ, hst, 'linewidth', 2 );
		set( gca, 'linewidth', 2, 'fontsize', 24 );
		axis( [settings.ax_lb, settings.ax_ub] );
		ylabel( 'counts/KeV-ish' );
		xlabel( 'KeV-ish' );
		title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum'] );
		grid on;
		
		leg = { ['Crystal #', num2str( settings.crys_nb )] };
		lg = legend( leg );
		set( lg, 'fontsize', 24 );

		%ask for user's opinion
		disp( "cc_do_spectrum: is this OK?" );
		[go_on, settings] = cc_do_spectrum_prompt( fig, settings );
	end
	close( fig );
end

%this function's command line
function [go_on, settings] = cc_do_spectrum_prompt( fig, old_settings )
	go_on = true;
	settings = old_settings;
	
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
		case 'bin'
			if numel( opts ) == 1
				settings.bin = str2num( opts{1} );
			else
				disp( 'command "bin" requires 1 argument.' );
			end
		case 'crys'
			if numel( opts ) == 1
				settings.crys_nb = str2num( opts{1} );
			else
				disp( 'command "crys" requires 1 argument.' );
			end
		case 'save'
			if ~isempty( opts )
				name = opts{1};
			else
				name = [ num2str( settings.crys_nb ), '_spc_', ...
				         num2str( settings.bin ), '_from_', ...
				         num2str( settings.ax_lb ), '_to_', ...
				         num2str( settings.ax_ub ) ];
				hgsave( fig, name );
			end
		otherwise
			disp( ['"', cmd, '" is not a valid command.'] );
	end
end
