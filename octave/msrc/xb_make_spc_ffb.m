%this routine produces a comparison between full, front and back spectrum
%of some data set ({a,}data or cluster) for the various interesting section of
%the CB
%
% [hst_ffb, binz_ffb, herr_ffb] = xb_make_spc_ffb( dataset, [figure] )
%

function [hst_ffb, binz_ffb, herr_ffb] = xb_make_spc_ffb( dataset, bin, varargin )
	if isfield( dataset, 'clusters' )
		iswhat = 'cluster';
	elseif isfield( dataset, 'e' ) && isfield( dataset, 'he' )
		iswhat = 'data';
	else
		error( 'dataset is neither cluster nor data' );
	end
	
	if nargin == 2 && nargout == 0
		fig = figure;
	elseif nargin == 3
		fig = varargin{1};
		hold on;
	end
	
	hst_ffb = cell( 3, 1 );
	binz_ffb = cell( 3, 1 );
	herr_ffb = cell( 3, 1 );
	nrg = cell( 3, 1 );
	icbf = [xb_ball_neigh( 81, 5 ).i];
	icbb = [xb_ball_neigh( 82, 5 ).i];
	ohf = @(p) xb_op_cbi( p, icbf );
	ohb = @(p) xb_op_cbi( p, icbb );
	
	if strcmp( iswhat, 'data' )
		front = xb_data_cut_on_field( dataset, ohf, 'i' );
		back = xb_data_cut_on_field( dataset, ohb, 'i' );
		nrg(1) = xb_data_nrg( dataset );
		nrg(2) = xb_data_nrg( front );
		nrg(3) = xb_data_nrg( back );
	elseif strcmp( iswhat, 'cluster' )
		front = xb_cluster_cut_on_field( dataset, ohf, 'centroid_id' );
		back = xb_cluster_cut_on_field( dataset, ohb, 'centroid_id' );
		nrg(1) = xb_cluster_nrg( dataset );
		nrg(2) = xb_cluster_nrg( front );
		nrg(3) = xb_cluster_nrg( back );
	end
	clear dataset front back;
	
	[hst_ffb(1), binz_ffb(1), herr_ffb(1)] = xb_make_spc( nrg{1}, bin );
	[hst_ffb(2), binz_ffb(2), herr_ffb(2)] = xb_make_spc( nrg{2}, bin );
	[hst_ffb(3), binz_ffb(3), herr_ffb(3)] = xb_make_spc( nrg{3}, bin );
	
	if exist( 'fig' ) && isfigure( fig );
		figure( fig );
		stairs( binz_ffb{1}, hst_ffb{1}, 'linewidth', 2 ); hold on;
		stairs( binz_ffb{2}, hst_ffb{2}, 'linewidth', 2 );
		stairs( binz_ffb{3}, hst_ffb{3}, 'linewidth', 2 );
		set( gca, 'fontsize', 24, 'linewidth', 2, 'yscale', 'log' );
		grid on;
		if isscalar( bin ); bstep = bin;
		else bstep = bin(2)-bin(1); end
		ylabel( ['#/',num2str(bstep),'KeV'] );
		xlabel( 'KeV' );
		legend( { 'full CB', 'front CB', 'back CB' } );
	end
end
