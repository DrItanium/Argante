MODULE Stuff ;

PROCEDURE WRITEINT( x : INTEGER );

VAR 
	a := 1 ;
CONST
	SIX = 6.2 ;
	SEVEN = SIX + 1 ;

PROCEDURE beatles( x : FLOAT := 0; READONLY a,b : INTEGER := 7) : FLOAT  = 
	BEGIN
		x := x + SIX ;
		a := b DIV a ;	
		b := a MOD b ;
		a := a * 2 ;
		RETURN x*a+b ;
	END beatles ;
VAR
	x : INTEGER ;
	y,z : FLOAT ;
BEGIN
	x := 1;
	y := 3;
	z := SEVEN + 1 ;
	beatles(x,y,z);

END Stuff.
