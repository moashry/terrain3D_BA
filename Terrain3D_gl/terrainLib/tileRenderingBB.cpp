/* Shader Code Information


Shared Code Blocks:

** Block 1 - uniform block (uniform declarations) **:
uniform mat4x4 mWorldView ;
uniform mat4x4 mProjection ;
uniform vec4 vColorBB ;


Passes:

Pass 0:

** Vertex Shader Code for Pass 0 **

uniform mat4x4 mWorldView ;
uniform mat4x4 mProjection ;
uniform vec4 vColorBB ;


#line 19 "c:\\Code\\kirschan\\Terrain3D_gl\\terrainLib\\tileRenderingBB.i"

		in vec4 position;
		
		void main() {
			gl_Position = mProjection * mWorldView * position;
		}

** Fragment Shader Code for Pass 0 **

uniform mat4x4 mWorldView ;
uniform mat4x4 mProjection ;
uniform vec4 vColorBB ;


#line 26 "c:\\Code\\kirschan\\Terrain3D_gl\\terrainLib\\tileRenderingBB.i"

		out vec4 vColor;

		void main() {
			vColor = vColorBB;
		}


*/

#include <stdio.h>
#include <GL/glew.h>
#include "tileRenderingBB.h"

using namespace Renderer;

static const char *codeForPasses[][3] = {
// pass 0
{
"uniform mat4x4 mWorldView ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform vec4 vColorBB ;\n"
"\n"
"\n"
"#line 19 \"c:\\\\Code\\\\kirschan\\\\Terrain3D_gl\\\\terrainLib\\\\tileRenderingBB.i\"\n"
"\n"
"\t\tin vec4 position;\n"
"\t\t\n"
"\t\tvoid main() {\n"
"\t\t\tgl_Position = mProjection * mWorldView * position;\n"
"\t\t}"
,
"uniform mat4x4 mWorldView ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform vec4 vColorBB ;\n"
"\n"
"\n"
"#line 26 \"c:\\\\Code\\\\kirschan\\\\Terrain3D_gl\\\\terrainLib\\\\tileRenderingBB.i\"\n"
"\n"
"\t\tout vec4 vColor;\n"
"\n"
"\t\tvoid main() {\n"
"\t\t\tvColor = vColorBB;\n"
"\t\t}"
,
NULL
}
};


void Renderer::checkGLError();


void tileRenderingBB::SetupPass( unsigned pass ) const {
	if( pass >= 1 ) {
		// error?
		return;
	}
	glUseProgram( programs[ pass ] );
	checkGLError();

	switch( pass ) {

		case 0: {

			// set the texture bindings
			GLuint texUnit = 0;
			

			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			return;
		}

	}
}

void tileRenderingBB::SetupBinding( unsigned stride, unsigned offset ) const {
	unsigned totalSize = stride;
	if( !totalSize ) {
		totalSize = 0 + 3 * 4;
	}

		glVertexAttribPointer( 0, 3, GL_FLOAT, false, totalSize, (const GLvoid*) offset );
		glEnableVertexAttribArray( 0 );
		offset += 4 * 3;


	// disable unused vertex arrays
	for( unsigned binding = 1 ; binding < MAX_NUM_BINDINGS ; binding++ ) {
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

bool tileRenderingBB::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique tileRenderingBB\n\n" );

	bool success = true;

	const char *vertexShaderSources[] = { 
		"#version 150\n"
""
		"#define VERTEX_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numVertexShaderSources = sizeof( vertexShaderSources ) / sizeof( *vertexShaderSources );

	const char *fragmentShaderSources[] = { 
		"#version 150\n"
""
		"#define FRAGMENT_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numFragmentShaderSources = sizeof( fragmentShaderSources ) / sizeof( *fragmentShaderSources );

	const char *geometryShaderSources[] = { 
		"#version 150\n"
""
		"#define GEOMETRY_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numGeometryShaderSources = sizeof( geometryShaderSources ) / sizeof( *geometryShaderSources );

	GLuint program;

	// initialize the program for pass 0
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[0][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 0, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[0][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 0, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "position" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "vColor" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 0, program ) && success;

	programs[ 0 ] = program;

	// init uniform locations

	uniformLocations[ 0][ 0 ] = glGetUniformLocation( programs[ 0 ], "mWorldView" );


	uniformLocations[ 1][ 0 ] = glGetUniformLocation( programs[ 0 ], "mProjection" );


	uniformLocations[ 2][ 0 ] = glGetUniformLocation( programs[ 0 ], "vColorBB" );


	checkGLError();

	// init the samplers
	{
		glUseProgram( programs[ 0 ] );
		GLuint texUnit = 0;
		
	}


	glUseProgram( 0 );
	checkGLError();

	return success;
}

tileRenderingBB::~tileRenderingBB() {
	SafeRelease();
}

void tileRenderingBB::SafeRelease() {
	for( int i = 0 ; i < 1 ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}
