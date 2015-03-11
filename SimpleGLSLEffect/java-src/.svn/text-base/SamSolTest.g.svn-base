grammar SamSolTest;
// original idea/hint from Sam Soldatenko sam[at]soldatenko.ru

pass: 'pass' '{' (state | vertex)* '}' ;
state: 'state' '{' code '}';
vertex: 'vertex' '{' code '}';

code
	: (~( '{' | '}') | '{' code '}' )*
	;
	
WS : ( '\t' | ' ' | '\r'  |'\n'| '\u000C' )+ { $channel = HIDDEN; } ;

NOTBRACES
	: ~( '{' | '}')
	;
