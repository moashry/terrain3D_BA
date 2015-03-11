// Andreas Kirsch 2010
// needs to be preprocessed with ctp!

#include <sstream>
#include <map>

#include "gfxRecognizer.h"

//#define _if( condition, trueBlock, falseBlock )	( (condition) ? (trueBlock) : (falseBlock))
std::string _if( bool condition, const std::string &trueString, const std::string &falseString ) {
	return condition ? trueString : falseString;
}

#define forEachInList( type, variable, listVariable ) \
	for( std::list< type >::const_iterator variable = listVariable.begin() ; variable != listVariable.end() ; variable++ )
#define forEachInListWithIndex( type, variable, outerIndexVariable, listVariable ) \
	outerIndexVariable = 0; \
	for( std::list< type >::const_iterator variable = listVariable.begin() ; variable != listVariable.end() ; variable++, outerIndexVariable++ )

static const EffectFile *effectFile;
static std::string name;

void initOutputStage( const EffectFile *file, const std::string &className ) {
	void initTypeToSizeMap();

	initTypeToSizeMap();

	effectFile = file;
	name = className;
}

std::string uniformName( const Uniform &uniform ) {
	return uniform.name;
}

std::string versionInfo() {
	if( effectFile->version != EffectFile::VERSION_NOT_SPECIFIED ) {
		return <<<#version <( effectFile->version )>
>>>;
	}
	return "";	
}

std::string initUniformLocations( const Uniform &uniform, int uniformIndex ) {
	std::ostringstream output;
	
	for( size_t i = 0 ; i < effectFile->passes.size() ; i++ ) {
		output <<
<<<uniformLocations[ <( uniformIndex )>][ <( i )> ] = glGetUniformLocation( programs[ <( i )> ], "<( uniform.name )>" );
>>>;
	}
	
	return output.str();
}

static std::map<std::string, std::string> typeToSizeMap;
static const char *_typeToSizeMap[][2] = {
	{"UNSIGNED_INT", "4"},
	{"FLOAT","4"}
};

void initTypeToSizeMap() {
	for( int i = 0 ; i < sizeof(_typeToSizeMap) / sizeof(_typeToSizeMap[0]) ; i++ ) {
		typeToSizeMap[ _typeToSizeMap[i][0] ] =  _typeToSizeMap[i][1];
	}
}

typedef std::string (*UniformSetCallback)( const Uniform &uniform, const std::string & programIndex, const std::string & uniformIndex );

std::string uniformSet( const Uniform &uniform, const std::string & uniformIndex, UniformSetCallback setCallback ) {
	return 
<<<
for( int i = 0 ; i < <( effectFile->passes.size() )> ; i++ ) {
	GLint location = uniformLocations[ <( uniformIndex )> ][ i ];
	if( location != -1 ) {
		GLuint program = programs[ i ];
		<( setCallback( uniform, "i", uniformIndex ) )>;
	}
}
>>>;
}

std::string textureUnitSet( const std::string & programIndex, UniformSetCallback setCallback ) {
	std::ostringstream output;
	int i = 0;
	for( std::list< Uniform >::const_iterator uniform = effectFile->uniforms.begin() ; uniform != effectFile->uniforms.end() ; uniform++, i++ ) {
		if( uniform->isSampler() ) {
			output <<
<<<
		if( uniformLocations[ <( i )> ][ <( programIndex )> ] != -1 ) {
			<( setCallback( *uniform, programIndex, <<<<( i )>>>> ) )>; texUnit++;
		}
>>>;
		}
	}
	return output.str();
}

std::string setTexUnit( const Uniform &uniform, const std::string & programIndex, const std::string & uniformIndex ) {
	return <<<glUniform1i( uniformLocations[ <( uniformIndex )> ][ <( programIndex )> ], texUnit )>>>;
}
	
std::string setupPassSampler( const Uniform &uniform, const std::string & programIndex, const std::string & uniformIndex ) {
	return <<<<( uniformName( uniform ))>.SetupPass( <(programIndex)>, texUnit )>>>;	
}

std::string headerFile() {
	std::ostringstream output;

	output << 
<<<
#ifndef __gfx_effect_header_<(name)>
#define __gfx_effect_header_<(name)>

#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class <(name)> : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~<(name)>();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < <( effectFile->passes.size() )> );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
>>>;

	forEachInList( Uniform, uniform, effectFile->uniforms ) {
		output <<
<<<Renderer::UniformWrapper_<( uniform->type )> <( uniformName( *uniform ) )>;
>>>;
	}

	output << name << "()";
	if( !effectFile->uniforms.empty() ) {
		output << " : ";
		bool isFirst = true;
		int i0;
		forEachInListWithIndex( Uniform, uniform, i0, effectFile->uniforms ) {
			if( !isFirst ) {
				output << ", ";
			}
			else {
				isFirst = false;
			}

			output << uniformName( *uniform ) << "( ";

			if( !uniform->isSampler() ) {
				output << effectFile->passes.size() << <<<, programs, uniformLocations[ <( i0 )> ]>>>;
			}
			output << " )";
		}
	}
	output << "{}\n";
	
	output <<
<<<
protected:
	static const unsigned numPasses = <( effectFile->passes.size() )>;
	
	GLuint programs[ <( effectFile->passes.size() )> ];
	<( _if( !effectFile->uniforms.empty(), <<<GLint uniformLocations[ <( effectFile->uniforms.size() )> ][ <( effectFile->passes.size() )> ];>>>, "" ) )>
};

#endif
>>>
;

	return output.str();
}

std::string sharedCodeTypeComment( const SharedCode &sharedCode ) {
	switch( sharedCode.type ) {
		case SharedCode::SCT_COMMON:
			return "for all shaders";
		case SharedCode::SCT_FRAGMENT:
			return "for fragment shaders";
		case SharedCode::SCT_GEOMETRY:
			return "for geometry shaders";
		case SharedCode::SCT_UNIFORM_BLOCK:
			return "uniform block (uniform declarations)";
		case SharedCode::SCT_VERTEX:
			return "for vertex shaders";
		default:
			assert( false );
			return "";
	}
}

std::string listSharedCodeForType( const std::list< SharedCode > &sharedCodes, SharedCode::TypeFunctor type ) {
	std::ostringstream output;
	forEachInList( SharedCode, sharedCode, sharedCodes ) {
		if( ((*sharedCode).*type)() ) {
			output << sharedCode->code.getText();
		}
	}
	return output.str();
}

// vertexShaderCode with parameter &SharedCode::isForVertexShaders, fragmentShaderCode, geometryShaderCode etc
std::string listShaderCode( const Pass &pass, SharedCode::TypeFunctor type ) {
	return listSharedCodeForType( effectFile->sharedCodes, type )
		+ "\n"
		+ listSharedCodeForType( pass.sharedCodes, type);
}

std::string replaceAll( const std::string input, const std::string &search, const std::string &replace ) {
	std::string output = input;
	size_t index = 0;

	while( (index = output.find( search, index )) != std::string::npos ) {
		output.replace( index, search.length(), replace );
		index += replace.length();
	}

	return output;
}

std::string formatAsCString( const std::string &input ) {
	return "\"" + replaceAll( replaceAll( replaceAll( replaceAll( input, "\\", "\\\\" ), "\t", "\\t" ), "\"", "\\\"" ), "\n", "\\n\"\n\"" ) + "\"";
}

std::string formatForCodeDump( const std::string &input ) {
	std::string output = input;

	for( std::string::iterator p = output.begin() ; p != output.end() ; p++ ) {
		if( *p == '$' ) {
			*p = '#';
		}
	}

	return output;
}

std::string formatForGLSLCString( const std::string &input ) {
	return formatAsCString( formatForCodeDump( input ) );
}

std::string formatForCommentDump( const std::string &input ) {
	return replaceAll( replaceAll( input, "/*", "/ *" ), "*/", "* /" );
}

std::string codeDump( const Pass &pass, SharedCode::TypeFunctor type ) {
	return formatForCodeDump( listShaderCode( pass, type ) );
}

std::string sharedCodeBlockInfo() {
	std::ostringstream output;
	
	output <<
<<<
Shared Code Blocks:

>>>; 
	
	int i;
	forEachInListWithIndex( SharedCode, sharedCode, i, effectFile->sharedCodes ) {
		output << <<<** Block <( i + 1 )> - <( sharedCodeTypeComment( *sharedCode ) )> **:>>> << std::endl
			<< formatForCommentDump(  sharedCode->code.getText() ) << std::endl;
	}
	output << std::endl;

	return output.str();
}

std::string passesInfo() {
	std::ostringstream output;

	output << "Passes:\n\n";

	int i;
	forEachInListWithIndex( Pass, pass, i, effectFile->passes ) {
		output << <<<Pass <( i )>:>>> << "\n\n";

		if( pass->hasVertexShader ) {
			output << <<<** Vertex Shader Code for Pass <( i )> **>>> << "\n\n"
				<< formatForCommentDump( listShaderCode( *pass, &SharedCode::isForVertexShaders ) ) << "\n\n";
		}
		if( pass->hasGeometryShader ) {
			output << <<<** Geometry Shader Code for Pass <( i )> **>>> << "\n\n"
				<< formatForCommentDump( listShaderCode( *pass, &SharedCode::isForGeometryShaders ) ) << "\n\n";
		}
		if( pass->hasFragmentShader ) {
			output << <<<** Fragment Shader Code for Pass <( i )> **>>> << "\n\n"
				<< formatForCommentDump( listShaderCode( *pass, &SharedCode::isForFragmentShaders ) ) << "\n\n";
		}
		output << "\n";
	}

	return output.str();
}

std::string sourceFile() {
	std::ostringstream output;
	bool first;
	int i;

	output << "/* Shader Code Information\n\n"
		<< sharedCodeBlockInfo()
		<< passesInfo() << "*/\n"
		<<
<<<
#include <stdio.h>
#include <GL/glew.h>
#include "<( name )>.h"

using namespace Renderer;

static const char *codeForPasses[][3] = {
>>>;

	first = true;
	forEachInListWithIndex( Pass, pass, i, effectFile->passes ) {
		if( !first ) {
			output << ",\n";
		}
		first = false;

		output << "// pass " << i << std::endl
			<< "{" << std::endl;

		if( pass->hasVertexShader ) {
			output << formatForGLSLCString( listShaderCode( *pass, &SharedCode::isForVertexShaders ) );
		}
		else {
			output << "NULL";
		}
		output << "\n,\n";

		if( pass->hasFragmentShader ) {
			output << formatForGLSLCString( listShaderCode( *pass, &SharedCode::isForFragmentShaders ) );
		}
		else {
			output << "NULL";
		}
		output << "\n,\n";

		if( pass->hasGeometryShader ) {
			output << formatForGLSLCString( listShaderCode( *pass, &SharedCode::isForGeometryShaders ) );
		}	
		else {
			output << "NULL";
		}
		output << "\n}";
	}
	output <<
<<<
};


void Renderer::checkGLError();


void <( name )>::SetupPass( unsigned pass ) const {
	if( pass >= <( effectFile->passes.size() )> ) {
		// error?
		return;
	}
	glUseProgram( programs[ pass ] );
	checkGLError();

	switch( pass ) {
>>>;
	
	forEachInListWithIndex( Pass, pass, i, effectFile->passes ) {
		output <<
<<<
		case <( i )>: {

			// set the texture bindings
			GLuint texUnit = 0;
			<( textureUnitSet( <<<<(i)>>>>, setupPassSampler ) )>

			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();

>>>;
		forEachInList( TextArtifact, stateBlock, pass->stateBlock ) {
			output <<
<<<
			{
				<( stateBlock->getText() )>
			}
>>>;
		}

		output <<
<<<
			return;
		}
>>>;
	}

	output <<
<<<
	}
}

void <( name )>::SetupBinding( unsigned stride, unsigned offset ) const {
	unsigned totalSize = stride;
	if( !totalSize ) {
		totalSize = 0>>>;

	forEachInList( InputBinding, binding, effectFile->inputBindings ) {
		output << " + " << binding->numComponents << " * " << typeToSizeMap[ binding->type ];
	}
	output <<		 <<<;
	}

>>>;
	
	forEachInListWithIndex( InputBinding, binding, i, effectFile->inputBindings ) {
		if( !binding->isIntegerType() ) {
			output <<
<<<		glVertexAttribPointer( <( i )>, <( binding->numComponents )>, GL_<( binding->type )>, false, totalSize, (const GLvoid*) offset );
>>>;
		}
		else {
			output <<
<<<		glVertexAttribIPointer( <( i )>, <( binding->numComponents )>, GL_<( binding->type )>, totalSize, (const GLvoid*) offset );
>>>;
		}
		output <<
<<<		glEnableVertexAttribArray( <( i )> );
		offset += <( typeToSizeMap[ binding->type ] )> * <( binding->numComponents )>;

>>>;
	}

	output <<
<<<
	// disable unused vertex arrays
	for( unsigned binding = <( effectFile->inputBindings.size() )> ; binding < MAX_NUM_BINDINGS ; binding++ ) {
		glDisableVertexAttribArray( binding );
	}

	checkGLError();
}

static const char *PrintableNameForShaderType( GLenum shaderType ) {
	switch( shaderType ) {
	case GL_VERTEX_SHADER:
		return "Vertex Shader";
	case GL_GEOMETRY_SHADER:
		return "Geometry Shader";
	case GL_FRAGMENT_SHADER:
		return "Fragment Shader";
	}
	return "Unknown Shader Type";
}

static bool CompileLog( unsigned pass, GLenum shaderType, GLuint shader ) {
	GLint infoSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoSize);

	char *message = new char[infoSize];

	GLsizei length;
	glGetShaderInfoLog(shader, infoSize, &length, message);

	if( length > 0 ) {
		printf( "Pass %i - %s Log:\n%s\n", pass, PrintableNameForShaderType( shaderType ), message );
	}

	delete[] message;

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	checkGLError();

	return compileStatus == GL_TRUE;
}

static bool LinkLog( unsigned pass, GLuint program ) {
	GLint infoSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoSize);

	char *message = new char[infoSize];

	GLsizei length;
	glGetProgramInfoLog(program, infoSize, &length, message);

	if( length > 0 ) {
		printf( "Pass %i - Linker Log:\n%s\n", pass, message );
	}

	delete[] message;

	GLint compileStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &compileStatus);
	checkGLError();

	return compileStatus == GL_TRUE;
}

bool <( name )>::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique <( name )>\n\n" );

	bool success = true;

	const char *vertexShaderSources[] = { 
		<( formatAsCString( versionInfo() ) )>
		"#define VERTEX_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numVertexShaderSources = sizeof( vertexShaderSources ) / sizeof( *vertexShaderSources );

	const char *fragmentShaderSources[] = { 
		<( formatAsCString( versionInfo() ) )>
		"#define FRAGMENT_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numFragmentShaderSources = sizeof( fragmentShaderSources ) / sizeof( *fragmentShaderSources );

	const char *geometryShaderSources[] = { 
		<( formatAsCString( versionInfo() ) )>
		"#define GEOMETRY_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numGeometryShaderSources = sizeof( geometryShaderSources ) / sizeof( *geometryShaderSources );

	GLuint program;
>>>;
	
	int passIndex;
	forEachInListWithIndex( Pass, pass, passIndex, effectFile->passes ) {
		output <<
<<<
	// initialize the program for pass <( passIndex )>
	program = glCreateProgram();
	checkGLError();

	<( _if( pass->hasVertexShader, <<<{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[<( passIndex )>][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( <( passIndex )>, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}>>>, "") )>
	<( _if( pass->hasFragmentShader, <<<{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[<( passIndex )>][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( <( passIndex )>, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}>>>, "") )>
	<( _if( pass->hasGeometryShader, <<<{
		GLuint geometryShader = glCreateShader( GL_GEOMETRY_SHADER_ARB );
		geometryShaderSources[ numGeometryShaderSources - 1 ] = codeForPasses[ <( passIndex )>][2];
		glShaderSource( geometryShader, numGeometryShaderSources, geometryShaderSources, NULL );
		glCompileShader( geometryShader );
		checkGLError();

		success = CompileLog( <( passIndex )>, GL_GEOMETRY_SHADER_ARB, geometryShader ) && success;

		glAttachShader( program, geometryShader );
		glDeleteShader( geometryShader );
		checkGLError();
	}>>>, "") )>

	// link the attributes
>>>;
		forEachInListWithIndex( InputBinding, binding, i, effectFile->inputBindings ) {
			output <<
<<<	glBindAttribLocation( program, <( i )>, <( formatAsCString( binding->name ) )> );
	checkGLError();
>>>;
		}

		if( pass->transformFeedbackVaryings.size() > 0 ) {
			output << <<<
	// link transform feedback varyings
	const char *varyings[] = { >>>;
			first = true;
			forEachInList( std::string, varying, pass->transformFeedbackVaryings ) {
				if( !first ) {
					output << ", ";
				}
				first = false;

				output << formatAsCString( *varying );
			}

			output << <<< };
	const unsigned numVaryings = sizeof( varyings ) / sizeof( *varyings );
	glTransformFeedbackVaryings( program, numVaryings, varyings, GL_INTERLEAVED_ATTRIBS );
	checkGLError();
>>>;
		}
		
		if( !pass->fragDataNames.empty() ) {
			output << <<<
	// link the frag data name
>>>;

			int i;
			forEachInListWithIndex( std::string, fragDataName, i, pass->fragDataNames ) {
			output << 
<<<	glBindFragDataLocation( program, <(i )>, <( formatAsCString( *fragDataName ) )> );
>>>;
			}

			output << std::endl;
		}

		output <<
<<<	glLinkProgram( program );
	checkGLError();
	success = LinkLog( <( passIndex )>, program ) && success;

	programs[ <( passIndex )> ] = program;
>>>;
	}

	output <<
<<<
	// init uniform locations
>>>;
	forEachInListWithIndex( Uniform, uniform, i, effectFile->uniforms ) {
		output << <<<
	<( initUniformLocations( *uniform, i ) )>
>>>;
	}

	output << <<<
	checkGLError();

	// init the samplers
>>>;
	forEachInListWithIndex( Pass, pass, passIndex, effectFile->passes ) {
		output <<
<<<	{
		glUseProgram( programs[ <( passIndex )> ] );
		GLuint texUnit = 0;
		<( textureUnitSet( <<<<( passIndex )>>>>, setTexUnit ) )>
	}
>>>;
	}

	output <<
<<<

	glUseProgram( 0 );
	checkGLError();

	return success;
}

<( name )>::~<( name )>() {
	SafeRelease();
}

void <( name )>::SafeRelease() {
	for( int i = 0 ; i < <( effectFile->passes.size() )> ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}
>>>;

	return output.str();
}