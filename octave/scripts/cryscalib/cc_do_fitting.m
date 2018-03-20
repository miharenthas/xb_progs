%this function drives the fitting of one crystal's energy spectrum
%The order of operations is:
% 1 -- cc_find_peaks
% 2 -- cc_find_rois
% 3 -- cc_find_gobal_fit
%and then returns the global fit parameters.
%
% [g_fit_pees, g_fit_err] = cc_do_fitting( energy_spc )
%
% -- energy_spc: the energy spectrum
%returns:
% -- g_fit_pees: the parameters of the miltigaussian fit
%                just completed.
%
%GLOBAL VARIABLES:
% -- ccg_repeat: it's a flag, set by the various (prompt) functions
%                that causes the loop to repeat (and allows to scrap
%                and redo the fitting procedure).

function [g_fit_pees, g_fit_err] = cc_do_fitting( energy_spc )
	global ccg_repeat;	
	do
		ccg_repeat = false; %reset the repat flag
		
		%just call in sequence here...
		[p_val, p_idx] = cc_find_peaks( energy_spc );
		if isempty( p_idx ) || isempty( p_val )
			%TODO: better handling here. Possibly.
			error( 'cc_do_fitting: no peaks found. Abort.' );
		end
		if ccg_repeat; continue; end
	
		[roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc, [p_val;p_idx] );
		if isempty( roi_centroids )
			error( 'cc_do_fitting: no ROI centroids --> no ROIs. Abort.' );
		end
		if ccg_repeat; continue; end
		
		[gfit_params, p_err] = cc_find_global_fit( energy_spc, guesses );
		if isempty( gfit_params )
			error( 'c_do_fitting: no fit results. Abort.' );
		end
		if ccg_repeat; continue; end
		
		%stack the parameters
		g_fit_pees = [];
		g_fit_err = [];
		for ii=1:3:numel( gfit_params ) %and this MUST be an integer
			g_fit_pees = [g_fit_pees; gfit_params(ii:ii+2)];
			g_fit_err = [g_fit_err; p_err(ii:ii+2)];
		end
		if ccg_repeat; continue; end
	until ~ccg_repeat;
end
		

	
