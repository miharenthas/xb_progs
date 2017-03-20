%-*- texinfo -*-
%@deftypefn {Function file} {@var{cmd},@var{opts}} = sg_parse_cmd( @var{a_string} )
%This function parses a string into a command and arguments. everything behind an hash sign is ignored as a comment.
%@seealso{ sg_input_from_prompt, sg_input_from_cf }
%end deftypefn

function [cmd, arguments] = sg_parse_cmd( a_string )
	%first, get rid of the comments
	idx = find( a_string == '#' );
	if isscalar( idx ) && idx > 1 %it's partlty a comment
		a_string = a_string(1:idx-1);
		while a_string(end) == ' ' || a_string(end) == '\t'
			a_string = a_string(1:end-1);
		end
	elseif isscalar( idx ) && idx == 1
		cmd = [];
		arguments = {};
		return; %it's a whole commend
	end
	
	%then, tokenize the string in command and options
	cmd = [];
	arguments = {};
	[cmd, a_string] = strtok( a_string );
	
	idx = 1;
	while a_string
		[arguments(idx), a_string] = strtok( a_string );
		idx += 1;
	end
end
