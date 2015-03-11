/******************************************************************************

buffer.cpp

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#include "device.h"

namespace Renderer {
	void Buffer::SubData( GLuint offset, GLuint size, const GLvoid *data ) const {
		ArrayBufferScope scope( m_bufferID );
		glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
		checkGLError();
	}

	void Buffer::Bind( GLenum target ) {
		glBindBuffer( target, m_bufferID );
		checkGLError();
	}

	void Buffer::BindRange( GLenum target, GLuint index, GLintptr offset ) {
		assert( offset >= 0 );
		assert( m_size - offset >= 0 );
		glBindBufferRange( target, index, m_bufferID, offset, m_size - offset );
		checkGLError();
	}

	void Buffer::ResetBind( GLenum target ) {
		glBindBuffer( target, 0 );
		checkGLError();
	}

	void Buffer::ResetBind( GLenum target, GLuint index ) {
		glBindBufferBase( target, index, 0 );
		checkGLError();
	}

	void Buffer::Create( Buffer * &buffer, uint size, GLenum usage, const void *initData /*= 0 */ ) {
		// can ignore bindflags in OpenGL
		buffer = new Buffer( CreateGLBuffer( size, usage, initData ), size );
	}

	GLuint Buffer::CreateGLBuffer( uint size, GLenum usage, const void *initData /*= 0 */ ) {
		GLuint bufferID;
		glGenBuffers( 1, &bufferID );

		ArrayBufferScope scope( bufferID );
		glBufferData( GL_ARRAY_BUFFER, size, initData, usage );
		checkGLError();

		return bufferID;
	}

	Buffer::~Buffer() {
		glDeleteBuffers( 1, &m_bufferID );
	}
	TextureBuffer::~TextureBuffer() {
		glDeleteTextures( 1, &m_textureID );
	}

	void TextureBuffer::Create( TextureBuffer * &buffer, GLenum format, uint size, GLenum usage, const void *initData /*= 0 */ ) {
		GLuint bufferID;
		bufferID = CreateGLBuffer( size, usage, initData );

		GLuint textureID;
		glGenTextures( 1, &textureID );

		TextureBufferScope scope( textureID );
		glTexBuffer( GL_TEXTURE_BUFFER, format, bufferID );
		checkGLError();

		buffer = new TextureBuffer( bufferID, size, textureID, format );
	}
}