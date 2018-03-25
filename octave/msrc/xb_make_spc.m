%this tiny script produces a spectrum
%and I'm really pretty sure I did this already once and then it got lost
% [hst, binz] = xb_make_spc( energy, bin, [figure] )
%
%Where "bin" is either the width of the bin or an array defining the binnage
%and figure is a figure handle to update.

function [hst, binz, herr] = xb_make_spc( energy, bin, varargin )
	if nargin == 2 && nargout == 0
		fig = figure;
	elseif nargin == 3
		fig = varargin {1};
		hold on;
	elseif nargin ~= 2
		error( 'Inconsistent arguments' );
	end
	
	if isscalar( bin )
		[hst, binz] = hist( energy, max( energy )/bin );
	elseif isvector( bin )
		[hst, binz] = hist( energy, bin );
	else
		error( 'bin needs to be either a scalar or a vector.' );
	end
	
	herr = sqrt( hst );
	
	if exist( 'fig' ) && isfigure( fig )
		figure( fig );
		stairs( binz, hst, 'linewidth', 2 );
		set( gca, 'fontsize', 24, 'linewidth', 2, 'yscale', 'log' );
		grid on;
		if isscalar( bin ) bstep = bin;
		else bstep = bin(2) - bin(1); end
		ylabel( ['#/',num2str(bstep),'KeV'] );
		xlabel( 'KeV' );
	end
end
