%This function finds the cutoff of the crystal that is being examined
%
% [gfit_params, p_err] = cc_find_global_fit( energy_spc, guesses )
%
%NOTE: p_err, which should contain the parameter errors, it's not deduced from the
%      fit itself yet, but very bone idly estimated from the data. Very bone idly indeed.
%
% -- energy_spc: contains the energy spectrum of the crystal
%                energy_spc = [binZ;hst];
% -- guesses: the ouptut of cc_find_rois, stacked
%
%GLOBAL VARIABLES
% -- settings: a struct with at least the fields:
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number
% -- ccg_repeat: a flag to cause the current fitting process to be discarded
%                and repeated from scratch.

function [gfit_params, p_err] = cc_find_global_fit( energy_spc, guesses )
	global settings;

	%parse the evtl. options
	if isempty( settings )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = 0;
	elseif isstruct( settings )
		settings.ax_lb = settings.ax_lb;
		settings.ax_ub = settings.ax_ub;
		settings.crys_nb = settings.crys_nb;
	elseif isscalar( settings )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = settings;
	end

	go_on = true;
	fig = figure( 'position', [100, 100, 1600, 1200] );
	
	%linearize the parameters
	%reshape just doesn't cut it here.	
	pees = [];
	new_pees = [];
	for ii=1:size( guesses, 1 ) %the number of rows in guesses is the number of gaussians
		new_pees = [new_pees, guesses(ii,:)];
	end
	
	%main loop
	g_all = xb_multigaus_stack_alloc( energy_spc(1,:), size( guesses, 1 ) );
	while go_on
		%a handy quantity
		lin_hgt = max( energy_spc(2,:) );
		rel_err_est = .67/sqrt( lin_hgt ); %as anticipated, very bone idly
		
		%run the global fit
		pees = xb_multigaus_fit( energy_spc, new_pees );
		p_err = pees.*rel_err_est; %very bone idly indeed
		
		%display
		figure( fig );
		stairs( energy_spc(1,:), energy_spc(2,:), 'linewidth', 2 );
		
		hold on;
		%plot the gaussians
		for gg=1:size( guesses, 1 )
			par_point = 3*(gg-1)+1;
			gs = xb_multigaus_stack_alloc( energy_spc(1,:), 1 );
			gaus = xb_multigaus_stack_exec( pees(par_point:par_point+2), gs );
			plot( energy_spc(1,:), gaus, 'linewidth', 2 );
		end

		%and add the global fit
		g_fit = xb_multigaus_stack_exec( pees, g_all );
		plot( energy_spc(1,:), g_fit, 'linewidth', 3, 'k--' );
		
		hold off;
		
		set( gca, 'linewidth', 2, 'fontsize', 24 );
		axis( [settings.ax_lb, settings.ax_ub] );
		ylabel( 'counts/KeV-ish' );
		xlabel( 'KeV-ish' );
		title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum, global fit'] );
		grid on;
		
		leg = { ['Crystal #', num2str( settings.crys_nb )] };
		for ii=1:size( guesses, 1 )
			leg = [leg; ['Peak #', num2str( ii )]];
		end
		leg = [leg; 'Global fit'];
		lg = legend( leg );
		set( lg, 'fontsize', 24 );
		
		%ask for user's opinion
		disp( "cc_find_global_fit: is this OK?" );
		%make a payload
		payload.gs = guesses;
		payload.spc = energy_spc; 
		[go_on, settings, new_pees] = cc_find_global_fit_prompt( fig, settings, payload );
	end
	close( fig );
	
	%remember to return
	gfit_params = pees;
end

%this function's command line
%TODO: some editing options for the ROI parameters might be useful here.
function [go_on, settings, pees] = cc_find_global_fit_prompt( fig, old_settings, payload )
	global ccg_repeat;

	go_on = true;
	gogo_on = true; %internal loop flag
	settings = old_settings;
	guess_fig = 0;
	
	while gogo_on
		user_says = input( 'cc> ', 'S' );
		if ~user_says
			return;
		end
	
		[cmd, opts] = cc_parse_cmd( user_says );
	
		switch( cmd )
			case 'ok'
				go_on = false;
				gogo_on = false;
			case 'try'
				go_on = true;
				gogo_on = false;
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
			case 'reset'
				pees = [];
				for ii=1:size( payload.gs, 1 ) 
					pees = [pees, payload.gs(ii,:)];
				end
			case 'guess?'
				disp( payload.gs );
				guess_fig = cc_find_global_fit_showguesses( settings,
				                                            payload,
				                                            guess_fig );
			%change manually the parameters
			%these commands take two options
			%first is gaussian numner
			%second is value
			case 'A'
				if numel( opts ) == 2
					try
						payload.gs(str2num( opts{1} ),1) = str2num( opts{2} );
					catch
						disp( 'A out of range.' );
					end
				else
					disp( 'command "A" requires 2 arguments.' );
				end
			case 'x0'
				if numel( opts ) == 2
					try
						payload.gs(str2num( opts{1} ),2) = str2num( opts{2} );
					catch
						disp( 'x0 out of range.' );
					end
				else
					disp( 'command "x0" requires 2 arguments.' );
				end
			case 'sigma'
				if numel( opts ) == 2
					try
						payload.gs(str2num( opts{1} ),3) = str2num( opts{2} );
					catch
						disp( 'sigma out of range.' );
					end
				else
					disp( 'command "sigma" requires 2 arguments.' );
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
			case 'scrap'
				ccg_repeat = true;
				gogo_on = false;
				go_on = false;
			otherwise
				disp( ['"', cmd, '" is not a valid command.'] );
		end
	end
	
	%if we are doing another attempt, reset the pees
	pees = [];
	if go_on
		for ii=1:size( payload.gs, 1 ) 
			pees = [pees, payload.gs(ii,:)];
		end
	end
	
	%close the guesses figure
	if guess_fig close( guess_fig ); end
end

function fig = cc_find_global_fit_showguesses( settings, payload, fig )
	if ~fig
		fig = figure( 'position', [ 300, 300, 1024, 768 ] );
	end
	hold off; %always redraw this
	
	%draw
	stairs( payload.spc(1,:), payload.spc(2,:), 'linewidth', 2 );
	hold on;
	%plot the gaussians
	for ii=1:size( payload.gs, 1 )
		gs = xb_multigaus_stack_alloc( payload.spc(1,:), 1 );
		gaus = xb_multigaus_stack_exec( payload.gs(ii,:), gs );
		plot( payload.spc(1,:), gaus, 'linewidth', 2 );
	end
	hold off;
	set( gca, 'linewidth', 2, 'fontsize', 24 );
	axis( [settings.ax_lb, settings.ax_ub] );
	ylabel( 'counts/KeV-ish' );
	xlabel( 'KeV-ish' );
	title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum with current guesses'] );
	grid on;
	
	%add a legend
	leg = {};
	for ii=1:size( payload.gs, 1 )
		leg(ii) = [ 'Guess #', num2str( ii ) ];
	end
	lg = legend( leg );
	
	set( gca, 'fontsize', 24 );
end
	
