#!/usr/bin/perl
@words=(
	"AND","ANY","ARRAY","AS","BEGIN","BITS","BRANDED",
	"BY","CASE","CONST","DIV","DO","ELSE","ELSIF","END","EVAL","EXCEPT",
	"EXCEPTION","EXIT","EXPORTS","FINALLY","FOR","FROM","GENERIC",
	"IF","IMPORT","IN","INTERFACE","LOCK","LOOP","METHODS","MOD",
	"MODULE","NOT","OBJECT","OF","OR","OVERRIDES","PROCEDURE",
	"RAISE","RAISES","READONLY","RECORD","REF","REPEAT",
	"RETURN","REVEAL","ROOT","SET","THEN","TO","TRY","TYPE",
	"TYPECASE","UNSAFE","UNTIL","UNTRACED","VALUE","VAR",
	"WHILE","WITH" );
for $w ( @words ) {
	print "\t\tcase $w : return \"$w\" ;\n"
}

for $w ( @words ) {
	print "$w\t\t{\tydebug(\"$w\");\treturn $w; }\n";
}
	
$i=0;
for $w ( @words ) {
	if (!($i%6)) {
		print "%token\t";
	}
	print "$w ";
	if (!(++$i%6)) {
		print "\n";
	}
}
print"\n";
@operkeys=(
	"+","-","*","/","<",">","<=",">=",
	"#","{","(","[","=","}",")","]",
	";","|","^",".","..",":=",",","\&",
	":","<:","=>","<>"
);
@opercodes=(
	"PLUS","MINUS","MUL","DIV","LT","GT","LEQ","GEQ",
	"NEQ","LBRA","LPAR","LSPAR","EQ","RBRA","RPAR","RSPAR",
	"SEMI","PIPE","UP","DOT","DOTDOT","ASSIGN","COMMA","AND",
	"COL","SUB","IMP","NEQ"
);
$operpref="KEY_";
$i=0;
for $o (@opercodes) {
	print "\%token <key> ".$operpref.$o." \"".@operkeys[$i++]."\"\n";
}
$i=0;
for $o (@opercodes) {
	print "\"".@operkeys[$i++]."\"\t\t\t{\tretkey(".$operpref.$o."); }\n";
}
$i=0;
for $o (@opercodes) {
	print "\t\tcase ".$operpref.$o." : return \"".@operkeys[$i++]."\" ;\n";
}
	
@reserved=("ABS","ADDRESS","ADR","ADRSIZE","BITSIZE","BOOLEAN",
	"BYTESIZE","CARDINAL","CEILING","CHAR","DEC","DISPOSE",
	"EXTENDED","FALSE","FIRST","FLOAT","FLOOR","INC",
	"INTEGER","ISTYPE","LAST","LONGREAL","LOOPHOLE","MAX",
	"MIN","MUTEX","NARROW","NIL","NULL",
	"NUMBER","ORD","REAL","REFANY","ROUND","SUBARRAY",
	"TEXT","TRUE","TRUNC","TYPECODE","VAL"
);
$i=0;
for $r (@reserved) {
	print "#define LIT_$r ".$i++."\n";
}

