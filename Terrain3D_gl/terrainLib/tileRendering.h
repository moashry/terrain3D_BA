
#ifndef __gfx_effect_header_tileRendering
#define __gfx_effect_header_tileRendering

#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class tileRendering : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~tileRendering();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < 2 );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
Renderer::UniformWrapper_mat4x4 mWorldView;
Renderer::UniformWrapper_vec3 vScale;
Renderer::UniformWrapper_float fBorderHeight;
Renderer::UniformWrapper_vec2 vTexCoordOffset;
Renderer::UniformWrapper_vec2 vTexCoordSize;
Renderer::UniformWrapper_mat4x4 mProjection;
Renderer::UniformWrapper_sampler2D txTerrain;
Renderer::UniformWrapper_float fSaturation;
tileRendering() : mWorldView( 2, programs, uniformLocations[ 0 ] ), vScale( 2, programs, uniformLocations[ 1 ] ), fBorderHeight( 2, programs, uniformLocations[ 2 ] ), vTexCoordOffset( 2, programs, uniformLocations[ 3 ] ), vTexCoordSize( 2, programs, uniformLocations[ 4 ] ), mProjection( 2, programs, uniformLocations[ 5 ] ), txTerrain(  ), fSaturation( 2, programs, uniformLocations[ 7 ] ){}

protected:
	static const unsigned numPasses = 2;
	
	GLuint programs[ 2 ];
	GLint uniformLocations[ 8 ][ 2 ];
};

#endif
