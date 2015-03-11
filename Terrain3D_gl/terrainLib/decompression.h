
#ifndef __gfx_effect_header_decompression
#define __gfx_effect_header_decompression

#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class decompression : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~decompression();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < 16 );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
Renderer::UniformWrapper_usamplerBuffer stripHeadersBuffer;
Renderer::UniformWrapper_usamplerBuffer stripDataBuffer;
Renderer::UniformWrapper_usampler2D texPingPong;
decompression() : stripHeadersBuffer(  ), stripDataBuffer(  ), texPingPong(  ){}

protected:
	static const unsigned numPasses = 16;
	
	GLuint programs[ 16 ];
	GLint uniformLocations[ 3 ][ 16 ];
};

#endif
