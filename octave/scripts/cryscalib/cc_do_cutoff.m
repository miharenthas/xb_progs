%This function finds the cutoff of the crystal that is being examined
%
% cutoff = cc_do_cutoff( energy_spc )
%
%NOTE: this function works on the spectrum, not on the raw energy data
%      this means that the returned value is somewhat dependent from the
%      binning. This anyway still makes more sense than just finding the
%      minimum energy deposit (which is anyway affected by the resolution).
%
% -- energy_spc: contains the energy spectrum of the crystal
%                energy_spc = [binZ;hst];
%
%GLOBAL VARIABLES
% -- settings: a struct with at least the fields:
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number
%              -- c_trg: the trigger level

function cutoff = cc_do_cutoff( energy_spc )
	global settings;

	%parse the evtl. options
	if isempty( settings )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = 0;
		settings.c_trg = 10;
	elseif isstruct( settings )
		if isfield( settings, 'ax_lb' ) settings.ax_lb = settings.ax_lb;
		else settings.ax_lb = 0; end
		if isfield( settings, 'ax_ub' ) settings.ax_ub = settings.ax_ub;
		else settings.ax_ub = 3e3; end
		if isfield( settings, 'crys_nb' ) settings.crys_nb = settings.crys_nb;
		else settings.crys_nb = 0; end
		if isfield( settings, 'c_trg' ) settings.c_trg = settings.c_trg;
		else settings.c_trg = 10; end
	elseif isscalar( settings )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = settings;
		settings.c_trg = 10;
	end

	go_on = true;
	fig = figure( 'position', [100, 100, 1600, 1200] );

	%find the minimm value
	cutoff = min( energy_spc(1, find( energy_spc(2,:) > settings.c_trg ) ) );
	lin_hgt = max( energy_spc(2,:) );
	while go_on
		%display
		figure( fig );
		stairs( energy_spc(1,:), energy_spc(2,:), 'linewidth', 2 );
		hold on;
		plot( cutoff*ones( 1, 2 ), [0, lin_hgt], 'r--', 'linewidth', 3 );
		hold off;
		set( gca, 'linewidth', 2, 'fontsize', 24 );
		axis( [settings.ax_lb, settings.ax_ub] );
		ylabel( 'counts/KeV-ish' );
		xlabel( 'KeV-ish' );
		title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum with cutoff'] );
		grid on;
		
		leg = { ['Crystal #', num2str( settings.crys_nb )]; ['Cutoff ',num2str( cutoff )]  };
		lg = legend( leg );
		set( lg, 'fontsize', 24 );
		
		%ask for user's opinion
		disp( "cc_do_cutoff: is this OK?" );
		[go_on, settings, cutoff] = cc_do_cutoff_prompt( fig, settings, cutoff );
	end
	close( fig );
end

%this function's command line
function [go_on, settings, cutoff] = cc_do_cutoff_prompt( fig, old_settings, cutoff )
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
				name = [ num2str( settings.crys_nb ), '_cutoff_', ...
				         num2str( settings.bin ), '_from_', ...
				         num2str( settings.ax_lb ), '_to_', ...
				         num2str( settings.ax_ub ) ];
				hgsave( fig, name );
			end
		case 'mv'
			if numel( opts ) == 2
				if strcmp( opts{1}, '+' ) cutoff += str2num( opts{2} );
				elseif strcmp( opts{1}, '-' ) cutoff -= str2num( opts{2} );
				else disp( 'Operator not supported.' ); end
			elseif numel( opts ) == 1
				cutoff = str2num( opts{1} );
			else
				disp( 'command "mv" requires 1 or 2 arguments.' );
			end
		otherwise
			disp( ['"', cmd, '" is not a valid command.'] );
	end
end
