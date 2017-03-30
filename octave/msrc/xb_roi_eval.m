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
	
	%look for the centroid in the data series
	c_idx = find( data_series(2,:) == centroid );
	if isempty( c_idx ) %we didn't find the centroid
		error( 'Could not find centroid in data_series' );
	end
	
	%-----------------------------------------------------------------------------
	%BROKEN: this works with indexes, not with energies.
	%        that is a BAD idea. FIX IT.
	%retrieve the extremes
	c_begin = [];
	c_end = [];
	if isscalar( span )
		c_begin = max( 1, floor( c_idx-span/2 ) );
		c_end = min( length( data_series ), ceil( c_idx+span/2 ) );
	elseif isvector( span )
		c_begin = span(1);
		c_end = span(2);
	end
	if isempty( c_begin ) || isempty( c_end )
		error( 'Could not make sense of span' );
	end
	
	%do the cuttage
	data_series = data_series(:,[c_begin:c_end] );
	%-----------------------------------------------------------------------------
	
	%make the cost function
	gs = xb_multigaus_stack_alloc( data_series(1,:), 1 );
	if ~isempty( exp_bkg_guesses )
		bkg = @( p ) p(1)*exp( data_series(1,:)*p(2) );
		J_fun = @( p ) sum( ( xb_multigaus_stack_exec( p(1:3), gs ) + ...
		              bkg( p([4 5]) ) - data_series(2,:) ).^2 )/ ...
		              ( length( data_series ) - 5 );
		guesses = [[1, centroid, 1], exp_bkg_guesses]; %this is also BROKEN (idx vs nrg)
	else
		J_fun = @( p ) sum( ( xb_multigaus_stack_exec( p(1:3), gs ) - ...
		              data_series(2,:) ).^2 )/ ...
		              ( length( data_series ) - 5 );
		guesses = [1, centroid, 1]; %this is also BROKEN (idx vs nrg)
	end
	
	
	%do the fitting
	
	for ii=1:10
		guesses = fminsearch( J_fun, guesses );
	end
end
