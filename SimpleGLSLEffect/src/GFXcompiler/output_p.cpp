#line 1 "c:\\Code\\kirschan\\SimpleGLSLEffect\\src\\GFXcompiler\\output.cpp"
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
		return "#version " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->version )))->str() + "\n"
"";
	}
	return "";	
}

std::string initUniformLocations( const Uniform &uniform, int uniformIndex ) {
	std::ostringstream output;
	
	for( size_t i = 0 ; i < effectFile->passes.size() ; i++ ) {
		output <<
"uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( uniformIndex )))->str() + "][ " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " ] = glGetUniformLocation( programs[ " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " ], \"" + ((std::ostringstream*)&(std::ostringstream() << ( uniform.name )))->str() + "\" );\n"
"";
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
"\n"
"for( int i = 0 ; i < " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " ; i++ ) {\n"
"\tGLint location = uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( uniformIndex )))->str() + " ][ i ];\n"
"\tif( location != -1 ) {\n"
"\t\tGLuint program = programs[ i ];\n"
"\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( setCallback( uniform, "i", uniformIndex ) )))->str() + ";\n"
"\t}\n"
"}\n"
"";
}

std::string textureUnitSet( const std::string & programIndex, UniformSetCallback setCallback ) {
	std::ostringstream output;
	int i = 0;
	for( std::list< Uniform >::const_iterator uniform = effectFile->uniforms.begin() ; uniform != effectFile->uniforms.end() ; uniform++, i++ ) {
		if( uniform->isSampler() ) {
			output <<
"\n"
"\t\tif( uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " ][ " + ((std::ostringstream*)&(std::ostringstream() << ( programIndex )))->str() + " ] != -1 ) {\n"
"\t\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( setCallback( *uniform, programIndex, "" + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + "" ) )))->str() + "; texUnit++;\n"
"\t\t}\n"
"";
		}
	}
	return output.str();
}

std::string setTexUnit( const Uniform &uniform, const std::string & programIndex, const std::string & uniformIndex ) {
	return "glUniform1i( uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( uniformIndex )))->str() + " ][ " + ((std::ostringstream*)&(std::ostringstream() << ( programIndex )))->str() + " ], texUnit )";
}
	
std::string setupPassSampler( const Uniform &uniform, const std::string & programIndex, const std::string & uniformIndex ) {
	return "" + ((std::ostringstream*)&(std::ostringstream() << ( uniformName( uniform ))))->str() + ".SetupPass( " + ((std::ostringstream*)&(std::ostringstream() << (programIndex)))->str() + ", texUnit )";	
}

std::string headerFile() {
	std::ostringstream output;

	output << 
"\n"
"#ifndef __gfx_effect_header_" + ((std::ostringstream*)&(std::ostringstream() << (name)))->str() + "\n"
"#define __gfx_effect_header_" + ((std::ostringstream*)&(std::ostringstream() << (name)))->str() + "\n"
"\n"
"#include <assert.h>\n"
"\n"
"#include \"renderer/effect.h\"\n"
"#include \"renderer/uniform.h\"\n"
"\n"
"class " + ((std::ostringstream*)&(std::ostringstream() << (name)))->str() + " : public Renderer::Effect {\n"
"public:\n"
"\tunsigned GetNumPasses() const {\n"
"\t\treturn numPasses;\n"
"\t}\n"
"\t\n"
"\t// aka input layout\n"
"\tvoid SetupBinding(unsigned stride, unsigned offset) const;\n"
"\t\n"
"\tvoid SetupPass(unsigned pass) const;\n"
"\t\n"
"\tbool Create( const char *customCode = NULL );\n"
"\t\n"
"\tvoid SafeRelease();\n"
"\t\n"
"\tvirtual ~" + ((std::ostringstream*)&(std::ostringstream() << (name)))->str() + "();\n"
"\t\n"
"\tunsigned GetProgramHandle(unsigned pass) {\n"
"\t\tassert( pass < " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " );\n"
"\t\treturn programs[ pass ];\n"
"\t}\n"
"\t\n"
"public: \n"
"\t// uniform variables\n"
"";

	forEachInList( Uniform, uniform, effectFile->uniforms ) {
		output <<
"Renderer::UniformWrapper_" + ((std::ostringstream*)&(std::ostringstream() << ( uniform->type )))->str() + " " + ((std::ostringstream*)&(std::ostringstream() << ( uniformName( *uniform ) )))->str() + ";\n"
"";
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
				output << effectFile->passes.size() << ", programs, uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( i0 )))->str() + " ]";
			}
			output << " )";
		}
	}
	output << "{}\n";
	
	output <<
"\n"
"protected:\n"
"\tstatic const unsigned numPasses = " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + ";\n"
"\t\n"
"\tGLuint programs[ " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " ];\n"
"\t" + ((std::ostringstream*)&(std::ostringstream() << ( _if( !effectFile->uniforms.empty(), "GLint uniformLocations[ " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->uniforms.size() )))->str() + " ][ " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " ];", "" ) )))->str() + "\n"
"};\n"
"\n"
"#endif\n"
""
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
"\n"
"Shared Code Blocks:\n"
"\n"
""; 
	
	int i;
	forEachInListWithIndex( SharedCode, sharedCode, i, effectFile->sharedCodes ) {
		output << "** Block " + ((std::ostringstream*)&(std::ostringstream() << ( i + 1 )))->str() + " - " + ((std::ostringstream*)&(std::ostringstream() << ( sharedCodeTypeComment( *sharedCode ) )))->str() + " **:" << std::endl
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
		output << "Pass " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + ":" << "\n\n";

		if( pass->hasVertexShader ) {
			output << "** Vertex Shader Code for Pass " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " **" << "\n\n"
				<< formatForCommentDump( listShaderCode( *pass, &SharedCode::isForVertexShaders ) ) << "\n\n";
		}
		if( pass->hasGeometryShader ) {
			output << "** Geometry Shader Code for Pass " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " **" << "\n\n"
				<< formatForCommentDump( listShaderCode( *pass, &SharedCode::isForGeometryShaders ) ) << "\n\n";
		}
		if( pass->hasFragmentShader ) {
			output << "** Fragment Shader Code for Pass " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " **" << "\n\n"
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
"\n"
"#include <stdio.h>\n"
"#include <GL/glew.h>\n"
"#include \"" + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + ".h\"\n"
"\n"
"using namespace Renderer;\n"
"\n"
"static const char *codeForPasses[][3] = {\n"
"";

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
"\n"
"};\n"
"\n"
"\n"
"void Renderer::checkGLError();\n"
"\n"
"\n"
"void " + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "::SetupPass( unsigned pass ) const {\n"
"\tif( pass >= " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " ) {\n"
"\t\t// error?\n"
"\t\treturn;\n"
"\t}\n"
"\tglUseProgram( programs[ pass ] );\n"
"\tcheckGLError();\n"
"\n"
"\tswitch( pass ) {\n"
"";
	
	forEachInListWithIndex( Pass, pass, i, effectFile->passes ) {
		output <<
"\n"
"\t\tcase " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + ": {\n"
"\n"
"\t\t\t// set the texture bindings\n"
"\t\t\tGLuint texUnit = 0;\n"
"\t\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( textureUnitSet( "" + ((std::ostringstream*)&(std::ostringstream() << (i)))->str() + "", setupPassSampler ) )))->str() + "\n"
"\n"
"\t\t\t// disable unused texture units\n"
"\t\t\tfor( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {\n"
"\t\t\t\tSampler::DisableTextureUnit( texUnit );\n"
"\t\t\t}\n"
"\n"
"\t\t\tcheckGLError();\n"
"\n"
"";
		forEachInList( TextArtifact, stateBlock, pass->stateBlock ) {
			output <<
"\n"
"\t\t\t{\n"
"\t\t\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( stateBlock->getText() )))->str() + "\n"
"\t\t\t}\n"
"";
		}

		output <<
"\n"
"\t\t\treturn;\n"
"\t\t}\n"
"";
	}

	output <<
"\n"
"\t}\n"
"}\n"
"\n"
"void " + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "::SetupBinding( unsigned stride, unsigned offset ) const {\n"
"\tunsigned totalSize = stride;\n"
"\tif( !totalSize ) {\n"
"\t\ttotalSize = 0";

	forEachInList( InputBinding, binding, effectFile->inputBindings ) {
		output << " + " << binding->numComponents << " * " << typeToSizeMap[ binding->type ];
	}
	output <<		 ";\n"
"\t}\n"
"\n"
"";
	
	forEachInListWithIndex( InputBinding, binding, i, effectFile->inputBindings ) {
		if( !binding->isIntegerType() ) {
			output <<
"\t\tglVertexAttribPointer( " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + ", " + ((std::ostringstream*)&(std::ostringstream() << ( binding->numComponents )))->str() + ", GL_" + ((std::ostringstream*)&(std::ostringstream() << ( binding->type )))->str() + ", false, totalSize, (const GLvoid*) offset );\n"
"";
		}
		else {
			output <<
"\t\tglVertexAttribIPointer( " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + ", " + ((std::ostringstream*)&(std::ostringstream() << ( binding->numComponents )))->str() + ", GL_" + ((std::ostringstream*)&(std::ostringstream() << ( binding->type )))->str() + ", totalSize, (const GLvoid*) offset );\n"
"";
		}
		output <<
"\t\tglEnableVertexAttribArray( " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + " );\n"
"\t\toffset += " + ((std::ostringstream*)&(std::ostringstream() << ( typeToSizeMap[ binding->type ] )))->str() + " * " + ((std::ostringstream*)&(std::ostringstream() << ( binding->numComponents )))->str() + ";\n"
"\n"
"";
	}

	output <<
"\n"
"\t// disable unused vertex arrays\n"
"\tfor( unsigned binding = " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->inputBindings.size() )))->str() + " ; binding < MAX_NUM_BINDINGS ; binding++ ) {\n"
"\t\tglDisableVertexAttribArray( binding );\n"
"\t}\n"
"\n"
"\tcheckGLError();\n"
"}\n"
"\n"
"static const char *PrintableNameForShaderType( GLenum shaderType ) {\n"
"\tswitch( shaderType ) {\n"
"\tcase GL_VERTEX_SHADER:\n"
"\t\treturn \"Vertex Shader\";\n"
"\tcase GL_GEOMETRY_SHADER:\n"
"\t\treturn \"Geometry Shader\";\n"
"\tcase GL_FRAGMENT_SHADER:\n"
"\t\treturn \"Fragment Shader\";\n"
"\t}\n"
"\treturn \"Unknown Shader Type\";\n"
"}\n"
"\n"
"static bool CompileLog( unsigned pass, GLenum shaderType, GLuint shader ) {\n"
"\tGLint infoSize;\n"
"\tglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoSize);\n"
"\n"
"\tchar *message = new char[infoSize];\n"
"\n"
"\tGLsizei length;\n"
"\tglGetShaderInfoLog(shader, infoSize, &length, message);\n"
"\n"
"\tif( length > 0 ) {\n"
"\t\tprintf( \"Pass %i - %s Log:\\n%s\\n\", pass, PrintableNameForShaderType( shaderType ), message );\n"
"\t}\n"
"\n"
"\tdelete[] message;\n"
"\n"
"\tGLint compileStatus;\n"
"\tglGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);\n"
"\tcheckGLError();\n"
"\n"
"\treturn compileStatus == GL_TRUE;\n"
"}\n"
"\n"
"static bool LinkLog( unsigned pass, GLuint program ) {\n"
"\tGLint infoSize;\n"
"\tglGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoSize);\n"
"\n"
"\tchar *message = new char[infoSize];\n"
"\n"
"\tGLsizei length;\n"
"\tglGetProgramInfoLog(program, infoSize, &length, message);\n"
"\n"
"\tif( length > 0 ) {\n"
"\t\tprintf( \"Pass %i - Linker Log:\\n%s\\n\", pass, message );\n"
"\t}\n"
"\n"
"\tdelete[] message;\n"
"\n"
"\tGLint compileStatus;\n"
"\tglGetProgramiv(program, GL_LINK_STATUS, &compileStatus);\n"
"\tcheckGLError();\n"
"\n"
"\treturn compileStatus == GL_TRUE;\n"
"}\n"
"\n"
"bool " + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "::Create( const char *customCode /* = NULL */ ) {\n"
"\t// a bit of an hack, if customCode is NULL, set it to an empty string instead\n"
"\tif( !customCode ) {\n"
"\t\tcustomCode = \"\";\n"
"\t}\n"
"\n"
"\tprintf( \"Compiling technique " + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "\\n\\n\" );\n"
"\n"
"\tbool success = true;\n"
"\n"
"\tconst char *vertexShaderSources[] = { \n"
"\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( formatAsCString( versionInfo() ) )))->str() + "\n"
"\t\t\"#define VERTEX_SHADER\\r\\n\",\n"
"\t\tcustomCode,\n"
"\t\tNULL\n"
"\t};\n"
"\tconst unsigned int numVertexShaderSources = sizeof( vertexShaderSources ) / sizeof( *vertexShaderSources );\n"
"\n"
"\tconst char *fragmentShaderSources[] = { \n"
"\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( formatAsCString( versionInfo() ) )))->str() + "\n"
"\t\t\"#define FRAGMENT_SHADER\\r\\n\",\n"
"\t\tcustomCode,\n"
"\t\tNULL\n"
"\t};\n"
"\tconst unsigned int numFragmentShaderSources = sizeof( fragmentShaderSources ) / sizeof( *fragmentShaderSources );\n"
"\n"
"\tconst char *geometryShaderSources[] = { \n"
"\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( formatAsCString( versionInfo() ) )))->str() + "\n"
"\t\t\"#define GEOMETRY_SHADER\\r\\n\",\n"
"\t\tcustomCode,\n"
"\t\tNULL\n"
"\t};\n"
"\tconst unsigned int numGeometryShaderSources = sizeof( geometryShaderSources ) / sizeof( *geometryShaderSources );\n"
"\n"
"\tGLuint program;\n"
"";
	
	int passIndex;
	forEachInListWithIndex( Pass, pass, passIndex, effectFile->passes ) {
		output <<
"\n"
"\t// initialize the program for pass " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + "\n"
"\tprogram = glCreateProgram();\n"
"\tcheckGLError();\n"
"\n"
"\t" + ((std::ostringstream*)&(std::ostringstream() << ( _if( pass->hasVertexShader, "{\n"
"\t\tGLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );\n"
"\t\tvertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[" + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + "][0];\n"
"\t\tglShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );\n"
"\t\tglCompileShader( vertexShader );\n"
"\t\tcheckGLError();\n"
"\n"
"\t\tsuccess = CompileLog( " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + ", GL_VERTEX_SHADER, vertexShader ) && success;\n"
"\n"
"\t\tglAttachShader( program, vertexShader );\n"
"\t\tglDeleteShader( vertexShader );\n"
"\t\tcheckGLError();\n"
"\t}", "") )))->str() + "\n"
"\t" + ((std::ostringstream*)&(std::ostringstream() << ( _if( pass->hasFragmentShader, "{\n"
"\t\tGLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );\n"
"\t\tfragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[" + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + "][1];\n"
"\t\tglShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );\n"
"\t\tglCompileShader( fragmentShader );\n"
"\t\tcheckGLError();\n"
"\n"
"\t\tsuccess = CompileLog( " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + ", GL_FRAGMENT_SHADER, fragmentShader ) && success;\n"
"\n"
"\t\tglAttachShader( program, fragmentShader );\n"
"\t\tglDeleteShader( fragmentShader );\n"
"\t\tcheckGLError();\n"
"\t}", "") )))->str() + "\n"
"\t" + ((std::ostringstream*)&(std::ostringstream() << ( _if( pass->hasGeometryShader, "{\n"
"\t\tGLuint geometryShader = glCreateShader( GL_GEOMETRY_SHADER_ARB );\n"
"\t\tgeometryShaderSources[ numGeometryShaderSources - 1 ] = codeForPasses[ " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + "][2];\n"
"\t\tglShaderSource( geometryShader, numGeometryShaderSources, geometryShaderSources, NULL );\n"
"\t\tglCompileShader( geometryShader );\n"
"\t\tcheckGLError();\n"
"\n"
"\t\tsuccess = CompileLog( " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + ", GL_GEOMETRY_SHADER_ARB, geometryShader ) && success;\n"
"\n"
"\t\tglAttachShader( program, geometryShader );\n"
"\t\tglDeleteShader( geometryShader );\n"
"\t\tcheckGLError();\n"
"\t}", "") )))->str() + "\n"
"\n"
"\t// link the attributes\n"
"";
		forEachInListWithIndex( InputBinding, binding, i, effectFile->inputBindings ) {
			output <<
"\tglBindAttribLocation( program, " + ((std::ostringstream*)&(std::ostringstream() << ( i )))->str() + ", " + ((std::ostringstream*)&(std::ostringstream() << ( formatAsCString( binding->name ) )))->str() + " );\n"
"\tcheckGLError();\n"
"";
		}

		if( pass->transformFeedbackVaryings.size() > 0 ) {
			output << "\n"
"\t// link transform feedback varyings\n"
"\tconst char *varyings[] = { ";
			first = true;
			forEachInList( std::string, varying, pass->transformFeedbackVaryings ) {
				if( !first ) {
					output << ", ";
				}
				first = false;

				output << formatAsCString( *varying );
			}

			output << " };\n"
"\tconst unsigned numVaryings = sizeof( varyings ) / sizeof( *varyings );\n"
"\tglTransformFeedbackVaryings( program, numVaryings, varyings, GL_INTERLEAVED_ATTRIBS );\n"
"\tcheckGLError();\n"
"";
		}
		
		if( !pass->fragDataNames.empty() ) {
			output << "\n"
"\t// link the frag data name\n"
"";

			int i;
			forEachInListWithIndex( std::string, fragDataName, i, pass->fragDataNames ) {
			output << 
"\tglBindFragDataLocation( program, " + ((std::ostringstream*)&(std::ostringstream() << (i )))->str() + ", " + ((std::ostringstream*)&(std::ostringstream() << ( formatAsCString( *fragDataName ) )))->str() + " );\n"
"";
			}

			output << std::endl;
		}

		output <<
"\tglLinkProgram( program );\n"
"\tcheckGLError();\n"
"\tsuccess = LinkLog( " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + ", program ) && success;\n"
"\n"
"\tprograms[ " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + " ] = program;\n"
"";
	}

	output <<
"\n"
"\t// init uniform locations\n"
"";
	forEachInListWithIndex( Uniform, uniform, i, effectFile->uniforms ) {
		output << "\n"
"\t" + ((std::ostringstream*)&(std::ostringstream() << ( initUniformLocations( *uniform, i ) )))->str() + "\n"
"";
	}

	output << "\n"
"\tcheckGLError();\n"
"\n"
"\t// init the samplers\n"
"";
	forEachInListWithIndex( Pass, pass, passIndex, effectFile->passes ) {
		output <<
"\t{\n"
"\t\tglUseProgram( programs[ " + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + " ] );\n"
"\t\tGLuint texUnit = 0;\n"
"\t\t" + ((std::ostringstream*)&(std::ostringstream() << ( textureUnitSet( "" + ((std::ostringstream*)&(std::ostringstream() << ( passIndex )))->str() + "", setTexUnit ) )))->str() + "\n"
"\t}\n"
"";
	}

	output <<
"\n"
"\n"
"\tglUseProgram( 0 );\n"
"\tcheckGLError();\n"
"\n"
"\treturn success;\n"
"}\n"
"\n"
"" + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "::~" + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "() {\n"
"\tSafeRelease();\n"
"}\n"
"\n"
"void " + ((std::ostringstream*)&(std::ostringstream() << ( name )))->str() + "::SafeRelease() {\n"
"\tfor( int i = 0 ; i < " + ((std::ostringstream*)&(std::ostringstream() << ( effectFile->passes.size() )))->str() + " ; i++ ) {\n"
"\t\tglDeleteProgram( programs[ i ] );\n"
"\t\tprograms[ i ] = 0;\n"
"\t}\n"
"\tcheckGLError();\n"
"}\n"
"";

	return output.str();
}