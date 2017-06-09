%This function performs a calibration of the crystal, given a source profile.
%
% [c_pees, cp_err] = cc_do_calib( g_fit_pees, g_fit_err, source_profile )
%
% -- g_fit_pees: the parameters of the global fit (all the peaks, their sigmas and amps)
%                as they come out of cc_do_fitting.
% -- g_fit_err: the uncertainties on those parameters.
%returns:
% --c_pees: the calibration parameters
% --cp_err: the uncertainties on them

function [c_pees, cp_err] = cc_do_calib( g_fit_pees, g_fit_err, source_profile )
	%...
end
