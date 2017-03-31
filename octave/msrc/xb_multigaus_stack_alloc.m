%this function allocates a stack of gaussians from a bunch
%of initial guess parameters.
%
% gaus_stack = xb_multigaus_stack_alloc( data_x, nb_gaus )

function gaus_stack = xb_multigaus_stack_alloc( data_abscissa, nb_gaus )
	%loop create the gaussians
	gaus_stack = {};
	for gg=1:nb_gaus
		gaus = @( p ) p(1)*exp( -(data_abscissa-p(2)).^2/p(3)^2 );
		gaus_stack(gg) = gaus;
	end
end
