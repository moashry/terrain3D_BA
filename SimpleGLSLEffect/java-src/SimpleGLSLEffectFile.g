grammar SimpleGLSLEffectFile;
options {
	language = Java;
	backtrack = true;
}

effectFile returns [Integer version, List<SharedCode> sharedCodes, List<Pass> passes, List<Uniform> uniforms, List<Binding> bindings]
@init {
	$passes = new ArrayList<Pass>();
	$uniforms = new ArrayList<Uniform>();
	$sharedCodes = new ArrayList<SharedCode>();
}
	:	('version' versionNumber=NUMBER { $version = new Integer( $versionNumber.text ); })?
		(
			'shared' commonCode=codeBlock { $sharedCodes.add( new SharedCode( SharedCode.Type.COMMON, $commonCode.innerCode ) ); }
		|	'vertex' vertexCode=codeBlock { $sharedCodes.add( new SharedCode( SharedCode.Type.VERTEX, $vertexCode.innerCode ) ); }
		|	'fragment' fragmentCode=codeBlock { $sharedCodes.add( new SharedCode( SharedCode.Type.FRAGMENT, $fragmentCode.innerCode ) ); }
		|	'geometry' geometryCode=codeBlock { $sharedCodes.add( new SharedCode( SharedCode.Type.GEOMETRY, $geometryCode.innerCode ) ); }
		|	uniformsDeclaration { 
				$uniforms.addAll( $uniformsDeclaration.uniforms );
				$sharedCodes.add( new SharedCode( SharedCode.Type.UNIFORM_BLOCK, $uniformsDeclaration.uniforms ) ); 
			}
		|	{$bindings == null}? inputLayoutDeclaration { $bindings = $inputLayoutDeclaration.bindings; }
		)* 
		(passDefinition
			{
				$passes.add( new Pass( $passDefinition.vertexShader,
									$passDefinition.geometryShader,
									$passDefinition.fragmentShader,
									$passDefinition.transformFeedbackVaryings,
									$passDefinition.stateBlock,
									$passDefinition.fragDataName ) );
			}
		)+ 
	;
	
uniformsDeclaration returns [ List<Uniform> uniforms ]
@init {
	$uniforms = new ArrayList<Uniform>();
}
	: 'uniform' '{'
		( uniformDeclaration { $uniforms.add( $uniformDeclaration.uniform ); } )*
		'}'
	;
	
inputLayoutDeclaration returns [List<Binding> bindings]
@init {
	$bindings = new ArrayList<Binding>();
}
	: 	'inputlayout' '{'
			(inputDeclaration[$bindings] (',' inputDeclaration[$bindings])*)?
		'}'
	;

inputDeclaration[List<Binding> bindings]
@init {
	int numComponents = 1;
}
	:	name=ID ':' type=ID ('[' size=NUMBER { numComponents = new Integer( $size.text ); } ']')? { $bindings.add( new Binding( $name.text, numComponents, $type.text ) ); }
	;

uniformDeclaration returns [Uniform uniform]
	: type=ID name=ID ';' { $uniform = new Uniform( $type.text, $name.text ); }
	;
	
passDefinition returns [String vertexShader, Pass.GeometryShader geometryShader, String fragmentShader, List<String> transformFeedbackVaryings,  Pass.State stateBlock, String fragDataName]
@init {
	$stateBlock = new Pass.State();
}	:	'pass' '{'
			({$vertexShader == null}?	'vertex' vertexShaderCode=codeBlock		{ $vertexShader = $vertexShaderCode.innerCode; }
			|{$fragmentShader == null}?	'fragment' fragmentShaderCode=codeBlock	{ $fragmentShader = $fragmentShaderCode.innerCode; }
			|{$geometryShader == null}?	'geometry' '(' inputType=ID ',' outputType=ID ',' maxEmitVertices=parameter ')' geometryShaderCode=codeBlock	
				{ $geometryShader = new Pass.GeometryShader( $geometryShaderCode.innerCode, $inputType.text, $outputType.text, $maxEmitVertices.text ); }
			// TODO: really add scope support to make this kind of thing easier!
			|{$transformFeedbackVaryings == null}? 'feedback' 
				{ $transformFeedbackVaryings = new ArrayList<String>(); }
				'{' 
					( transformFeedbackVarying[ $transformFeedbackVaryings ] (',' transformFeedbackVarying[ $transformFeedbackVaryings ])* )?
				'}'
			| {$fragDataName == null}? 'fragDataName' fragDataNameID=ID { $fragDataName = $fragDataNameID.text; }
			| stateDefinition[$stateBlock]
			| ';' /* eat ;s */)* '}'
	;

transformFeedbackVarying[List<String> transformFeedbackVaryings]
	: ID { $transformFeedbackVaryings.add( $ID.text ); }
	;

stateDefinition[Pass.State stateBlock]
	: 'state' '{'
		( ('depthTest' ':' toggle=bool { $stateBlock.depthTest = $toggle.value; })
		| ('stencilTest' ':' toggle=bool { $stateBlock.stencilTest = $toggle.value; }) )*
	   '}'
	;

bool returns [boolean value]
	: 'true' { $value = true; } | 'false' { $value = false; }
	;
	
parameter
	: ( '(' parameter ')' | '{' | '}' | ~('(' | ')') )*
	;

/* from the book */	
codeBlock returns [String innerCode]
	: '{' code '}' { $innerCode = "#line " + ($code.start.getLine() - 1) + "\r\n" + $code.text + "\r\n"; }
	;
	
code
	:  ( simpleCode | '(' | ')' | ~('{' |'}') )*
	;
	
simpleCode
	: '{' ( simpleCode | '(' | ')' | ~('{' |'}') )* '}'
	;
	
ID 
	:	(LETTER|'_')(LETTER|'0'..'9'|'_')* ;
	
fragment
LETTER
	: 'A'..'Z'|'a'..'z';

// WS, COMMENT and LINE_COMMENT from http://www.antlr.org/grammar/1153358328744/C.g
WS  :  (' '|'\r'|'\t'|'\u000C'|'\n') {$channel=HIDDEN;}
    ;

COMMENT
    :   '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;

LINE_COMMENT
    : '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    ;
	
LINE
	: '#line' WS+ NUMBER  { getCharStream().setLine( new Integer( $NUMBER.text ) ); $channel=HIDDEN; }
	~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
	;
	
NUMBER
	:	('0'..'9')+
	;
	
// filler token (never used in rules) but needed for reading in blocks of data (anything)
ANYTHING_BUT_BRACES_OR_PARENTHESES	
	: ~( '{' | '}' | '(' | ')' )
	;
	
