%this function extracts data from the drone, either an histogram 
%or cluster data
%
% some_data = xb_drone_get( drone, ["histo"|"klz"] )
%
%NOTE: on anything other than linux, probably "klz" won't work and
%      you'll have to get a histogram directly.

function some_data = xb_drone_get( drone, type_str )
	if nargin == 1 || ~ischar( type_str );
		type_str = 'histo';
	elseif nargin == 0
		error( 'Need at least a drone to work with.' );
	end

	if strcmp( type_str, 'klz' )
		try
			xb_drone_ctrl( drone, 'put data; go' );
			some_data = xb_load_clusterZ( 'no-compression', drone.out );
			return;
		catch
			warning( 'Something went wrong with the cluster reader. Doing histo instead.' );
		end
	end
	
	%read the histogram (sadly, "load" doesn't work)
	xb_drone_ctrl( drone, 'put hist; go' );
	data = fopen( drone.out );
	some_data = [];
	while ( l = fgetl( data ) ) ~= -1
		some_data = [some_data; sscanf( l, '%f %f' )(:)'];
	end
	fclose( data );
		
end
