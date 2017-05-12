%this function drives the fitting of one crystal's energy spectrum
%The order of operations is:
% 1 -- cc_find_peaks
% 2 -- cc_find_rois
% 3 -- cc_find_gobal_fit
%and then returns the global fit parameters.
%
% [g_fit_pees, g_fit_err] = cc_do_fitting( energy_spc )
% [g_fit_pees, g_fit_err] = cc_do_fitting( energy_spc, settings )
%
% -- energy_spc: the energy spectrum
% -- settings: a stricture containing at least
%              -- ax_lb: the x axis lower bound
%              -- ax_ub: the x axis upper bound
%              -- crys_nb: the crystal number
%              optionally, it can also contain:
%              -- trg: the minimum aplitude of a peak
%              -- sg_len: the length of the Savitzky-Golay (sg) filter
%              -- sg_smt: number of passes of the smoother
%              -- sg_ord: order of the sg filter
%returns:
% -- g_fit_pees: the parameters of the miltigaussian fit
%                just completed.

function [g_fit_pees, g_fit_err] = cc_do_fitting( energy_spc, settings )
	if nargin == 1
		%don't pass any settings.
		nosettings_flag = 1;
	elseif nargin == 2
		nosettings_flag = 0;
	end
	
	%just call in sequence here...
	if nosettings_flag
		[p_val, p_idx] = cc_find_peaks( energy_spc );
		if isempty( p_idx ) || isempty( p_val )
			%TODO: better handling here. Possibly.
			error( 'cc_do_fitting: no peaks found. Abort.' );
		end
		
		[roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc, [p_val;p_idx] );
		if isempty( roi_centroids )
			error( 'cc_do_fitting: no ROI centroids --> no ROIs. Abort.' );
		end
		
		[gfit_params, p_err] = cc_find_global_fit( energy_spc, guesses );
		if isempty( gfit_params )
			error( 'c_do_fitting: no fit results. Abort.' );
		end
		%stack the parameters
		g_fit_pees = [];
		g_fit_err = [];
		for ii=1:3:numel( gfit_params ) %and this MUST be an integer
			g_fit_pees = [g_fit_pees; gfit_params(ii:ii+2)];
			g_fit_err = [g_fit_err; p_err(ii:ii+2)];
		end
	else
		[p_val, p_idx] = cc_find_peaks( energy_spc, settings );
		if isempty( p_idx ) || isempty( p_val )
			%TODO: better handling here. Possibly.
			error( 'cc_do_fitting: no peaks found. Abort.' );
		end
		
		[roi_spans, roi_centroids, guesses] = cc_find_rois( energy_spc,
		                                                    [p_val;p_idx],
		                                                    settings );
		if isempty( roi_centroids )
			error( 'cc_do_fitting: no ROI centroids --> no ROIs. Abort.' );
		end
		
		[gfit_params, p_err] = cc_find_global_fit( energy_spc, guesses, settings );
		if isempty( gfit_params )
			error( 'c_do_fitting: no fit results. Abort.' );
		end
		%stack the parameters
		g_fit_pees = [];
		g_fit_err = [];
		for ii=1:3:numel( gfit_params ) %and this MUST be an integer
			g_fit_pees = [g_fit_pees; gfit_params(ii:ii+2)];
			g_fit_err = [g_fit_err; p_err(ii:ii+2)];
		end
	end
end
		

	
