%This function returns the energy resolution of a crystal
%
% dE_E = cc_do_eres( g_fit_pees, g_fit_err, calib )
%
% -- g_fit_pees: global fit parameters
% -- g_fit_err: errors thereupon (not used yet!)
% -- calib: the calibration function (comes out of cc_do_calib)
%returns:
% -- dE_E: an ARRAY with the dE/E of each peak.


function dE_E = cc_do_eres( g_fit_pees, g_fit_err, calib )
	%calibrate
	pks = calib( g_fit_pees(:,2) );
	
	%do the fwhm
	fwhm = 2*sqrt(2*log(2))*g_fit_pees(:,3);
	
	%calc dE/E
	dE_E = [fwhm./pks; pks(:)];
end
