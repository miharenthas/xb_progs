%This function performs a calibration of the crystal, given a source profile.
%
% [c_pees, cp_err, calib, source_profile] = cc_do_calib( g_fit_pees, g_fit_err, source_profile )
%
% -- g_fit_pees: the parameters of the global fit (all the peaks, their sigmas and amps)
%                as they come out of cc_do_fitting.
% -- g_fit_err: the uncertainties on those parameters.
% -- source_profile: an array of energies at which there's a peak (no intensities)
%returns:
% --c_pees: the calibration parameters
% --cp_err: the uncertainties on them
% --calib: a calibration function (just comfort).

function [c_pees, cp_err, calib, source_profile] = ...
		cc_do_calib( g_fit_pees, g_fit_err, source_profile )
	%first, do a linear regression
	
	if size( g_fit_pees, 1 ) <= length( source_profile(1,:) ) %crop the source profile
		                                                  %by intensity
		[~, ii] = sort( source_profile(2,:), 'descend' );
		source_profile = source_profile(:,ii(1:size( g_fit_pees, 1 )));
		[~, ii] = sort( source_profile(1,:) );
		source_profile = source_profile(:,ii);
	elseif size( g_fit_pees, 1 ) >= length( source_profile(1,:) ) %freak out
		error( 'More peaks than source lines.' );
		%NOTE: this should be done manually with a command line
		%TODO: command line.
	end
	
	pks = g_fit_pees(:,2); %extract the peak info
	calib = @( p ) sum( (p(1)*pks + p(2) - source_profile(1,:)(:) ).^2 );

	%do the minimisation
	opt = optimset( 'MaxIter', 1e6, 'TolX', 1e-5 );
	[c_pees, ~, ~, ~, ~, hessian] = fminunc( calib, [1, 0], opt ); 
	
	%calculate the errors.
	%first, the ones coming out of the fit itself.
	cp_err = sqrt( diag( inv( hessian ) ) );
	
	%make the calibration function
	calib = @(x) x*c_pees(1) + c_pees(2);
end
