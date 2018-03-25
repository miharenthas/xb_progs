%this is the index operator to find what indexes are in among
%the ones of interest. It has a slightly werid output but
%it's more comfortable this way with the xb_data_cut_on_field tool
% truth_arr = xb_op_cbi( idx_crys, idx_interest )

function truth_arr = xb_op_cbi( idx_crys, idx_interest )
	idx_interest = idx_interest(:)'; %make it a row vecotr
	idx_crys = idx_crys(:); %make this a column vector
	
	truth_m = idx_crys == idx_interest;
	truth_arr = sum( truth_m, 2 );
end
