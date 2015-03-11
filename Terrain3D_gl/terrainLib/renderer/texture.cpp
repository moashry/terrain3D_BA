/******************************************************************************

texture.cpp

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#include "device.h"

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"

namespace Renderer {
	Texture::Texture( GLuint textureID, GLenum internalFormat )
		: m_textureID( textureID ), m_internalFormat( internalFormat )
	{
	}

	Texture::~Texture() {
		glDeleteTextures( 1, &m_textureID );
	}

	void Texture2D::GenerateMipMap() {
		Texture2DScope scope( m_textureID );
		glGenerateMipmap( GL_TEXTURE_2D );
		checkGLError();
	}

	void Texture2D::CompressedImage( GLint level, const GLvoid *data ) {
		// mip-mapping requires adapted texture extents
		uint width = m_width >> level;
		uint height = m_height >> level;

		//FIXME: this should probably make realWidth/realHeight a multiple of four, not minimum four [15/4/2010 Marc Treib]
		uint realWidth = width > 4 ? width : 4;
		uint realHeight = height > 4 ? height : 4;
		uint imageSize = GetSizeForInternalFormat( realWidth, realHeight, m_internalFormat );

		Texture2DScope scope( m_textureID );
		glCompressedTexImage2D( GL_TEXTURE_2D, level, m_internalFormat, width, height, 0, imageSize, data );
		checkGLError();
	}

	GLuint Texture2D::CreateGLTexture( uint width, uint height, uint mipLevels, GLenum internalFormat, const void *initData /*= 0*/ ) {
		GLuint textureID;
		glGenTextures( 1, &textureID );

		Texture2DScope scope( textureID );

		// mip level == 0 means full mip pyramid
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( mipLevels != 1 ) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
		if( mipLevels != 0 ) {
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevels - 1 );
		}
		// NOTE: we can ignore pitch again, because its just the real row length in all calls [8/15/2009 Andreas Kirsch]
		if( !IsCompressedFormat( internalFormat ) ) {
			glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GetFormatForInternalFormat( internalFormat ), GetTypeForInternalFormat( internalFormat ), initData );
		}
		else {
			if( !initData ) {
				glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
			}
			else {
				glCompressedTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GetSizeForInternalFormat( width, height, internalFormat ), initData );
			}
		}
		checkGLError();

		return textureID;
	}

	void Texture2D::Create( uint width, uint height, uint mipLevels, GLenum internalFormat, FramebufferTexture2D* &texture, const void *initData /*= 0 */ ) {
		// pitch is not needed because it always is a full row, when used
		GLuint textureID = CreateGLTexture( width, height, mipLevels, internalFormat, initData );
		texture = new FramebufferTexture2D( textureID, internalFormat, 0, width, height );
	}

	bool Texture2D::CreateFromFile( const char *filename, FramebufferTexture2D* &texture, uint &width, uint &height ) {
		int comp;
		// make sure that
		width = height = 0;
		stbi_uc* imageData = stbi_load( filename, (int*) &width, (int*) &height, &comp, 4 );

		if( imageData ) {
			Create( width, height, 0, GL_RGBA8, texture, imageData );
			stbi_image_free( imageData );

			return true;
		}
		else {
			return false;
		}
	}

	bool Texture2D::CreateFromMemory( const uchar *data, uint numBytes, FramebufferTexture2D* &texture, uint &width, uint &height ) {
		int comp;
		// make sure that
		width = height = 0;
		stbi_uc* imageData = stbi_load_from_memory( data, numBytes, (int*) &width, (int*) &height, &comp, 4 );

		if( imageData ) {
			Create( width, height, 0, GL_RGBA8, texture, imageData );
			stbi_image_free( imageData );

			return true;
		}
		else {
			return false;
		}
	}

	FramebufferTexture2D::~FramebufferTexture2D() {
		glDeleteFramebuffers( 1, &m_framebufferID );
	}

	void FramebufferTexture2D::BindFramebuffer() {
		assert( m_framebufferID != 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, m_framebufferID );
		checkGLError();
	}

	void FramebufferTexture2D::Create( uint width, uint height, uint mipLevels, GLenum format, FramebufferTexture2D* &texture ) {
		// don't need generateMipMaps, because its always allowed in OGL (the flag does something different: automatically generates mipmaps without the developer having control over it)
		GLuint textureID = CreateGLTexture( width, height, mipLevels, format, 0 );

		GLuint framebufferID;
		glGenFramebuffers( 1, &framebufferID );

		// WTF?! Apparently, have to use DSA here, or stuff breaks in weird ways if the FBO is shared among contexts.
		// This doesn't work:
		//FramebufferScope scope( framebufferID );
		//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0 );
		// But this does: (but not if glBindFramebuffer( GL_FRAMEBUFFER, framebufferID ) is called before?!)
		glNamedFramebufferTexture2DEXT( framebufferID, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0 );
		// ^^ using nVidia drivers 197.45 [21/4/2010 Marc Treib]
		checkGLError();

		texture = new FramebufferTexture2D(textureID, format, framebufferID, width, height);
	}
}