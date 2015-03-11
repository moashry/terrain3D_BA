
#ifndef __gfx_effect_header_decompressionStreamout
#define __gfx_effect_header_decompressionStreamout

#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class decompressionStreamout : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~decompressionStreamout();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < 1 );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
Renderer::UniformWrapper_usampler2D triangleDataTexture0;
Renderer::UniformWrapper_usampler2D triangleDataTexture1;
Renderer::UniformWrapper_uint firstStrip;
Renderer::UniformWrapper_uint base;
decompressionStreamout() : triangleDataTexture0(  ), triangleDataTexture1(  ), firstStrip( 1, programs, uniformLocations[ 2 ] ), base( 1, programs, uniformLocations[ 3 ] ){}

protected:
	static const unsigned numPasses = 1;
	
	GLuint programs[ 1 ];
	GLint uniformLocations[ 4 ][ 1 ];
};

#endif
