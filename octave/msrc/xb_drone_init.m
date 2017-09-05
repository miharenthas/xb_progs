%this function initializes a drone struct (it's not a class, because of lazy)
%takes in the arguments to pass to the program at launch.
%it's a string.
%
% drone = xb_drone_init()
% drone = xb_drone_init( 'command', 'line', 'arguments' );
%
%NOTE: of course, it requires the dronized version of xb_make_spc

function drone = xb_drone_init( varargin )
	%first, check and make some pipes
	%make unique, novel names
	do
		drone.in = sprintf( '.drin_%f', rand( 1 ) );
	until ~exist( drone.in, 'file' );
	do
		drone.out = sprintf( '.drout_%f', rand( 1 ) );
	until ~exist( drone.out, 'file' );
	
	%open the program
	command = 'xb_make_spc ';
	if ~isempty( varargin )
		for aa=1:numel( varargin )
			%ignore any "--drone" option.
			if ischar( varargin{aa} ) && ...
			   ( findstr( varargin{aa}, '--drone' ) || ...
			     findstr( varargin{aa}, '-D' ) )
				continue;
			end
			if ischar( varargin{aa} )
				command = [command, varargin{aa}, ' '];
			end
		endfor
	endif
	command = [command, '--drone f:', drone.in, ':f:', drone.out, ' &' ];
	
	%and now, make the pipes.
	system( ['mkfifo ', drone.in] );
	system( ['mkfifo ', drone.out] );
	
	%and finally, issue the command.
	system( command );
end
	
	 
			
