%This function allows to pass command strings to the drone in a somewhat easier way
%than just fputs( ... ); fflush. But it's the same thing really.
%
% drone = xb_drone_ctrl( drone, 'semicolumn; separated; commands' );
%
%NOTE: while a grammar check on the command string is possible, it's not implemented (yet).
%      This means that you have the freedom of screwing up.

function drone = xb_drone_ctrl( drone, command_str )
	if ~isfield( drone, 'cmd' )
		drone.cmd = fopen( drone.in, 'w' );
		if drone.cmd < 0
			error( "This drone's input cannot be opened" );
		end
	end
	
	fputs( drone.cmd, [command_str, "\n"] ); fflush( drone.cmd );
end
