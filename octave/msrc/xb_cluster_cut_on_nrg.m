%this function selects cluster based on their total energy.
%usage: [klz, nb_removed] = xb_cluster_cut_on_nrg( klz, op_handle )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [klz, nb_removed] = xb_cluster_cut_on_nrg( klz, op_handle )
	%klz is a tructure representing the event
	%this will be maintained
	

	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
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
			[klz_rest nbr_rest] = xb_cluster_cut_on_field( klz(idx_part(ii):end), ...
			                                               op_handle );
			break;
		end
		nb_proc += 1;
	end

	%do the parallel execution
	proc_handle = @( p ) _processor( p, op_handle, field_name );
	try
		pkg load parallel;
		[klz_part, nb_removed_part] = parcellfun( nb_proc, proc_handle, ...
		                                          klz_part, 'VerboseLevel', 0 );
	catch
		warning( 'Parallel package not available. This will take a while.' );
		[klz_part, nb_removed_part] = cellfun( proc_handle, klz_part, 'UniformOutput', false );
		klz_part = cell2mat( klz_part );
		nb_removed_part = sum( cell2mat( nb_removed_part ) );
	end
	
	
	%stitch together the stuff
	klz = reshape( klz_part, 1, [] );
	if ~isempty( klz_rest ) klz = [klz(:); klz_rest(:)]; end
	nb_removed = sum( nb_removed_part ) + nbr_rest;	

	%remove empty events.
	evt = evt( find( [evt.n] ) );
end

%the processor function
function [klz, nb_removed] = _processor( klz, op_handle )
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [klz.n] );
	for ii=1:length( klz )
		keep_idx = find( op_handle( [klz(ii).clusters.sum_e](:) ) );
		klz(ii).clusters = klz(ii).clusters( keep_idx );
		
		%at the end of things, multiplicity update
		klz(ii).n = length( klz(ii).clusters );
	end
	nb_removed = nb_removed - sum( [klz.n] );
end
