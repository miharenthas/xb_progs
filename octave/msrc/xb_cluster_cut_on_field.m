%this function selects cluster based on the content of a field.
%usage: [klz_cut, nb_removed] = xb_cluster_cut_on_nrg( klz, op_handle, field_name )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [klz_cut, nb_removed] = xb_cluster_cut_on_field( klz_cut, op_handle, field_name )
	%klz is a tructure representing the event
	%this will be maintained
	
	%checkin the input
	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
	end
	if ~ischar( field_name )
		error( 'Second argument **MUST** be a string.' );
	end
	
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [klz_cut.multiplicity] );
	for ii=1:length( klz_cut )
		if isfield( klz_cut(ii).clusters, field_name )
			[keep_idx, ~] = find( op_handle( [klz_cut(ii).clusters.( field_name )](:) ) );
			klz_cut(ii).clusters = klz_cut(ii).clusters( keep_idx );
		
			%at the end of things, multiplicity update
			klz_cut(ii).multiplicity = length( klz_cut(ii).clusters );
		else
			warning( ['At index', num2str(ii), ...
			          ' the requested field "', ... 
			          field_name, ' wasn`t found.'] );
		end
	end
	nb_removed = nb_removed - sum( [klz_cut.multiplicity] );
end
