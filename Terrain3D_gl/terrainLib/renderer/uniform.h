/******************************************************************************

uniform.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#pragma once
#include <assert.h>

namespace Renderer {
	void checkGLError();

// uniformSetter is an expression that can access the following variables: program, location and value
#define UniformClass( glslName, valueType, uniformSetter ) \
	class UniformWrapper_##glslName { \
	public: \
		UniformWrapper_##glslName( uint numPrograms, const GLuint *programs, const GLint *locations ) \
			: numPrograms( numPrograms ), programs( programs ), locations( locations ) { \
		} \
 \
		void Set( const valueType value ) { \
			for( uint i = 0 ; i < numPrograms ; i++ ) { \
				if( isActive( i ) ) { \
					ProgramScope programScope( programs[ i ] ); \
					GLuint location = locations[ i ]; \
					uniformSetter; \
				} \
			} \
		} \
 \
		bool isActive( uint pass ) { \
			return locations[ pass ] != -1; \
		} \
 \
	private: \
		uint numPrograms; \
 \
		const GLint *locations; \
		const GLuint *programs; \
	}

#define UniformSingleClass( glslName, valueType, shortie) \
	UniformClass( glslName, valueType, glUniform1##shortie( location, value ) )
#define UniformVectorClass( glslName, valueType, shortie, compCount ) \
	UniformClass( glslName, valueType *, glUniform##compCount##shortie##v( location, 1, value ) )
	
#define UniformVectorClasses( glslName, valueType, shortie ) \
	UniformVectorClass( glslName##2, valueType, shortie, 2 ); \
	UniformVectorClass( glslName##3, valueType, shortie, 3 ); \
	UniformVectorClass( glslName##4, valueType, shortie, 4 )

#define UniformAlias( original, alias ) typedef UniformWrapper_##original UniformWrapper_##alias

	UniformSingleClass( float, GLfloat, f );
	UniformSingleClass( int, GLint, i );
	UniformSingleClass( uint, GLuint, ui );
	UniformSingleClass( bool, GLint, i );

	UniformVectorClasses( vec, GLfloat, f );
	UniformVectorClasses( ivec, GLint, i );
	UniformVectorClasses( uvec, GLuint, ui );
	UniformVectorClasses( bvec, GLint, i );

	UniformClass( mat2, float *, glUniformMatrix2fv( location, 1, false, value ) );
	UniformClass( mat3, float *, glUniformMatrix3fv( location, 1, false, value ) );
	UniformClass( mat4, float *, glUniformMatrix4fv( location, 1, false, value ) );

	UniformAlias( mat2, mat2x2 );
	UniformAlias( mat3, mat3x3 );
	UniformAlias( mat4, mat4x4 );

#undef UniformAlias
#undef UniformVectorClasses
#undef UniformVectorClass
#undef UniformSingleClass
#undef UniformClass

	class Sampler {
	public:
		static void DisableTextureUnit( uint texUnit ) {
			glActiveTexture( GetGLTexUnit( texUnit ) );
			glBindTexture( GL_TEXTURE_2D, 0 );
			glBindTexture( GL_TEXTURE_BUFFER, 0 );
			checkGLError();
		}

		static GLenum GetGLTexUnit( uint texUnit ) {
			return GL_TEXTURE0 + texUnit;
		}
	};

	template< GLenum target, typename setType >
	class SamplerTemplate : Sampler {
	public:
		// assert: the Sampler is active in <pass>
		void SetupPass( uint pass, uint texUnit ) const {
			glActiveTexture( GetGLTexUnit( texUnit ) );
			glBindTexture( target, textureID );

			checkGLError();
		}

		void Set( setType *sampler ) {
			textureID = sampler->GetTextureID();
		}

		SamplerTemplate() : textureID( 0 ) {}

	protected:
		GLuint textureID;
	};

	class RawTextureHandler {
	public:
		GLuint GetTextureID() { return m_textureID; }
		void SetTextureID( GLuint textureID ) { m_textureID = textureID; }

		RawTextureHandler( GLuint textureID = 0 ) : m_textureID( textureID ) {}

	protected:
		GLuint m_textureID;
	};

#define UniformType( glslType ) UniformWrapper_##glslType

	typedef SamplerTemplate< GL_TEXTURE_2D, Texture2D > UniformType( sampler2D );
	typedef SamplerTemplate< GL_TEXTURE_2D, Texture2D > UniformType( isampler2D );
	typedef SamplerTemplate< GL_TEXTURE_2D, Texture2D > UniformType( usampler2D );

	typedef SamplerTemplate< GL_TEXTURE_BUFFER, TextureBuffer > UniformType( samplerBuffer );
	typedef SamplerTemplate< GL_TEXTURE_BUFFER, TextureBuffer > UniformType( isamplerBuffer );
	typedef SamplerTemplate< GL_TEXTURE_BUFFER, TextureBuffer > UniformType( usamplerBuffer );

	typedef SamplerTemplate< GL_TEXTURE_1D, RawTextureHandler > SamplerRaw1D;
	typedef SamplerTemplate< GL_TEXTURE_2D, RawTextureHandler > SamplerRaw2D;
	typedef SamplerTemplate< GL_TEXTURE_1D_ARRAY, RawTextureHandler > SamplerRaw1DArray;
	typedef SamplerTemplate< GL_TEXTURE_2D_ARRAY, RawTextureHandler > SamplerRaw2DArray;

	typedef SamplerRaw1D UniformType( sampler1DShadow );
	typedef SamplerRaw1DArray UniformType( sampler1DArrayShadow );
	typedef SamplerRaw2D UniformType( sampler2DShadow );
	typedef SamplerRaw2DArray UniformType( sampler2DArrayShadow );

#undef UniformType
}