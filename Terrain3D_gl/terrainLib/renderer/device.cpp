#include "device.h"

#include <stdio.h>

#include "buffer.h"
#include "texture.h"
#include "effect.h"

#include "uniform.h"

namespace Renderer {
#ifdef GLEW_MX
    __declspec( thread ) GLEWContext *__glewContext = 0;
#endif

	void Effect::ResetState() {
		glUseProgram( 0 );

		for( uint texUnit = 0; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
			Sampler::DisableTextureUnit( texUnit );
		}

		for( uint binding = 0 ; binding < MAX_NUM_BINDINGS ; binding++ ) {
			glDisableVertexAttribArray( binding );
		}
	}

	// Device part

	void checkGLError() {
#ifndef NDEBUG
		GLenum error = glGetError();
		switch( error ) {
			case GL_NO_ERROR:
				break;
			case GL_INVALID_ENUM:
				printf( "checkGLError: invalid enum\n" );
				break;
			case GL_INVALID_VALUE:
				printf( "checkGLError: invalid value\n" );
				break;
			case GL_INVALID_OPERATION:
				printf( "checkGLError: invalid operation\n" );
				break;
			case GL_STACK_OVERFLOW:
				printf( "checkGLError: stack overflow\n" );
				break;
			case GL_STACK_UNDERFLOW:
				printf( "checkGLError: stack underflow\n" );
				break;
			case GL_OUT_OF_MEMORY:
				printf( "checkGLError: out of memory\n" );
				break;
			default:
				printf( "checkGLError: unknown error %u\n", error );
				break;
		}
#endif
	}

	Device::Device()
		: indexDrawingArrayBufferID( 0 ), screenPassVertexBufferID( 0 )
	{
	}


	void Device::Create()
	{
#ifdef GLEW_MX
		glewSetContext( &glewContext );

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			// Problem: glewInit failed, something is seriously wrong.
			printf("Error: %s\n", glewGetErrorString(err));
			return;
		}
		
		printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

		glGenBuffers( 1, &indexDrawingArrayBufferID );
		float data = 0.0;

		ArrayBufferScope scope( indexDrawingArrayBufferID );
		glBufferData( GL_ARRAY_BUFFER, sizeof( data ), &data, GL_STATIC_DRAW );

		InitScreenPassVertexBuffer();
	}

	void Device::SafeRelease() {
		glDeleteBuffers( 1, &indexDrawingArrayBufferID );
		glDeleteBuffers( 1, &screenPassVertexBufferID );
	}

	void Device::StringMarker( const char *format, ... ) {
#ifdef GDEBUGGER
		char buffer[512];
		va_list list;
		va_start( list, format );
		_vsnprintf( buffer, 512, format, list );
		va_end( list );

		if( GLEW_GREMEDY_string_marker ) {
			char message[1024];
#ifdef UNICODE
			wcstombs( message, buffer, 1024 );
#else
			strcpy( message, buffer );
#endif
			glStringMarkerGREMEDY( 0, message );
		}
#endif
	}

	void Device::SetNoVertexBuffer() {
		glBindBuffer( GL_ARRAY_BUFFER, indexDrawingArrayBufferID );
		glVertexAttribPointer( 0, 4, GL_BYTE, true, 0, 0 );
		glEnableVertexAttribArray( 0 );

		// TODO: magic value [6/14/2010 Andreas Kirsch]
		for( uint i = 1 ; i < 8 ; i++ ) {
			glDisableVertexAttribArray( i );
		}

		checkGLError();
	}

	void Device::InitScreenPassVertexBuffer() {
		static const float positions[3][4] = {
			{ 3.0f, -1.0f, -0.5f, 1.0f},
			{-1.0f, -1.0f, -0.5f, 1.0f},
			{-1.0f,  3.0f, -0.5f, 1.0f}
		};

		glGenBuffers( 1, &screenPassVertexBufferID );
		ArrayBufferScope scope( screenPassVertexBufferID );
		glBufferData( GL_ARRAY_BUFFER, sizeof( positions ), &positions, GL_STATIC_DRAW );
	}

	void Device::SetupScreenPass() {
		glBindBuffer( GL_ARRAY_BUFFER, screenPassVertexBufferID );
		glVertexAttribPointer( 0, 4, GL_FLOAT, false, sizeof(float) * 4, 0 );
	}
}