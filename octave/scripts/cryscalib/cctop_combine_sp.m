%This tool alligns merges the source profiles
%
% source_profile = cctop_combine_data( data, source_profile )

function source_profile = cctop_combine_sp( data, source_profile )
	%merge the source profiles
	source_profile = cell2mat( source_profile );
	[~, ii] = sort( source_profile(1,:) );
	source_profile = source_profile(:,ii);
end
		
