%this function creates a density matrix for 2-tuples of datas

function [ d_matrix, i_centroids, j_centroids ] = xb_density( tuples, i_div, j_div )

	%straighten the tuples
	if size( tuples, 2 ) ~= 2; tuples = tuples'; end

	%work out the various bins
	if isscalar( i_div ); i_spans = [min( tuples(:,1)  ): ...
	                                 (max( tuples(:,1) ) - min( tuples(:,1) ))/i_div: ...
	                                 max( tuples(:,1) )];
	else i_spans = i_div; end
	i_div = length( i_spans )-1;

	if isscalar( j_div ); j_spans = [min( tuples(:,2)  ): ...
	                                 (max( tuples(:,2) ) - min( tuples(:,2) ))/j_div: ...
	                                 max( tuples(:,2) )];
	else j_spans = j_div; end
	j_div = length( j_spans )-1;
	
	%make the matrix
	d_matrix = zeros( i_div, j_div );

	%fill the matrix
	for ii=1:i_div
		[is, ~] = find( tuples(:,1) < i_spans(ii+1) & tuples(:,1) >= i_spans(ii) );
		for jj=1:j_div
			[js, ~] = find( tuples(is,2) < j_spans(jj+1) & tuples(is,2) >= j_spans(jj) );
			d_matrix(ii,jj) = numel( js );
		end
	end

	%do the centroids
	i_centroids = (i_spans(1:end-1) + i_spans(2:end))./2;
	j_centroids = (j_spans(1:end-1) + j_spans(2:end))./2;
end
