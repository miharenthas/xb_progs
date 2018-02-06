%this function extract the sum energies from a structure array of events (data)

function nrg = xb_data_nrg( evt )
	%allocate the necessary space before copy
	nrg = zeros( sum( [evt.n] ), 1 );
	
	%loop-copy the energies into nrg
	idx = 1;
	for ii = 1:length( evt )
		if evt(ii).n
			if ~isempty( [evt(ii).e] )
				nrg(idx:idx + evt(ii).n -1) = [evt(ii).e];
			else
				nrg(idx:idx + evt(ii).n -1) = zeros( evt(ii).n, 1 );
			end
			idx = idx + evt(ii).n;
		endif
	endfor
end
