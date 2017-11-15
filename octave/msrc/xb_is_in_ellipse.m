%this function finds the points inside an ellipse of given semiaxes
%
% [idx_inside, idx_outside] = xb_is_in_ellipse( Xs, Ys, ell )
%
%parameters:
% Xs -- an array with the X coordinates in it.
% Ys -- an array with the Y coordinates in it.
%       NOTE: Xs and Ys MUST have the same size something-by-one
% ell -- a scalar structure with at least the fields:
%        a -- the semiaxis parallel to X
%        b -- the semiaxis parallel to Y
%        ctr -- the centroid of the ellipse
%        A field "rot" setting a rotiation CCwise may be provided.

function [idx_inside, idx_outside] = xb_is_in_ellipse( Xs, Ys, ell )
	if size( Xs ) ~= size( Ys )
		error( 'Xs and Ys MUST have the same size' );
	end
	
	if ~isfield( ell, 'a' ) || ~isfield( ell, 'b' ) || ~isfield( ell, 'ctr' )
		error( 'ell is not an ellipse' );
	end
	
	if length( ell.ctr ) ~= 2 error( 'ell.ctr is not a point.' ); end
	
	%bring the data in the ellipse's frame of reference
	%it's easier this way
	Xs = Xs(:)-ell.ctr(1);
	Ys = Ys(:)-ell.ctr(2);
	if isfield( ell, 'rot' );
		rotM = [ cos( ell.rot ) sin( ell.rot ); -sin( ell.rot ) cos( ell.rot )];
		xy = rotM*[Xs';Ys'];
		Xs = xy(1,:); Ys = xy(2,:);
	end
	
	ell_eval = Xs.^2/ell.a^2 + Ys.^2/ell.b^2;
	
	idx_inside = find( ell_eval <= 1 );
	idx_outside = find( ell_eval > 1 );
end
