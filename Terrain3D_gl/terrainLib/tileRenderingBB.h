
#ifndef __gfx_effect_header_tileRenderingBB
#define __gfx_effect_header_tileRenderingBB

#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class tileRenderingBB : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~tileRenderingBB();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < 1 );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
Renderer::UniformWrapper_mat4x4 mWorldView;
Renderer::UniformWrapper_mat4x4 mProjection;
Renderer::UniformWrapper_vec4 vColorBB;
tileRenderingBB() : mWorldView( 1, programs, uniformLocations[ 0 ] ), mProjection( 1, programs, uniformLocations[ 1 ] ), vColorBB( 1, programs, uniformLocations[ 2 ] ){}

protected:
	static const unsigned numPasses = 1;
	
	GLuint programs[ 1 ];
	GLint uniformLocations[ 3 ][ 1 ];
};

#endif
