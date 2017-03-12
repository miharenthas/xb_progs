%this function selects hits based on their energy.
%usage: [evt, nb_removed] = xb_data_cut_on_nrg( evt, op_handle )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [evt, nb_removed] = xb_data_cut_on_nrg( evt, op_handle )
	%klz is a tructure representing the event
	%this will be maintained
	
	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
	end
	
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [evt.n] );
	for ii=1:length( evt )
		[keep_idx, ~] = find( op_handle( [evt(ii).e](:) ) );
		if evt(ii).i; evt(ii).i = evt(ii).i( keep_idx ); end
		if evt(ii).e; evt(ii).e = evt(ii).e( keep_idx ); end
		if evt(ii).he; evt(ii).he = evt(ii).he( keep_idx ); end
		if evt(ii).t; evt(ii).t = evt(ii).t( keep_idx ); end
		if evt(ii).pt; evt(ii).pt = evt(ii).pt( keep_idx ); end
		
		%at the end of things, multiplicity update
		evt(ii).n = length( keep_idx );
	end

	nb_removed = nb_removed - sum( [evt.n] );
end