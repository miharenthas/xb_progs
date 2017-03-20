%This scripts generates a source-like spectrum to be fed into the R3B ASCII generator
%
% source_generator()
% source_generator( config_file )
% source_generator( output_file, nb_events, spc_specs )
%
%TODO: make proper documentation

function source_generator( varargin )
	disp( 'Welcome in SG - a source generator!' );
	%keep track of the time
	
	if ~nargin
		%input from prompt
		[ out_file, nb_events, spc_spec ] = sg_input_from_prompt();
	elseif nargin == 1
		if ischar( varargin{1} )
			%load a config file with the settings
			sg_input_from_cf( varargin{1} );
		else
			error( 'One agument given: I expected a path to a config file.' );
		end
	elseif nargin == 3
		out_file = varargin{1}; %where we're going to save th events
		nb_events = varargin{2}; %number of events to be generated
		spc_spec = varargin{3}; %the energy of the photons.
	end
	
	disp( 'SG will run with:' );
	printf( ['\toutput file: ',out_file,'\n'] );
	printf( ['\t# events: ',num2str(nb_events),'\n'] );
	str = '\tspectrum (KeV, I, ...): ';
	for ii=1:length( spc_spec )
		str = [str,num2str( spc_spec(ii) ),' '];
	end
	printf( [str, '\n'] );
	
	%timing gear
	init_t = time;
	
	%do the spectrum
	printf( 'Preparing spectrum...' );
	spc = sg_gen_energy_spc( nb_events, spc_spec );
	disp( [' done. Elapsed time: ', num2str( time - init_t )] );
	
	%do the momenta
	printf( 'Preparing momenta...' );
	momenta = sg_gen_momenta( spc );
	momenta *= 1e-6; %r3broot friendly scaling
	disp( [' done. Elapsed time: ', num2str( time - init_t )] );
	
	%allocate the tracks and events and save them (one track per event)
	printf( 'Saving... ' );
	evts = r3bascii_evnts_alloc( nb_events, 'nTracks', 1 );
	trks = r3bascii_tracks_alloc( nb_events, 'iPid', 1, 'iA', 0, 'iZ', 22, ...
	                              'px', momenta(1,:), ...
	                              'py', momenta(2,:), ...
	                              'pz', momenta(3,:) );
	r3bascii_write_compressed_events( trks, evts, out_file );
	disp( [' done. Elapsed time: ', num2str( time - init_t )] );
	
	disp( 'Completed. Goodbye.' ); 
end
