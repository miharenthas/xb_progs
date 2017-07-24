%this function selects cluster based on the content of a field.
%usage: [klz, nb_removed] = xb_cluster_cut_on_nrg( klz, op_handle, field_name )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [klz, nb_removed] = xb_cluster_cut_on_field( klz, op_handle, field_name )
	%klz is a tructure representing the event
	%this will be maintained
	%first, check on the package
	try
		pkg load parallel;
	catch
		error( 'parallel package not installed!' );
	end
	
	%checkin the input
	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
	end
	if ~ischar( field_name )
		error( 'Third argument **MUST** be a string.' );
	end
	
	if isempty( klz )
		nb_removed = 0;
		return;
	end
	
	%prepare to spli the events in the number of processes
	nb_events = length( klz );
	adj = mod( nb_events, nproc );
	if adj adj = nproc - adj; end
	idx_part = 1:(nb_events+adj)/nproc:(nb_events+adj);
	idx_part = linspace( idx_part, ...
	                     idx_part+(nb_events+adj)/nproc-1, ...
	                     (nb_events+adj)/nproc );
	
	%split the events into a cell
	klz_part = {};
	klz_rest = [];
	nbr_rest = 0;
	nb_proc = 1;
	for ii=1:nproc
		try
			klz_part(ii) = klz(idx_part(ii,:));
		catch
			%I'm not yet sure this is the most brilliant solution
			%but it's the best I can think right now.
			[klz_rest nbr_rest] = xb_data_cut_on_field( klz(idx_part(ii):end), ...
			                                            op_handle, field_name );
			break;
		end
		nb_proc += 1;
	end

	%do the parallel execution
	proc_handle = @( p ) _processor( p, op_handle, field_name );
	[klz_part, nb_removed_part] = parcellfun( nb_proc, proc_handle, klz_part, 'VerboseLevel', 0 );
	
	%stitch together the stuff
	klz = reshape( klz_part, 1, [] );
	if ~isempty( klz_rest ) klz = [klz, klz_rest]; end
	nb_removed = sum( nb_removed_part ) + nbr_rest;
	
	%remove empty events.
	evt = evt( find( [evt.n] ) );
	
end

%the usual processor function
function [klz, nb_removed] = _processor( klz, op_handle, field_name )
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [klz.n] );
	for ii=1:length( klz )
		if isfield( klz(ii).clusters, field_name )
			keep_idx = find( op_handle( [klz(ii).clusters.( field_name )](:) ) );
			klz(ii).clusters = klz(ii).clusters( keep_idx );
		
			%at the end of things, multiplicity update
			klz(ii).n = length( klz(ii).clusters );
		else
			warning( ['At index', num2str(ii), ...
			          ' the requested field "', ... 
			          field_name, ' wasn`t found.'] );
		end
	end
	nb_removed = nb_removed - sum( [klz.n] );
end
