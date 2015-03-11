/******************************************************************************

effect.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#pragma once

#include "GL/glew.h"

#include "device.h"

namespace Renderer {
	class Effect {
		friend class Device;
	public:
		// aka input layout
		virtual void SetupBinding(uint stride, uint offset) const = 0;
		virtual uint GetNumPasses() const = 0;
		virtual void SetupPass(uint pass) const = 0;

		static void ResetState();

		// this breaks encapsulation but is useful if you want to prototype/embed other code
		virtual uint GetProgramHandle(uint pass) = 0;
	protected:
		static const int MAX_NUM_BINDINGS = 8;
		static const int MAX_NUM_TEXTURE_UNITS = 16;
	};
}