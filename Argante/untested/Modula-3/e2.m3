MODULE Example ;

VAR
	z : INTEGER ;

PROCEDURE Mul( a,b : INTEGER ) : INTEGER = 
	VAR x : INTEGER := 444;
	BEGIN
		x := a * b ;
		z := z + x ; 		
(*		RETURN x ; *)
	END Mul;

VAR 
	x : INTEGER := 1 ;
	y : INTEGER := 3 ;
BEGIN
	x := 5;
	y := 7;
(*	y := Mul( x,y ); *)
END Example.
