%a tiny tool to make spectra a bit faster.
%
% [h, b, herr] = xb_spc( energy_deps, bin, 'title' )
%
% energy_deps : the energy deposits ( comes out of xb_*_nrg() )
% bin : either the bin span or an array with the bin delimiters (as in histogram)
% ax : X axis range
% 'title' : a string with the title for the spectrum.

function [h, b, herr] = xb_spc( energy_deps, bin, ax, the_title )
	[h, b] = hist( energy_deps, max( energy_deps )/bin );
	herr = sqrt( h );
	
	plot( b, h, '+', 'linewidth', 2 );
	set( gca, 'linewidth', 2, 'yscale', 'log' );
	if nargin >= 3
		axis( ax );
	else
		axis( [0 6000] );
	end
	grid on;
	
	if nargin == 4
		title( the_title );
	end
	xlabel( 'KeV' );
	ylabel( ['#/',num2str( bin ),'KeV'] );
end
