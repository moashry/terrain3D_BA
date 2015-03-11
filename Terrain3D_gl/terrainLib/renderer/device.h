/******************************************************************************

device.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#pragma once

#include <GL/glew.h>

#ifdef GDEBUGGER
#	include <GRemedyExtensions.h>
#endif

#include <assert.h>

namespace Renderer {
	//FIXME remove this [21/4/2010 Marc Treib]
#ifdef GLEW_MX
	extern __declspec( thread ) GLEWContext *__glewContext;
	inline GLEWContext * glewGetContext() { return __glewContext; }
	inline void glewSetContext( GLEWContext *context ) { __glewContext = context; }
#endif

	void checkGLError();

	class Effect;
	class Buffer;
	class TextureBuffer;
	class Texture2D;
	class FramebufferTexture2D;
	class DepthStencilTexture2D;

	class Device {
	public:
		Device();

		void Create();
		void SafeRelease();

		void StringMarker( const char *format, ... );

		void SetNoVertexBuffer();

		void SetupScreenPass();

	private:
		GLuint screenPassVertexBufferID;
		void InitScreenPassVertexBuffer();

		GLuint indexDrawingArrayBufferID;

#	ifdef GLEW_MX
	public:
		GLEWContext* glewGetContext() { return &glewContext; }
	private:
		GLEWContext glewContext;
#	endif
	};
}

#include "helper.h"
#include "texture.h"
#include "buffer.h"