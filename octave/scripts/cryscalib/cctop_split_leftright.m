%This tool splits calibration runs left and right
%and will be painfully slow.
%
% split_data = cctop_split_leftright( data )

function split_data = cctop_split_leftright( data )
	split_data = cell( 1, 2*numel(data) );
	op_left = @( p ) p <= 82;
	op_right = @( p ) p > 82;
	for rr=0:numel( data )-1
		rrp = rr+1;
		split_data(2*rr+1) = xb_data_cut_on_field( data{rrp}, op_left, 'i' );
		split_data(2*rrp) = xb_data_cut_on_field( data{rrp}, op_right, 'i' );
	end
	clear data;
end
