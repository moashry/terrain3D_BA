/******************************************************************************

helper.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#pragma once

#include "global.h"

namespace Renderer {
	template<GLenum state>
	struct StateScope {
		GLint oldState;

		StateScope() {
			glGetIntegerv( state, &oldState );
		}
	};

	struct FramebufferScope : public StateScope< GL_FRAMEBUFFER_BINDING > {
		FramebufferScope( GLuint id ) {
			glBindFramebuffer( GL_FRAMEBUFFER, id );
		}

		~FramebufferScope() {
			glBindFramebuffer( GL_FRAMEBUFFER, oldState );
		}
	};

	struct Texture2DScope : public StateScope< GL_TEXTURE_BINDING_2D > {
		Texture2DScope( GLuint id ) {
			glBindTexture( GL_TEXTURE_2D, id );
		}

		~Texture2DScope() {
			glBindTexture( GL_TEXTURE_2D, oldState );
		}
	};

	struct TextureBufferScope : public StateScope< GL_TEXTURE_BINDING_BUFFER > {
		TextureBufferScope( GLuint id ) {
			glBindTexture( GL_TEXTURE_BUFFER, id );
		}

		~TextureBufferScope() {
			glBindTexture( GL_TEXTURE_BUFFER, oldState );
		}
	};

	struct ArrayBufferScope : public StateScope< GL_ARRAY_BUFFER_BINDING > {
		ArrayBufferScope( GLuint id ) {
			glBindBuffer( GL_ARRAY_BUFFER, id );
		}

		~ArrayBufferScope() {
			glBindBuffer( GL_ARRAY_BUFFER, oldState );
		}
	};

	struct ProgramScope : public StateScope< GL_CURRENT_PROGRAM > {
		ProgramScope( GLuint id ) {
			glUseProgram( id );
		}

		~ProgramScope() {
			glUseProgram( oldState );
		}
	};
	
	inline uint GetSizeForInternalFormat( uint width, uint height, GLenum internalFormat ) {
		switch( internalFormat ) {
			case GL_R16UI:
				return width * height * 2;
			case GL_R32UI:
				return width * height * 4;
			case GL_RGBA32UI:
				return width * height * 4 * 4;
			case GL_RGBA8:
				return width * height * 4;
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				return width * height / 2;
			default:
				assert( false );
				return 0;
		}
	}

	inline GLenum GetTypeForInternalFormat( GLuint internalFormat ) {
		switch( internalFormat ) {
			case GL_R16UI:
				return GL_UNSIGNED_SHORT;
			case GL_R32UI:
				return GL_UNSIGNED_INT;
			case GL_RGBA32UI:
				return GL_UNSIGNED_INT;
			case GL_RGBA8:
				return GL_UNSIGNED_BYTE;
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				/* compressed internal formats have no type */
			default:
				assert( false );
				return 0;
		}
	}

	inline GLenum GetFormatForInternalFormat( GLuint internalFormat ) {
		switch( internalFormat ) {
			case GL_R16UI:
				return GL_RED_INTEGER;
			case GL_R32UI:
				return GL_RED_INTEGER;
			case GL_RGBA32UI:
				return GL_RGBA_INTEGER;
			case GL_RGBA8:
				return GL_RGBA;
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				/* compressed internal formats have no format */
			default:
				assert( false );
				return 0;
		}
	}

	inline bool IsCompressedFormat( GLuint internalFormat ) {
		switch( internalFormat ) {
			case GL_R16UI:
			case GL_R32UI:
			case GL_RGBA32UI:
			case GL_RGBA8:
				return false;
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				return true;
			default:
				assert( false );
				return false;
		}	
	}
}