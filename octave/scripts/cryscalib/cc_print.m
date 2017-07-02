%this function prints the calibration data into a hooman readable file
%
% cc_print( file{_name}, writin_mode, c_pees, cp_err, dE_E )
%
% -- file{_name}: either a file pointer or a file name, the output's target
% -- writing_mode: either "w", for overwrite, or "a", for append
%                  (mute if the file is already opened).
% -- crystal_number: the number of the crystal
% -- c_pees: calibration parameters (come out of cc_do_calib)
% -- cp_err: calibration parameters errorZ.
% -- dE_E: the energy resolution (for each peak, the user will decide what to do)
%returns nothing.
%outputs a file.

function cc_print( file, writing_mode, crystal_number, c_pees, cp_err, dE_E )
	if ischar( file )
		file = fopen( file, writing_mode );
	end
	
	fprintf( file, 'Crystal number %d\n', crystal_number );
	
	fprintf( file, 'Calibration' );
	fprintf( file, ' %f', c_pees );
	fprintf( file, '\n' );
	
	fprintf( file, 'Errors' );
	fprintf( file, ' %f', cp_err );
	fprintf( file, '\n' );
	
	fprintf( file, 'dE_E' )
	fprintf( file, ' %f', dE_E );
	fprintf( file, '\n' );
	
	fprintf( file, '\n######\n\n' );

	fclose( file );
end
