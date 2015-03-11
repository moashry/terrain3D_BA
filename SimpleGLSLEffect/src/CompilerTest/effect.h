
#pragma once
#include <assert.h>

#include "renderer/effect.h"
#include "renderer/uniform.h"

class effect : public Renderer::Effect {
public:
	unsigned GetNumPasses() const {
		return numPasses;
	}
	
	// aka input layout
	void SetupBinding(unsigned stride, unsigned offset) const;
	
	void SetupPass(unsigned pass) const;
	
	bool Create( const char *customCode = NULL );
	
	void SafeRelease();
	
	virtual ~effect();
	
	unsigned GetProgramHandle(unsigned pass) {
		assert( pass < 1 );
		return programs[ pass ];
	}
	
public: 
	// uniform variables
Renderer::UniformFloat3 ambientIntensity;
effect() : ambientIntensity( 1, programs, uniformLocations[ 0 ] ){}

protected:
	static const unsigned numPasses = 1;
	
	GLuint programs[ 1 ];
	GLint uniformLocations[ 1 ][ 1 ];
};
