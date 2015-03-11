/* Shader Code Information


Shared Code Blocks:

** Block 1 - uniform block (uniform declarations) **:
uniform vec3 ambientIntensity = vec3( 0, 0, 0 );


Passes:

Pass 0:

** Vertex Shader Code for Pass 0 **

uniform vec3 ambientIntensity = vec3( 0, 0, 0 );


#line 13 "..\\CompilerTest\\effect.gfx"

		void main(void)
		{
			gl_Position = vec4( 0, 0, 0, 1 );
		}

** Geometry Shader Code for Pass 0 **

uniform vec3 ambientIntensity = vec3( 0, 0, 0 );


#line 20 "..\\CompilerTest\\effect.gfx"

		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;
		 
		void main() {
		  for(int i = 0; i < gl_in.length(); i++) {
			gl_Position = gl_in[i].gl_Position;
			EmitVertex();
		  }
		  EndPrimitive();
		}

** Fragment Shader Code for Pass 0 **

uniform vec3 ambientIntensity = vec3( 0, 0, 0 );


#line 33 "..\\CompilerTest\\effect.gfx"

		void main(void)
		{
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		}


*/

#include <stdio.h>
#include <GL/glew.h>
#include "effect.h"

using namespace Renderer;

static const char *codeForPasses[][3] = {
// pass 0
{
"uniform vec3 ambientIntensity = vec3( 0, 0, 0 );\n"
"\n"
"\n"
"#line 13 \"..\\\\CompilerTest\\\\effect.gfx\"\n"
"\n"
"\t\tvoid main(void)\n"
"\t\t{\n"
"\t\t\tgl_Position = vec4( 0, 0, 0, 1 );\n"
"\t\t}"
,
"uniform vec3 ambientIntensity = vec3( 0, 0, 0 );\n"
"\n"
"\n"
"#line 33 \"..\\\\CompilerTest\\\\effect.gfx\"\n"
"\n"
"\t\tvoid main(void)\n"
"\t\t{\n"
"\t\t\tgl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"\t\t}"
,
"uniform vec3 ambientIntensity = vec3( 0, 0, 0 );\n"
"\n"
"\n"
"#line 20 \"..\\\\CompilerTest\\\\effect.gfx\"\n"
"\n"
"\t\tlayout(triangles) in;\n"
"\t\tlayout(triangle_strip, max_vertices = 3) out;\n"
"\t\t \n"
"\t\tvoid main() {\n"
"\t\t  for(int i = 0; i < gl_in.length(); i++) {\n"
"\t\t\tgl_Position = gl_in[i].gl_Position;\n"
"\t\t\tEmitVertex();\n"
"\t\t  }\n"
"\t\t  EndPrimitive();\n"
"\t\t}"
}
};

/*
static void checkGLError() {
	GLenum error = glGetError();
	if( error != GL_NO_ERROR ) {
		printf( "%s\n", gluErrorString(error) );
	}
}*/
void Renderer::checkGLError();


void effect::SetupPass( unsigned pass ) const {
	if( pass >= 1 ) {
		// error?
		return;
	}
	glUseProgram( programs[ pass ] );
	checkGLError();

	// TODO: this should allow us to simplify the code [6/12/2010 Andreas Kirsch]
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );
	
	switch( pass ) {

		case 0: {

			// set the texture bindings
			GLuint texUnit = 0;
			

			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 9 "..\\CompilerTest\\effect.gfx"

		glDisable( GL_DEPTH_TEST ;
			}

			return;
		}

	}
}

void effect::SetupBinding( unsigned stride, unsigned offset ) const {
	unsigned totalSize = stride;
	if( !totalSize ) {
		totalSize = 0;
	}

		unsigned oldNumUsedBindings = numUsedBindings;
		numUsedBindings = 0;

	// disable unused vertex arrays
	for( unsigned binding = numUsedBindings ; binding < MAX_NUM_BINDINGS ; binding++ ) {
		glDisableVertexAttribArray( binding );
	}

	checkGLError();
}

static const char *PrintableNameForShaderType( GLenum shaderType ) {
	switch( shaderType ) {
	case GL_VERTEX_SHADER:
		return "Vertex Shader";
	case GL_GEOMETRY_SHADER_ARB:
		return "Geometry Shader ARB";
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

bool effect::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique effect\n\n" );

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
	{
		GLuint geometryShader = glCreateShader( GL_GEOMETRY_SHADER_ARB );
		geometryShaderSources[ numGeometryShaderSources - 1 ] = codeForPasses[ 0][2];
		glShaderSource( geometryShader, numGeometryShaderSources, geometryShaderSources, NULL );
		glCompileShader( geometryShader );
		checkGLError();

		success = CompileLog( 0, GL_GEOMETRY_SHADER_ARB, geometryShader ) && success;

		glAttachShader( program, geometryShader );
		glDeleteShader( geometryShader );
		checkGLError();

		// TODO: use layouts from the GLSL specs? [5/31/2010 Andreas Kirsch]
		/*// setup the parameters
		glProgramParameteriEXT( program, GL_GEOMETRY_INPUT_TYPE_EXT, GL_<pass.geometryShader.inputType> );
		glProgramParameteriEXT( program, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_<pass.geometryShader.outputType> );
		glProgramParameteriEXT( program, GL_GEOMETRY_VERTICES_OUT_EXT, <pass.geometryShader.maxEmitVertices> );
		checkGLError();*/
	}

	// link the attributes

	// link the frag data name
	glBindFragDataLocation( program, 0, "test1" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 0, program ) && success;

	programs[ 0 ] = program;

	// init uniform locations

	uniformLocations[ 0][ 0 ] = glGetUniformLocation( programs[ 0 ], "ambientIntensity" );


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

effect::~effect() {
	SafeRelease();
}

void effect::SafeRelease() {
	for( int i = 0 ; i < 1 ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}