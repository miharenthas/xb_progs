%this function generates a spectrum of a source.
%
% spc = sg_gen_energy_spc( nb_lines, [line, line_strength, ...] );
%
%TODO: proper documentation.

function spc = sg_gen_energy_spc( nb_events, spc_spec )
	if mod( length( spc_spec ), 2 ) ~= 0
		error( 'An odd number of spc specs is wrong. Review your choiches.' );
	end
	
	%name the two aspects
	spc_lines = spc_spec([1:2:length( spc_spec )]);
	l_strengths = spc_spec([2:2:length( spc_spec )]);
	
	%sort according to the line strengths.
	[l_strengths, idx] = sort( l_strengths );
	spc_lines = spc_lines( idx );
	
	%normalize the strengths for their sum
	l_strengths = l_strengths/sum( l_strengths );
	
	%now create the spectrum
	spc = [];
	for ii = 1:length( spc_lines )
		howmany = round( nb_events*l_strengths(ii) );
		spc = [spc; spc_lines(ii)*ones( howmany, 1 )];
	end
	if length( spc ) > nb_events
		spc = spc(1:nb_events); %crop it (we may have a few events more)
	end
	
	%shuffle them around, and that's it
	spc = spc( randperm( length(spc) ) )';
end
