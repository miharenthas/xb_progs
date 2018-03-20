%this function extract the sum-over-event energies from a structure array of clusters

function sumnrg = xb_cluster_sumnrg( klz )
	%allocate the necessary space before copy
	sumnrg = zeros( length( klz ), 1 );
	
	%loop-copy the energies into nrg
	for ii = 1:length( klz )
		if klz(ii).n
			sumnrg(ii) = sum( [klz(ii).clusters.sum_e] );
		endif
	endfor
end
