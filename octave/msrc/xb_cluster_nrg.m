%this function extract the sum energies from a structure array of clusters

function nrg = xb_cluster_nrg( klz )
	%allocate the necessary space before copy
	nrg = zeros( sum( [klz.multiplicity] ), 1 );
	
	%loop-copy the energies into nrg
	idx = 1;
	for ii = 1:length( klz )
		if klz(ii).multiplicity
			nrg(idx:idx + klz(ii).multiplicity -1) = klz(ii).clusters.sum_e;
			idx = idx + klz(ii).multiplicity;
		endif
	endfor
end
