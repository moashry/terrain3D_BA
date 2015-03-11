#pragma once

#include <string.h>
#include <sstream>
#include <iostream>
#include <list>

#include "baseRecognizer.h"
#include "strprintf.h"

using str::Recognizer;
using str::TextFragment;
using str::TextArtifact;
using str::LineFileMapper;

static bool isalpha( char c ) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static bool isdigit( char c ) {
	return '0' <= c && c <= '9';
}

struct IntegerLiteral : Recognizer {
	int value;

	void parse() {
		while( !isEOF() ) {
			char c = peek();
			if( isdigit( c ) ) {
				next();
			}
			else {
				break;
			}
		}

		str::TextFragment fragment = getFragment();
		if( fragment.empty() ) {
			error( "Expected integer literal!");
		}

		std::stringstream stream( fragment.getText() );
		stream >> value;
	}
};

struct Identifier : Recognizer {
	std::string name;

	void parse() {
		char c = peek();
		if( !isalpha( c ) && c != '_' ) {
			error( "Expected identifier!" );
		}

		while( !isEOF() ) {
			c = peek();
			if( isalpha( c ) || c == '_' || isdigit( c ) ) {
				next();
			}
			else {
				break;
			}
		}

		name = getFragment().getText();
	}
};

struct CodeBlock : Recognizer {
	// TODO: move this helper function somewhere else.. [6/12/2010 Andreas Kirsch]
private:
	static std::string replaceAll( const std::string input, const std::string &search, const std::string &replace ) {
		std::string output = input;
		size_t index = 0;

		while( (index = output.find( search, index )) != std::string::npos ) {
			output.replace( index, search.length(), replace );
			index += replace.length();
		}

		return output;
	}
public:
	TextArtifact code;
	TextFragment fragment;

	void parse() {
		expectLiteral( "{" );
		setStartMarker();

		int braceLevel = 1;
		while( true ) {
			setEndMarker();

			skip();
			char c = next();
			if( c == '{' ) {
				braceLevel++;
			}
			else if( c == '}' && !--braceLevel ) {
				break;
			}
		}
	
		code = fragment = getFragment();

		LineFileMapper::LineFile lineFile = fragment.getBegin().getMappedLineFile();

		std::ostringstream codeStream;
		// two-line offset for "#version" and "#define VERTEX/GEOMETRY/FRAGMENT_SHADER" (i think) [18.6.2010 Marc Treib]
		codeStream << "\n#line " << lineFile.line - 2 << " \"" << replaceAll( lineFile.filename, "\\", "\\\\" ) << "\"\n" << fragment.getText();
		code.text = codeStream.str();
	}
};

struct QuotedString : Recognizer {
	TextFragment content;

	void parse() {
		expectLiteral( "\"" );
		setStartMarker();

		while( true ) {
			char c = next();

			setEndMarker();

			if( matchLiteral( "\\\"" ) ) {

			} else if( matchLiteral( "\"" ) ) {
				break;
			}
		}

		content = getFragment();
	}
};

namespace HiddenChannel {
	struct SlashComment : Recognizer {
		void parse() {
			if( matchLiteral( "//" ) ) {
				while( next() != '\n' )
					;
			}
			else {
				error( "Expected // comment!");
			}
		}
	};/*asd*/

	struct BlockComment : Recognizer {
		void parse() {
			if( matchLiteral( "/*" ) ) {
				while( !matchLiteral( "*/" ) )
					next();
			}
			else {
				error( "Expected /* comment!");
			}
		}
	};

	struct Whitespace : Recognizer {
		void parse() {
			while( !isEOF() ) {
				char c = peek();
				if( c != ' ' && c != '\t' && c != '\n' ) {
					break;
				}
				next();
			}

			if( getFragment().empty() ) {
				error( "Expected whitespace!" );
			}
		}
	};

	struct SharpLine : Recognizer {
		void parse() {
			Whitespace whitespaceRecognizer;
			setSkipRule( whitespaceRecognizer );

			if( matchLiteral( "#line" ) ) {
				str::TextPosition lineStart = getTextPosition();

				int line = expect<IntegerLiteral>().value;
				QuotedString filename;
				if( !match( filename ) ) {
					lineStart.addLineFileMapping( line, getTextPosition().getSourceName() );
				}
				else {
					lineStart.addLineFileMapping( line, filename.content.getText() );
				}
			}
			else {
				error( "Expected #line ...!" );
			}
		}
	};
}

struct CSkipRule : Recognizer {
	void parse() {
		using namespace HiddenChannel;

		while( 
			ignore<Whitespace>()	||
			ignore<SlashComment>()	||
			ignore<BlockComment>()	||
			ignore<SharpLine>()
			)
			;
	}
};

struct Uniform : Recognizer {
	std::string type, name, remainder;

	void parse() {
		type = expect<Identifier>().name;
		name = expect<Identifier>().name;
		remainder = readUpToLiteral( ";" );
		expectLiteral( ";" );
	}

	bool isSampler() const {
		return type.find( "sampler" ) != std::string::npos;
	}
};

struct UniformDeclarations : Recognizer {
	std::list< Uniform > uniforms;

	void parse() {
		while( !isEOF() ) {
			uniforms.push_back( expect<Uniform>() );
		}
	}
};

struct SharedCode : Recognizer {
	enum Type {
		SCT_COMMON,
		SCT_VERTEX,
		SCT_FRAGMENT,
		SCT_GEOMETRY,
		SCT_UNIFORM_BLOCK
	};
	Type type;
	TextArtifact code;
	UniformDeclarations uniformDecls;

	typedef bool (SharedCode::*TypeFunctor)() const;

	bool isUniformBlock() const {
		return type == SCT_COMMON;
	}

	bool isForAllShaders() const {
		return type == SCT_COMMON || type == SCT_UNIFORM_BLOCK;
	}

	bool isForVertexShaders() const {
		return isForAllShaders() || type == SCT_VERTEX;
	}

	bool isForFragmentShaders() const {
		return isForAllShaders() || type == SCT_FRAGMENT;
	}

	bool isForGeometryShaders() const {
		return isForAllShaders() || type == SCT_GEOMETRY;
	}

	void parse() {
		if( matchLiteral( "shared" ) ) {
			type = SCT_COMMON;
		}
		else if( matchLiteral( "vertex" ) ) {
			type = SCT_VERTEX;
		}
		else if( matchLiteral( "fragment" ) ) {
			type = SCT_FRAGMENT;
		}
		else if( matchLiteral( "geometry" ) ) {
			type = SCT_GEOMETRY;
		}
		else if( matchLiteral( "uniform" ) ) {
			type = SCT_UNIFORM_BLOCK;
		}
		else {
			error( "Bad type in shared code block!" );
		}

		CodeBlock codeBlock = expect<CodeBlock>();
		code = codeBlock.code;

		if( type == SCT_UNIFORM_BLOCK ) {
			uniformDecls.recognize<CSkipRule>( codeBlock.fragment );

			// TODO: fix this hack somehow.. maybe generate the relevant code in output.cpp? [6/12/2010 Andreas Kirsch]
			std::ostringstream uniformBlock;
			for( std::list< Uniform >::const_iterator iter = uniformDecls.uniforms.begin() ; iter != uniformDecls.uniforms.end() ; iter++ ) {
				uniformBlock << "uniform " << iter->type << " " << iter->name << " " << iter->remainder << ";\n";
			}
			code.text = uniformBlock.str();
		}

	}
};



struct InputBinding : Recognizer {
	std::string name, type;
	int numComponents;

	bool isIntegerType() const {
		return type.find( "INT" ) != std::string::npos;
	}

	void parse() {
		name = expect<Identifier>().name;
		expectLiteral(":");
		type = expect<Identifier>().name;

		if( matchLiteral( "[" ) ) {
			numComponents = expect<IntegerLiteral>().value;
			expectLiteral( "]" );
		}
		else {
			numComponents = 1;
		}
	}
};

struct InputBindings : Recognizer {
	std::list< InputBinding > bindings;

	void parse() {
		expectLiteral( "inputlayout" );
		expectLiteral( "{" );

		while( true ) {
			InputBinding inputBinding;
			if( match( inputBinding ) ) {
				bindings.push_back( inputBinding );
			}
			if( matchLiteral( "}" ) ) {
				break;
			}
			expectLiteral( "," );
		}
	}
};

struct Pass : Recognizer {
	std::list< SharedCode > sharedCodes;
	std::list< std::string > transformFeedbackVaryings;
	std::list< std::string > fragDataNames;
	std::list< TextArtifact > stateBlock;
	bool hasVertexShader, hasFragmentShader, hasGeometryShader;

	void parse() {
		hasVertexShader = hasGeometryShader = hasFragmentShader = false;

		expectLiteral( "pass" );
		expectLiteral( "{" );

		while( !matchLiteral( "}" ) ) {
			SharedCode sharedCode;
			if( match( sharedCode ) ) {
				switch( sharedCode.type ) {
					case SharedCode::SCT_VERTEX:
						hasVertexShader = true;
						break;
					case SharedCode::SCT_FRAGMENT:
						hasFragmentShader = true;
						break;
					case SharedCode::SCT_GEOMETRY:
						hasGeometryShader = true;
						break;
				}

				sharedCodes.push_back( sharedCode );
			}
			else if( transformFeedbackVaryings.empty() && matchLiteral( "feedback" ) ) {
				expectLiteral( "{" );
				while( true ) {
					if( check<Identifier>() ) {
						transformFeedbackVaryings.push_back( expect<Identifier>().name );
					}
					if( matchLiteral( "}" ) ) {
						break;
					}
					expectLiteral( "," );
				}
			}
			else if( fragDataNames.empty() && matchLiteral( "fragDataNames" ) ) {
				expectLiteral( "{" );
				while( true ) {
					if( check<Identifier>() ) {
						fragDataNames.push_back( expect<Identifier>().name );
					}
					if( matchLiteral( "}" ) ) {
						break;
					}
					expectLiteral( "," );
				}
			}
			else if( fragDataNames.empty() && matchLiteral( "fragDataName" ) ) {
				fragDataNames.push_back( expect<Identifier>().name );
			}
			else if( matchLiteral( "state" ) ) {
				stateBlock.push_back( expect<CodeBlock>().code );
			}
			else if( matchLiteral( ";" ) ) {
				// eat ;
			}
			else {
				error( "Unknown content in pass block!" );
			}
		}
	}
};

struct EffectFile : Recognizer {
	int version;
	std::list< SharedCode > sharedCodes;
	std::list< Uniform > uniforms;
	std::list< InputBinding > inputBindings;
	std::list< Pass > passes;

	static const int VERSION_NOT_SPECIFIED = -1;

	EffectFile() : version( VERSION_NOT_SPECIFIED ) {}

	void parse() {
		while( !isEOF() ) {
			SharedCode sharedCode;
			InputBindings bindings;
			Pass pass;

			if( matchLiteral( "version" ) ) {
				version = expect<IntegerLiteral>().value;
			}
			else if( match( sharedCode ) ) {
				if( sharedCode.type == SharedCode::SCT_UNIFORM_BLOCK ) {
					uniforms.insert( uniforms.end(), sharedCode.uniformDecls.uniforms.begin(), sharedCode.uniformDecls.uniforms.end() );
				}

				sharedCodes.push_back( sharedCode );
			}
			else if( inputBindings.empty() && match( bindings ) ) {
				inputBindings = bindings.bindings;
			}
			else if( match( pass ) ) {
				passes.push_back( pass );
			}
			else {
				error( "Unknown content!" );
			}
		}
	}
};