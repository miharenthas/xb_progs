%this function processes a ROI to make a guess (or actually directly fit)
%a single gaussian with (optionally) an exponential background.
%NOTE: it is not supposed to be a precision tool, but to be used in addition
%      with xb_multigaus_find to safely operate xb_multigaus_fit
%
% [A, x_0, sigma] = xb_roi_eval( data_series, span, centroid )
% [A, x_0, sigma, A_e, decay_e] = xb_roi_eval( data_series, span, centroid, [A_e, decay_e] )
% 
%Arguments:
%    data_series : [x;y] information
%    span: a number or a couple that represent the section of the data series to be considered
%    centroid: the centroid of said section -- MUST belong to data series

function guesses = xb_roi_eval( data_series, span, centroid, varargin )
	
	%retrieve the (optional) exponential decay initial guesses
	exp_bkg_guesses = [];
	if ~isempty( varargin ) && isvector( varargin{1} )
		exp_bkg_guesses = varargin{1};
	end
	
	%-----------------------------------------------------------------------------
	%retrieve the extremes
	c_right = [];
	c_left = [];
	c_all = [];
	if isscalar( span )
		c_right = data_series(1,:) < centroid + span/2;
		c_left = data_series(1,:) > centroid - span/2;
		c_all = c_left == c_right;
	elseif isvector( span )
		c_right = data_series(1,:) < span(2);
		c_left = data_series(1,:) > span(1);
		c_all = c_left == c_right;
	end
	
	%do the cuttage
	data_series = data_series(:,c_all);

	%-----------------------------------------------------------------------------
	%make the cost function
	gs = xb_multigaus_stack_alloc( data_series(1,:), 1 );
	if ~isempty( exp_bkg_guesses )
		bkg = @( p ) p(1)*exp( data_series(1,:)*p(2) );
		J_fun = @( p ) sum( ( xb_multigaus_stack_exec( p(1:3), gs ) + ...
		              bkg( p([4 5]) ) - data_series(2,:) ).^2 )/ ...
		              ( length( data_series ) - 5 );
		guesses = [[centroid, 1, 1], exp_bkg_guesses];
	else
		J_fun = @( p ) sum( ( xb_multigaus_stack_exec( p(1:3), gs ) - ...
		              data_series(2,:) ).^2 )/ ...
		              ( max( length( data_series ) - 5, 1 ) );
		guesses = [centroid, 1, 1];
	end
	
	
	%do the fitting
	for ii=1:10
		guesses = fminsearch( J_fun, guesses );
	end
end
