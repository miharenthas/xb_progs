%this function extract the sum energies from a structure array of clusters

function field = xb_cluster_field( klz, field_name )
	%allocate the necessary space before copy
	field = zeros( sum( [klz.n] ), 1 );
	
	%checking the input
	if ~ischar( field_name )
		error( 'Second argument **MUST** be a string.' );
	end
	
	%loop-copy the energies into nrg
	idx = 1;
	for ii = 1:length( klz )
		if klz(ii).n
			if isfield( klz(ii).clusters, field_name )
				field(idx:idx + klz(ii).n -1) = ...
				    klz(ii).clusters.( field_name );
				idx = idx + klz(ii).n;
			else
				warning( ['At index', num2str(ii), ...
				          ' the requested field "', ...
				          field_name, '" wasn`t found.'] );
			end
		endif
	endfor
end
