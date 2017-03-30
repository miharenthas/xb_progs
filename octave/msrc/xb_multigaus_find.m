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
	sg_len = 3; %default sgolay length
	sg_smt = 5; %smoother settings
	sg_ord = 2; %filter order
	
	if ~isempty( varargin )
		prop_name = varargin(1:2:end);
		prop_value = varargin(2:2:end);
		
		for aa=1:length( prop_name )
			if strcmp( prop_name{aa}, 'triglevel' )
				trg = prop_value{aa};
			elseif strcmp( prop_name{aa}, 'sgolaylength' )
				sg_len = prop_value{aa};
			elseif strcmp( prop_name{aa}, 'sgolayorder' )
				sg_ord = prop_value{aa};
			elseif strcmp( prop_name{aa}, 'smoothpasses' )
				sg_smt = prop_value{aa};
			else
				error( [prop_name{aa}, ' is an uknown propery.'] );
			end
		end
	end
	
	%the procedure at this point is:
	% 1 -- calculate first and second derivatives
	% 2 -- flag as maxima those that have first derivative 0 and second positive
	
	if mod( sg_len, 2 ) == 0
		sg_len += 1;
	end
	first_d = sgolay( sg_ord, sg_len, 1 ); %first derivative
	second_d = sgolay( sg_ord, sg_len, 2 ); %second derivative
	smoother = sgolay( sg_ord, 5*sg_len ); %a smoother

	%apply the derivatives (and do smoothing)
	d_data = sgolayfilt( data_series, first_d );
	for ii=1:sg_smt d_data = sgolayfilt( d_data, smoother ); end %smoother
	dd_data = sgolayfilt( data_series, second_d );
	for ii=1:sg_smt dd_data = sgolayfilt( dd_data, smoother ); end %smoother
	
	%find the zeroes -- which means find when the sign changes
	sgn_changes = find( sign( d_data(2:end) ) ~= sign( d_data(1:end-1) ) );
	sgn_changes += 1; %seems to be needed.
	%check if i'm seeing double
	adj_stp = max( length( data_series )/1e3, 1 );
	adj = find( sgn_changes(2:end) - sgn_changes(1:end-1) <= adj_stp );
	if numel( adj ) >= 1 %then we have adjacent hits
		sgn_changes(adj) = sgn_changes(adj.+1);
		sgn_changes = unique( sgn_changes );
	end

	%now look for the sign changes that also have a positive second
	%derivative
	m_idx = find( dd_data( sgn_changes ) <= 0 );
	m_idx = sgn_changes( m_idx );
	maxima = data_series( m_idx );

	%now, apply the trigger
	t_idx = find( maxima >= trg );
	m_idx = m_idx( t_idx );
	maxima = maxima( t_idx );
	
	%and that should be it.
end
	
