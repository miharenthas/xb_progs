%this function executes a gaussian stack given an array of parameters
%
%fcn_eval = xb_multigaus_stack_exec( parameters, gaus_stack )

function fcn_eval = xb_multigaus_stack_exec( parameters, gaus_stack )
	%check that the number of parametes makes sense
	if numel( parameters ) == 0 || mod( numel( parameters ), 3 ) != 0
		error( 'Number of parameters is not consistent.' );
	end

	%loop-sum the function outputs (assuming we have at least one)
	fcn_eval = gaus_stack{1}( parameters(1:3) );
	for gg = 2:numel( gaus_stack )
		par_point = 3*(gg-1)+1;
		fcn_eval += gaus_stack{gg}( parameters(par_point:par_point+2) );
	end
end
