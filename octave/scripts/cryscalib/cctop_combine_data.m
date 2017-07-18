%This tool alligns the data to the source profiles (they are supposed to be
%in the SAME order) and merges the runs with the same source profile
%So be careful to feed the software with mergeable data!
%
% [data, source_profiles, nb_runZ] = cctop_combine_data( data, source_profile )

function [data, source_profiles, nb_runZ] = cctop_combine_data( data, source_profile )
	if numel( source_profile ) ~= numel( data )
		error( 'different number of runs and source profiles.' );
	end

	%returns TRUE when the vectors are equal, FALSE otherwiese
	%or an error if they have different shapes.
	__ck_vect = @( p, q ) isempty( find( p ~= q ) ); %p and q are matrices of the same size
	
	%do some renaming
	r_data = data;
	r_sp = source_profile;
	clear data, source_porfile; %data might be huge, cleanup.
	data = {};
	source_profle = {};
	while ~isempty( source_profile )
		__ck_this = @( p ) __ck_vec( p, r_sp{1} ); %make a check for this sp
		idx = cellfun( __ck_this, srouce_profile ); %check it on all of them
		
		%move about data:
		%save for this elaboration
		d_buf = r_data( idx );
		%and for the next
		r_data = r_data( ~idx );
		r_sp = r_sp( ~idx );
		
		%cat the data
		data( numel( data )+1 ) = [];
		for ii=1:numel( d_buf )
			data(end) = [data{end}(:); d_buf{ii}(:)];
		end 
		source_profile( numel( source_profile )+1 ) = r_sp{1};
	end
	
	%number of surviving runs
	nb_runZ = numel( source_profile );
end
		
