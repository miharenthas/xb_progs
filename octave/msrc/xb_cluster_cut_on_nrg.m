%this function selects cluster based on their total energy.
%usage: [klz_cut, nb_removed] = xb_cluster_cut_on_nrg( klz, op_handle )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [klz_cut, nb_removed] = xb_cluster_cut_on_nrg( klz_cut, op_handle )
	%klz is a tructure representing the event
	%this will be maintained
	
	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
	end
	
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [klz_cut.multiplicity] );
	for ii=1:length( klz_cut )
		[keep_idx, ~] = find( op_handle( [klz_cut(ii).clusters.sum_e](:) ) );
		klz_cut(ii).clusters = klz_cut(ii).clusters( keep_idx );
		
		%at the end of things, multiplicity update
		klz_cut(ii).multiplicity = length( klz_cut(ii).clusters );
	end
	nb_removed = nb_removed - sum( [klz_cut.multiplicity] );
end
