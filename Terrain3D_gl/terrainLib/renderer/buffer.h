/******************************************************************************

buffer.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#pragma once


#include <GL/glew.h>

#include "global.h"


namespace Renderer {
	class Buffer {
	public:
		virtual ~Buffer();

		GLuint GetBufferID() const {
			return m_bufferID;
		}

		uint GetSize() const {
			return m_size;
		}

		// TODO: method ChangeSize? [3/12/2010 Andreas Kirsch]

		// update the data
		void SubData( GLuint offset, GLuint size, const GLvoid *data ) const;

		void Bind( GLenum target );

		void BindRange( GLenum target, GLuint index, GLintptr offset );

		static void ResetBind( GLenum target );
		static void ResetBind( GLenum target, GLuint index );
	
		static void Create( Buffer * &buffer, uint size, GLenum usage, const void *initData = 0 );

		Buffer( GLuint bufferID, uint size ) : m_bufferID( bufferID ), m_size( size ) {}

	protected:
		GLuint m_bufferID;
		uint m_size;

		static GLuint CreateGLBuffer( uint size, GLenum usage, const void *initData = 0 );
	};

	// buffer + shader resource view
	class TextureBuffer : public Buffer {
		friend class Device;
	public:
		virtual ~TextureBuffer();

		// TODO: use composition with a Texture object! [3/13/2010 Andreas Kirsch]
		GLuint GetTextureID() const {
			return m_textureID;
		}

		GLenum GetInternalFormat() const {
			return m_internalFormat;
		}

		static void Create( TextureBuffer * &buffer, GLenum format, uint size, GLenum usage, const void *initData = 0 );

		TextureBuffer( GLuint bufferID, uint size, GLuint textureID, GLenum internalFormat ) : Buffer( bufferID, size ), m_textureID( textureID ), m_internalFormat( internalFormat ) {}
	
	protected:

		GLuint m_textureID;
		GLenum m_internalFormat;
	}; 
}