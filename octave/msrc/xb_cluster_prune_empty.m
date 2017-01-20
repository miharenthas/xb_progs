%this function removes the empty events from a cluster collection

function klz_pruned = xb_cluster_prune_empty( klz )
	multix = [klz.multiplicity];
	multix = multix(:); %now we are a column vector, guaranteed

	[idx, ~] = find( multix );
	klz_pruned = klz(idx);
end
