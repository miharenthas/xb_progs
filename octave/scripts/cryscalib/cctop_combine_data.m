%This tool alligns merges the data and the source profiles.
%So be careful to feed the software with mergeable data!
%
% [data, source_profiles] = cctop_combine_data( data, source_profile )

function [data, source_profiles] = cctop_combine_data( data, source_profile )
	%merge the source profiles
	source_profile = cell2mat( source_profile );
	[~, ii] = sort( source_profile(1,:) );
	source_profile = source_profile(:,ii);

	%merge the data
	data = cell2mat( data ); %this should do it. No sort or cuts.
end
		
