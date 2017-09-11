%this function makes sure that the drone is properly closed.
%
% xb_drone_free( drone )

function xb_drone_free( drone )
	if isfield( drone, 'cmd' )
		xb_drone_ctrl( drone, 'exit' );
		fclose( drone.cmd )
	end
	
	%this is probably not the safest procedure possible
	system( ['rm ', drone.in] );
	system( ['rm ', drone.out] );
end
