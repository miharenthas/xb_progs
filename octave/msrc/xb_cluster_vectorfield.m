%this function extract the vector fields from a structure array of clusters

function field = xb_cluster_vectorfield( klz, field_name )
	%checking the input
	if ~ischar( field_name )
		error( 'Second argument **MUST** be a string.' );
	end
	
	%loop-copy the energies into nrg
	idx = 1;
	field = [];
	for ii = 1:length( klz )
		if klz(ii).n
			if isfield( klz(ii).clusters, field_name )
				for kk = 1:length( klz(ii).clusters )
					field = [field; [klz(ii).clusters(kk).( field_name )](:)];
				end
			else
				warning( ['At index', num2str(ii), ...
				          ' the requested field "', ...
				          field_name, '" wasn`t found.'] );
			end
		endif
	endfor
end
