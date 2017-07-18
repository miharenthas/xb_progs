%this tool parses a simple text file containing the source profile
%the format is as follows:
% Lines <float> ...
% Intensities <float> ...
%An error is thrown if the two lists are of different length.
%
% source_profle = cctop_parse_sp( sp_name )

function source_profile = cctop_parse_sp( sp_name )
	if ~ischar( sp_name )
		error( 'String requires, something else given' );
	end

	file = fopen( sp_name, 'r' );
	source_profile = [];
	while ~feof( file )
		ll = fgetl( file );
		[cmd, args] = cc_parse_cmd( ll );
		if strcmp( cmd, 'Lines' ) source_profile = cellfun( 'str2num', args )(:)';
		else source_profile = [source_profile; cellfun( 'str2num', args )(:)'];
	end
	fclose( file );
end
