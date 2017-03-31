%this function executes a gaussian stack given an array of parameters
%
%fcn_eval = xb_multigaus_stack_exec( parameters, gaus_stack )

function fcn_eval = xb_multigaus_stack_exec( parameters, gaus_stack )
	%loop-sum the function outputs
	fcn_eval = 0;
	for gg = 1:numel( gaus_stack )
		par_point = 3*(gg-1)+1;
		fcn_eval += gaus_stack{gg}( parameters(par_point:par_point+2) );
	end
end
