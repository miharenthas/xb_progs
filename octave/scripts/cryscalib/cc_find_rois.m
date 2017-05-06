%This function finds the cutoff of the crystal that is being examined
%
% [roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc, p_info )
% [roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc, p_info, settings )
%
% -- energy_spc: contains the energy spectrum of the crystal
%                energy_spc = [binZ;hst];
% -- p_info: the ouptu of cc_find_peaks, stacked
%            p_info = [p_val;p_idx]
% -- settings: a struct with at least the fields:
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number

function [roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc, p_info, varargin )
	%parse the evtl. options
	if isempty( varargin )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = 0;
	elseif isstruct( varargin{1} )
		settings.ax_lb = varargin{1}.ax_lb;
		settings.ax_ub = varargin{1}.ax_ub;
		settings.crys_nb = varargin{1}.crys_nb;
	elseif isscalar( varargin{1} )
		settings.ax_lb = 0;
		settings.ax_ub = 3e3;
		settings.crys_nb = varargin{1};
	end

	go_on = true;
	fig = figure( 'position', [100, 100, 1600, 1200] );
		
	%run the roi partitioner
	[roi_spans, roi_centroids] = xb_roi_partition( energy_spc, p_info(2,:) ); 
	
	%fit the rois
	guesses = [];
	for ii=1:length( roi_centroids )
		guesses = [guesses; xb_roi_eval( energy_spc, ...
		                                 roi_spans(ii:ii+1), ...
		                                 roi_centroids(ii) ) ];
	end
	
	%a handy quantity
	lin_hgt = max( energy_spc(2,:) );

	while go_on
		%display
		figure( fig );
		stairs( energy_spc(1,:), energy_spc(2,:), 'linewidth', 2 );
		hold on;
		%plot the gaussians
		for ii=1:length( p_info(2,:) )
			gs = xb_multigaus_stack_alloc( energy_spc(1,:), 1 );
			gaus = xb_multigaus_stack_exec( guesses(ii,:), gs );
			plot( energy_spc(1,:), gaus, 'linewidth', 2 );
		end
		%and the ROIs separators
		for ii=1:length( roi_spans )
			plot( roi_spans(ii)*ones( 1, 2 ), [0, lin_hgt], 'r--', 'linewidth', 3 );
		end
		hold off;
		set( gca, 'linewidth', 2, 'fontsize', 24 );
		axis( [settings.ax_lb, settings.ax_ub] );
		ylabel( 'counts/KeV-ish' );
		xlabel( 'KeV-ish' );
		title( ['Crystal #', num2str( settings.crys_nb ), ' energy spectrum with ROIs'] );
		grid on;
		
		leg = { ['Crystal #', num2str( settings.crys_nb )] };
		for ii=1:length( roi_centroids )
			leg = [leg; ['Roi from ', num2str( roi_spans(ii) ), ...
			             ' to ', num2str( roi_spans(ii+1) ) ] ];
		end
		lg = legend( leg );
		set( lg, 'fontsize', 24 );
		
		%ask for user's opinion
		disp( "cc_find_rois: is this OK?" );
		[go_on, settings, guesses] = cc_find_rois_prompt( fig, settings, guesses );
	end
	close( fig );
end

%this function's command line
%TODO: some editing options for the ROI parameters might be useful here.
function [go_on, settings, guesses] = cc_find_rois_prompt( fig, old_settings, guesses )
	go_on = true;
	gogo_on = true;
	settings = old_settings;
	
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
			case 'guess?'
				disp( guesses );
			%change manually the parameters
			%these commands take two options
			%first is gaussian numner
			%second is value
			case 'A'
				if numel( opts ) == 2
					try
						guesses(str2num( opts{1} ),1) = str2num( opts{2} );
					catch
						disp( 'A out of range.' );
					end
				else
					disp( 'command "A" requires 2 arguments.' );
				end
			case 'x0'
				if numel( opts ) == 2
					try
						guesses(str2num( opts{1} ),2) = str2num( opts{2} );
					catch
						disp( 'x0 out of range.' );
					end
				else
					disp( 'command "x0" requires 2 arguments.' );
				end
			case 'sigma'
				if numel( opts ) == 2
					try
						guesses(str2num( opts{1} ),3) = str2num( opts{2} );
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
			otherwise
				disp( ['"', cmd, '" is not a valid command.'] );
		end
	end
end
