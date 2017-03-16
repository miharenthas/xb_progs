%this function looks for candidate maxima (for a gaussian fit, but
%its application isn't restricted to that.
%
%maxima = xb_multigaus_find( data_series )
%    maxima -- an array of values, the candidate maxima
%    m_idx -- indexes thereof
%    data_series -- the data to be searched
%TODO: proper documentation

function [maxima, m_idx] = xb_multigaus_find( data_series, varargin )
	
	%begin with a check on the packages
	try
		pkg load signal; %for savitzky-golay algorithm
	catch
		error( "Package 'signal' and its dependencies are required." );
		return;
	end
	
	%input parsing
	trg = min( data_series ); %trigger level
	sg_len = min( max( floor( length( data_series )/100 ), 5 ), 100 ); %between 5 and 100
	if nargin >= 2 %then we have also a trigger level specified
		trg = varargin(1);
	end
	if nargin >= 3 %then we also have a length for the sgolay smoother
		sg_len = varargin(2);
	end
	%the procedure at this point is:
	% 1 -- calculate first and second derivatives
	% 2 -- flag as maxima those that have first derivative 0 and second positive
	
	if mod( sg_len, 2 ) == 0
		sg_len += 1;
	end
	first_d = sgolay( 3, sg_len, 1 ); %first derivative
	second_d = sgolay( 3, sg_len, 2 ); %second derivative

	%apply the derivatives
	d_data = sgolayfilt( data_series, first_d );
	dd_data = sgolayfilt( data_series, second_d );
	
	%find the zeroes -- which means find when the sign changes
	sgn_changes = find( sign( d_data(2:end) ) ~= sign( d_data(1:end-1) ) );
	sgn_changes += 1; %seems to be needed.
	%check if i'm seeing double
	adj = find( sgn_changes(2:end) - sgn_changes(1:end-1) == 1 );
	if numel( adj ) >= 1 %then we have adjacent hits
		sgn_changes(adj) = sgn_changes(adj.+1);
		sgn_changes = unique( sgn_changes );
	end

	%now look for the sign changes that also have a positive second
	%derivative
	m_idx = find( dd_data( sgn_changes ) <= 0 );
	m_idx = sgn_changes( m_idx );
	if nargout == 2
		maxima = data_series( m_idx );
	end
	%and that should be it.
end
	
