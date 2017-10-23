%this function returns an ellipse structure given a rectangle.
%
% ell = xb_get_ell_from_rect( rect )
%
%parameters:
% rect -- an array describing a rectangle (4 scalars for an unrotated one, 2x4 else)
%         NOTE: if it's not a rectangle, all of YOUR life has no meaning
%
%The rectancle can be rotated, but the ordering of the vertices must be as follows:
%   A B C D
%  [x x x x;
%   y y y y]
%
%  C-------B
%  |       |
%  |       |
%  D-------A
%
%The returned structure will be for direct plug in into the xb_is_ellipse tool.

function ell = xb_get_ell_from_rect( rect )
	if numel( rect ) ~= 4 && numel( rect ) ~= 8
		error( 'rect is not a valid rectanle' );
	end
	
	is_rot = 0;
	if numel( rect ) == 4; rect = rect(:); end
	if numel( rect ) == 8; rect = reshape( rect, 2, 4 ); is_rot = 1; end
	
	if ~is_rot %that's simple
		ell.ctr = [ rect(1) + rect(2), rect(3) + rect(4) ]/2;
		ell.a = abs( rect(2) - rect(1) )/2;
		ell.b = abs( rect(4) - rect(3) )/2;
	else %we have a rotation, deduce it.
		ell.ctr = mean( rect, 2 );
		rect = rect - ell.ctr;
		a = rect(:,1); b = rect(:,2); c = rect(:,3);
		d = rect(:,4); f = [a(1);d(2)];
		ell.a = sqrt( ( b(1) - c(1) )^2 + ( b(2) - c(2) )^2 )/2;
		ell.b = sqrt( ( a(1) - b(1) )^2 + ( a(2) - b(2) )^2 )/2;
		ell.rot = atan( ( f(1) - d(1) )/( f(2) - a(2) ) ) - pi/2;
	end
end
		
		
		
	
