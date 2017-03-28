%this function fit a dataset with some gaussians, specified by passing their
%initial guess values.
%
% [[A, x_0, sigma, ... ], J_val] = xb_multigaus_fit( data_set, [A, x_0, sigma, ...]
% data_set -- is supposed to be a two row matrix, the first row representing
%             the abscissa, the second the ordinate

function [parameters, J_val] = xb_multigaus_fit( data_set, parameters, varargin )
	%check that the number of parametes makes sense
	if numel( parameters ) == 0 || mod( numel( parameters ), 3 ) != 0
		error( 'Number of parameters is not consistent.' );
	end
	
	%if we've come here, we're safe
	nb_gaus = (numel( parameters ))/3;
	
	%allocate the gaus stack
	gaus_stack = xb_multigaus_stack_alloc( data_set(1,:), nb_gaus );

	%if we have optional aruments, interpret it
	%as a request for a 2-parameters exponential background
	if ~isempty( varargin )
		exp_bkg_p = varargin{1};
		parameters = [parameters(:); exp_bkg_p(:)];
		exp_bkg = @(p) p(end-1)*exp(data_set(1,:).*p(end));
	else
		exp_bkg = @(p) 0;
	end
	
	%make the cost function
	J_fun = @( p ) sum( ( xb_multigaus_stack_exec( p, gaus_stack ) ...
	                      + exp_bkg( p ) - data_set(2,:) ).^2 )/ ...
	                      (length( data_set ) - length( parameters ))^2;
	
	%do the minimization
	for ii=1:30
		[parameters, J_val] = fminsearch( J_fun, parameters );
	end
end
