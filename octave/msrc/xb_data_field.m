%this function extract the sum energies from a structure array of events (data)

function field = xb_data_field( evt, field_name )
	%allocate the necessary space before copy
	field = zeros( sum( [evt.n] ), 1 );
	
	%checking the input
	if ~ischar( field_name )
		error( 'Second argument **MUST** be a string.' );
	end
	
	%loop-copy the energies into nrg
	idx = 1;
	for ii = 1:length( evt )
		try
			if evt(ii).n
				if isfield( evt(ii), field_name )
					if ~isempty( [evt(ii).( field_name )] )
						field(idx:idx + evt(ii).n -1) = ...
							[evt(ii).( field_name )];
					else
						field(idx:idx + evt(ii).n -1) = ...
							zeros( evt(ii).n, 1 );
					end
					idx = idx + evt(ii).n;
				else
					warning( ['At index', num2str(ii), ...
							' the requested field "', ...
							field_name, '" wasn`t found.'] );
				end
			endif
		catch err
			warning( ["Something happened: ", err.message] );
		end %catch
	endfor
end
