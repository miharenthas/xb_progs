%this function selects hits based on their energy.
%usage: [evt, nb_removed] = xb_data_cut_on_nrg( evt, op_handle )
%       where "op_handle" is a function handle that
%           -takes an aray of energies as argument
%           -returns an array of boolean values (or something that can
%            go into an if statement)
%       NOTE: those for which TRUE is returned are KEPT!

function [evt, nb_removed] = xb_data_cut_on_nrg( evt, op_handle )
	if ~is_function_handle( op_handle )
		error( "Second argument **MUST** be a function handle!" );
	end
	if isempty( evt )
		nb_removed = 0;
		return;
	end
	
	%prepare to spli the events in the number of processes
	nb_events = length( evt );
	adj = mod( nb_events, nproc );
	if adj adj = nproc - adj; end
	idx_part = 1:(nb_events+adj)/nproc:(nb_events+adj);
	idx_part = linspace( idx_part, ...
	                     idx_part+(nb_events+adj)/nproc-1, ...
	                     (nb_events+adj)/nproc );
	
	%split the events into a cell
	evt_part = {};
	evt_rest = [];
	nbr_rest = 0;
	nb_proc = 1;
	for ii=1:nproc
		try
			evt_part(ii) = evt(idx_part(ii,:));
		catch
			%I'm not yet sure this is the most brilliant solution
			%but it's the best I can think right now.
			[evt_rest nbr_rest] = xb_data_cut_on_nrg( evt(idx_part(ii):end), ...
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
		[klz_part, nb_removed_part] = cellfun( proc_handle, klz_part );
	end
	
	%stitch together the stuff
	evt = reshape( evt_part, [], 1 );
	if ~isempty( evt_rest ) evt = [evt(:); evt_rest(:)]; end
	nb_removed = sum( nb_removed_part ) + nbr_rest;

	%prune the empty ones
	evt = evt( find( [evt.n] ) );
end

%processor function
function [evt, nb_removed] = _processor( evt, op_handle )
	%loop clear the uninsteresting stuff.
	nb_removed = sum( [evt.n] );
	for ii=1:length( evt )
		keep_idx = find( op_handle( [evt(ii).e](:) ) );
		if size( evt(ii).i ) evt(ii).i = evt(ii).i( keep_idx ); end
		if size( evt(ii).e ) evt(ii).e = evt(ii).e( keep_idx ); end
		if size( evt(ii).he ) evt(ii).he = evt(ii).he( keep_idx ); end
		if size( evt(ii).t ) evt(ii).t = evt(ii).t( keep_idx ); end
		if size( evt(ii).pt ) evt(ii).pt = evt(ii).pt( keep_idx ); end
		
		%at the end of things, multiplicity update
		evt(ii).n = length( keep_idx );
		evt(ii).sum_e = sum( [evt(ii).e] );
	end

	nb_removed = nb_removed - sum( [evt.n] );
end
