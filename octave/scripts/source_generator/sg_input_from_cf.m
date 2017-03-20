%-*- texinfo -*-
%@deftypefn {Function file} {[@var{run_file},@var{out_file},@var{nb_events},@var{energy}]} = sg_input_from_cf( @var{config_file} )
%This function drives the parsing of a config file and returns the four relevant entries to the sg script.
%The config file shall be formatted as follows:
%Everything behing an hash sign will be regarded as a comment and left out.
%Every line shall contain one single command with its arguments.
%Valid commands are: run, out, events, energy.
%The first two expect a string, the second two expect a numeric value.
%@seealso{ sg_input_from_prompt, sg_parse_cmd }
%@end deftypefn

function [ out_file, nb_events, spc_spec ] = sg_input_from_cf( cf_name )
	%decalre-ish the input variables
	out_file = 'none';
	nb_events = 10;
	spc_spec = [30, 1];
	
	%check the existence of a file
	if ~exist( cf_name, 'file' )
		error( 'The config file does not exist.' );
	end

	cf = fopen( cf_name );

	%loop on the file's lines
	while ~feof( cf )
		user_says = fgetl( cf );
		if ~user_says
			continue;
		end
		
		[cmd, opts] = sg_parse_cmd( user_says );
		
		switch( cmd )
			case 'out' %sets an output file name
				if isempty( opts )
					disp( 'sg: syntax error: out requires one argument.' );
				else
					out_file = opts{1};
				end
			case 'events' %sets the number of events
				if isempty( opts )
					disp( 'sg: syntax error: events requires one argument.' );
				else
					nb_events = str2num( opts{1} );
				end
			case 'spspec' %sets the energy
				if isempty( opts )
					disp( 'sg: syntax error: spspec requires an even number of arguments.' );
				else
					spc_spec = [];
					for ii=1:length( opts )
						spc_spec = [spc_spec,str2num( opts{ii} )];
					end
				end
		end
	end
end
