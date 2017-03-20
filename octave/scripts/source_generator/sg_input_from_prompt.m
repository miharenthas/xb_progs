%-*- texinfo -*-
%@deftypefn {Function file} {[@var{run_file},@var{out_file},@var{nb_events},@var{energy}]} = sg_input_from_prompt()
%This function displays a prompt capable of reading the relevant quantities to sg: run file, output file, number of events and energy.
%The command syntax is a) trivial b) the same as required by the config file.
%@seealso{ sg_input_from_cf, sg_parse_cmd }
%@end deftypefn

function [ out_file, nb_events, spc_spec ] = sg_input_from_prompt()
	%decalre-ish the input variables
	get_out = 0;
	out_file = 'none';
	nb_events = 10;
	spc_spec = [30, 1];
	
	%loop on the prompt
	while ~get_out
		user_says = input( 'sg> ', 'S' );
		if ~user_says
			continue;
		end
		
		[cmd, opts] = sg_parse_cmd( user_says );
		
		switch( cmd )
			case 'out' %sets an output file name
				if isempty( opts )
					disp( 'Error: out requires one argument.' );
				else
					out_file = opts{1};
				end
			case 'events' %sets the number of events
				if isempty( opts )
					disp( 'Error: events requires one argument.' );
				else
					nb_events = str2num( opts{1} );
				end
			case 'spspec' %sets the energy
				if isempty( opts ) !! mod( length( opts ), 2 ) ~= 0
					disp( 'Error: spspec requires an even number of arguments.' );
				else
					spc_spec = [];
					for ii=1:length( opts )
						spc_spec = [spc_spec,str2num( opts{ii} )];
					end
				end
			case 'done' %exits the prompt
				get_out = 1;
			otherwise
				disp( ['"', cmd, '" is not a valid command.'] );
		end
	end
end
