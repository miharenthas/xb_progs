%this function extract the sum-over-event energies from single deposit data

function sumnrg = xb_data_sumnrg( data )
	%allocate the necessary space before copy
	sumnrg = zeros( length( data ), 1 );
	
	%loop-copy the energies into nrg
	for ii = 1:length( data )
		if data(ii).n
			sumnrg(ii) = sum( [data(ii).e] );
		endif
	endfor
end
