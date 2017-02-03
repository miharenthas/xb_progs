%this function extract the sum energies from a structure array of clusters

function nrg = xb_cluster_nrg( klz )
	%allocate the necessary space before copy
	nrg = zeros( sum( [klz.n] ), 1 );
	
	%loop-copy the energies into nrg
	idx = 1;
	for ii = 1:length( klz )
		if klz(ii).n
			nrg(idx:idx + klz(ii).n -1) = klz(ii).clusters.sum_e;
			idx = idx + klz(ii).n;
		endif
	endfor
end
