%this functio attempts to partition a ROI to drive the guesses
%Again, this is not a precision tool but it's mostly thought
%to assist in hand calibration of crystals.
%
% [roi_spans, roi_centroids] = xb_roi_partition( data_series, max_idx )
%
%NOTE: max_idx is the output of xb_multigaus_find

function [roi_spans, roi_centroids] = xb_roi_partition( data_series, max_idx )
	%if we have just one maximum, then it's trivial
	if isscalar( max_idx )
		roi_centroids = data_series(1,max_idx);
		roi_spans = min( 200, data_series(1,end) - data_series(1,1) );
		return;
	end

	%else, do this
	roi_centroids = data_series(1,max_idx);
	roi_spans = (roi_centroids(2:end) + roi_centroids(1:end-1))/2; %these are the inner spans
	                                                   %between the gaussians
	roi_spans = roi_spans(:)'; %make sure it's a row vector
	%add the first span
	roi_spans = [ max( roi_centroids(1) - roi_spans(1), min( data_series(1,:) ) ), roi_spans ];
	%add the last span
	roi_spans = [ roi_spans, min( 2*roi_centroids(end) - roi_spans(end), max( data_series(1,:) ) ) ];
	
	%happy thougths
end
