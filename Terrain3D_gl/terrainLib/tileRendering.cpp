/* Shader Code Information


Shared Code Blocks:

** Block 1 - for all shaders **:

#line 2 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx"

	
	
	
	
	

	

#line 13 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/ * Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
* /
		return result;
	}

	uvec3 UnpackVertex(uint v)
	{
		uvec3 result;

		result.x = v & ((1u << 10u) - 1u);
		result.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;
		result.z = v >> (2u * 10u);
		return result;
	}
** Block 2 - for all shaders **:

#line 4 ".\\\\terrainLib\\\\tileRendering.gfx"

	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);
** Block 3 - uniform block (uniform declarations) **:
uniform mat4x4 mWorldView ;
uniform vec3 vScale ;
uniform float fBorderHeight ;
uniform vec2 vTexCoordOffset ;
uniform vec2 vTexCoordSize ;
uniform mat4x4 mProjection ;
uniform sampler2D txTerrain ;

** Block 4 - for vertex shaders **:

#line 22 ".\\\\terrainLib\\\\tileRendering.gfx"

$ifndef INDEXED_DRAWING
	in uint pos;
$else
$	define pos uint( gl_VertexID )
$endif

	out vec2 vTexCoord;
	
		out vec3 vPosCS;
** Block 5 - for fragment shaders **:

#line 35 ".\\\\terrainLib\\\\tileRendering.gfx"

	in vec2 vTexCoord;
	
		in vec3 vPosCS;
	#line 41 ".\\terrainLib\\tileRendering.gfx"

	out vec4 vColor;
** Block 6 - for vertex shaders **:

#line 44 ".\\\\terrainLib\\\\tileRendering.gfx"

	
	// f = exp(-(density * dist)^n) => density = (-ln(f))^(1/n) / dist

	
	
	
	#line 53 ".\\terrainLib\\tileRendering.gfx"

	//--------------------------------------------------------------------------------------
	// Vertex Shader
	//--------------------------------------------------------------------------------------
	void VS()
	{ 
		uvec3 vuPos = UnpackVertex( pos );
	    
		bool bBorder = false;
		if (vuPos.x > 512u)
		{
			vuPos.x = (vuPos.x << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
		else if (vuPos.y > 512u)
		{
			vuPos.y = (vuPos.y << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
	    
		vec4 vPos = vec4(vuPos, 1.0f);

		vTexCoord = vTexCoordOffset + (vPos.xy / vec2(512u, 512u)) * vTexCoordSize;
	    
		vPos.z += 0.5f;
		vPos.xyz *= vScale;

		if (bBorder)
		{
			vPos.z -= fBorderHeight;
		}
	    
		vPos = mWorldView * vPos;
	
		vPosCS = vPos.xyz / vPos.w;
	#line 89 ".\\terrainLib\\tileRendering.gfx"
		gl_Position = mProjection * vPos;
	}
** Block 7 - for fragment shaders **:

#line 96 ".\\\\terrainLib\\\\tileRendering.gfx"

	void PS()
	{
		vColor = texture( txTerrain, vTexCoord );

	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 107 ".\\terrainLib\\tileRendering.gfx"
	}
** Block 8 - uniform block (uniform declarations) **:
uniform float fSaturation ;

** Block 9 - for fragment shaders **:

#line 113 ".\\\\terrainLib\\\\tileRendering.gfx"

	vec3 SAT_MODIFY(vec3 clr) {
		float mx    = max(clr.r,max(clr.g,clr.b));
		float mn    = min(clr.r,min(clr.g,clr.b));
		float delta = 1.0f/(mx-mn);
		float sat   = (mx==0 ? 0.0f : 1.0f-(mn/mx))*fSaturation;
		float val	= mx;
		
		float f;
		// hue
		if (mx==mn)				return	vec3(val,val,val);	// achromatic case	OK	 
		else if (mx==clr.r) {
			if (clr.g>=clr.b)	{ f=(clr.g-clr.b)*delta;		return val*vec3(1.0f, (1.0f-(1.0f-f)*sat), (1.0f-sat)); }
			else				{ f=1.0f+(clr.g-clr.b)*delta;	return val*vec3(1.0f, (1.0f-sat),(1.0f-f*sat)); }
		}
		else if (mx==clr.g) {
			if (clr.b>=clr.r)	{ f=(clr.b-clr.r)*delta;		return val*vec3((1.0f-sat), 1.0f, (1.0f-(1.0f-f)*sat)); }
			else				{ f=1.0f+(clr.b-clr.r)*delta;	return val*vec3((1.0f-f*sat), 1.0f,(1.0f-sat)); }
		}
		else {
			if (clr.r>=clr.g)	{ f=(clr.r-clr.g)*delta;		return val*vec3((1.0f-(1.0f-f)*sat), (1.0f-sat), 1.0f); }
			else				{ f=1.0f+(clr.r-clr.g)*delta;	return val*vec3((1.0f-sat),(1.0f-f*sat), 1.0f); }
		}
	}

	vec4 SAT_MODIFY(vec4 rgba) {
		return vec4(SAT_MODIFY(rgba.xyz),rgba.w);
	}
	
	void PSSatModify()
	{
		vColor = SAT_MODIFY(texture(txTerrain, vTexCoord));
	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 151 ".\\terrainLib\\tileRendering.gfx"
	}

Passes:

Pass 0:

** Vertex Shader Code for Pass 0 **


#line 2 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx"

	
	
	
	
	

	

#line 13 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/ * Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
* /
		return result;
	}

	uvec3 UnpackVertex(uint v)
	{
		uvec3 result;

		result.x = v & ((1u << 10u) - 1u);
		result.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;
		result.z = v >> (2u * 10u);
		return result;
	}
#line 4 ".\\\\terrainLib\\\\tileRendering.gfx"

	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;
uniform vec3 vScale ;
uniform float fBorderHeight ;
uniform vec2 vTexCoordOffset ;
uniform vec2 vTexCoordSize ;
uniform mat4x4 mProjection ;
uniform sampler2D txTerrain ;

#line 22 ".\\\\terrainLib\\\\tileRendering.gfx"

$ifndef INDEXED_DRAWING
	in uint pos;
$else
$	define pos uint( gl_VertexID )
$endif

	out vec2 vTexCoord;
	
		out vec3 vPosCS;
#line 44 ".\\\\terrainLib\\\\tileRendering.gfx"

	
	// f = exp(-(density * dist)^n) => density = (-ln(f))^(1/n) / dist

	
	
	
	#line 53 ".\\terrainLib\\tileRendering.gfx"

	//--------------------------------------------------------------------------------------
	// Vertex Shader
	//--------------------------------------------------------------------------------------
	void VS()
	{ 
		uvec3 vuPos = UnpackVertex( pos );
	    
		bool bBorder = false;
		if (vuPos.x > 512u)
		{
			vuPos.x = (vuPos.x << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
		else if (vuPos.y > 512u)
		{
			vuPos.y = (vuPos.y << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
	    
		vec4 vPos = vec4(vuPos, 1.0f);

		vTexCoord = vTexCoordOffset + (vPos.xy / vec2(512u, 512u)) * vTexCoordSize;
	    
		vPos.z += 0.5f;
		vPos.xyz *= vScale;

		if (bBorder)
		{
			vPos.z -= fBorderHeight;
		}
	    
		vPos = mWorldView * vPos;
	
		vPosCS = vPos.xyz / vPos.w;
	#line 89 ".\\terrainLib\\tileRendering.gfx"
		gl_Position = mProjection * vPos;
	}uniform float fSaturation ;


#line 156 ".\\\\terrainLib\\\\tileRendering.gfx"
 void main() { VS(); }

** Fragment Shader Code for Pass 0 **


#line 2 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx"

	
	
	
	
	

	

#line 13 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/ * Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
* /
		return result;
	}

	uvec3 UnpackVertex(uint v)
	{
		uvec3 result;

		result.x = v & ((1u << 10u) - 1u);
		result.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;
		result.z = v >> (2u * 10u);
		return result;
	}
#line 4 ".\\\\terrainLib\\\\tileRendering.gfx"

	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;
uniform vec3 vScale ;
uniform float fBorderHeight ;
uniform vec2 vTexCoordOffset ;
uniform vec2 vTexCoordSize ;
uniform mat4x4 mProjection ;
uniform sampler2D txTerrain ;

#line 35 ".\\\\terrainLib\\\\tileRendering.gfx"

	in vec2 vTexCoord;
	
		in vec3 vPosCS;
	#line 41 ".\\terrainLib\\tileRendering.gfx"

	out vec4 vColor;
#line 96 ".\\\\terrainLib\\\\tileRendering.gfx"

	void PS()
	{
		vColor = texture( txTerrain, vTexCoord );

	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 107 ".\\terrainLib\\tileRendering.gfx"
	}uniform float fSaturation ;

#line 113 ".\\\\terrainLib\\\\tileRendering.gfx"

	vec3 SAT_MODIFY(vec3 clr) {
		float mx    = max(clr.r,max(clr.g,clr.b));
		float mn    = min(clr.r,min(clr.g,clr.b));
		float delta = 1.0f/(mx-mn);
		float sat   = (mx==0 ? 0.0f : 1.0f-(mn/mx))*fSaturation;
		float val	= mx;
		
		float f;
		// hue
		if (mx==mn)				return	vec3(val,val,val);	// achromatic case	OK	 
		else if (mx==clr.r) {
			if (clr.g>=clr.b)	{ f=(clr.g-clr.b)*delta;		return val*vec3(1.0f, (1.0f-(1.0f-f)*sat), (1.0f-sat)); }
			else				{ f=1.0f+(clr.g-clr.b)*delta;	return val*vec3(1.0f, (1.0f-sat),(1.0f-f*sat)); }
		}
		else if (mx==clr.g) {
			if (clr.b>=clr.r)	{ f=(clr.b-clr.r)*delta;		return val*vec3((1.0f-sat), 1.0f, (1.0f-(1.0f-f)*sat)); }
			else				{ f=1.0f+(clr.b-clr.r)*delta;	return val*vec3((1.0f-f*sat), 1.0f,(1.0f-sat)); }
		}
		else {
			if (clr.r>=clr.g)	{ f=(clr.r-clr.g)*delta;		return val*vec3((1.0f-(1.0f-f)*sat), (1.0f-sat), 1.0f); }
			else				{ f=1.0f+(clr.r-clr.g)*delta;	return val*vec3((1.0f-sat),(1.0f-f*sat), 1.0f); }
		}
	}

	vec4 SAT_MODIFY(vec4 rgba) {
		return vec4(SAT_MODIFY(rgba.xyz),rgba.w);
	}
	
	void PSSatModify()
	{
		vColor = SAT_MODIFY(texture(txTerrain, vTexCoord));
	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 151 ".\\terrainLib\\tileRendering.gfx"
	}

#line 157 ".\\\\terrainLib\\\\tileRendering.gfx"
 void main() { PS(); }


Pass 1:

** Vertex Shader Code for Pass 1 **


#line 2 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx"

	
	
	
	
	

	

#line 13 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/ * Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
* /
		return result;
	}

	uvec3 UnpackVertex(uint v)
	{
		uvec3 result;

		result.x = v & ((1u << 10u) - 1u);
		result.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;
		result.z = v >> (2u * 10u);
		return result;
	}
#line 4 ".\\\\terrainLib\\\\tileRendering.gfx"

	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;
uniform vec3 vScale ;
uniform float fBorderHeight ;
uniform vec2 vTexCoordOffset ;
uniform vec2 vTexCoordSize ;
uniform mat4x4 mProjection ;
uniform sampler2D txTerrain ;

#line 22 ".\\\\terrainLib\\\\tileRendering.gfx"

$ifndef INDEXED_DRAWING
	in uint pos;
$else
$	define pos uint( gl_VertexID )
$endif

	out vec2 vTexCoord;
	
		out vec3 vPosCS;
#line 44 ".\\\\terrainLib\\\\tileRendering.gfx"

	
	// f = exp(-(density * dist)^n) => density = (-ln(f))^(1/n) / dist

	
	
	
	#line 53 ".\\terrainLib\\tileRendering.gfx"

	//--------------------------------------------------------------------------------------
	// Vertex Shader
	//--------------------------------------------------------------------------------------
	void VS()
	{ 
		uvec3 vuPos = UnpackVertex( pos );
	    
		bool bBorder = false;
		if (vuPos.x > 512u)
		{
			vuPos.x = (vuPos.x << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
		else if (vuPos.y > 512u)
		{
			vuPos.y = (vuPos.y << 2u) & ((1u << 10u) - 1u);
			bBorder = true;
		}
	    
		vec4 vPos = vec4(vuPos, 1.0f);

		vTexCoord = vTexCoordOffset + (vPos.xy / vec2(512u, 512u)) * vTexCoordSize;
	    
		vPos.z += 0.5f;
		vPos.xyz *= vScale;

		if (bBorder)
		{
			vPos.z -= fBorderHeight;
		}
	    
		vPos = mWorldView * vPos;
	
		vPosCS = vPos.xyz / vPos.w;
	#line 89 ".\\terrainLib\\tileRendering.gfx"
		gl_Position = mProjection * vPos;
	}uniform float fSaturation ;


#line 166 ".\\\\terrainLib\\\\tileRendering.gfx"
 void main() { VS(); }

** Fragment Shader Code for Pass 1 **


#line 2 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx"

	
	
	
	
	

	

#line 13 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\code\\kirschan\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/ * Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
* /
		return result;
	}

	uvec3 UnpackVertex(uint v)
	{
		uvec3 result;

		result.x = v & ((1u << 10u) - 1u);
		result.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;
		result.z = v >> (2u * 10u);
		return result;
	}
#line 4 ".\\\\terrainLib\\\\tileRendering.gfx"

	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;
uniform vec3 vScale ;
uniform float fBorderHeight ;
uniform vec2 vTexCoordOffset ;
uniform vec2 vTexCoordSize ;
uniform mat4x4 mProjection ;
uniform sampler2D txTerrain ;

#line 35 ".\\\\terrainLib\\\\tileRendering.gfx"

	in vec2 vTexCoord;
	
		in vec3 vPosCS;
	#line 41 ".\\terrainLib\\tileRendering.gfx"

	out vec4 vColor;
#line 96 ".\\\\terrainLib\\\\tileRendering.gfx"

	void PS()
	{
		vColor = texture( txTerrain, vTexCoord );

	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 107 ".\\terrainLib\\tileRendering.gfx"
	}uniform float fSaturation ;

#line 113 ".\\\\terrainLib\\\\tileRendering.gfx"

	vec3 SAT_MODIFY(vec3 clr) {
		float mx    = max(clr.r,max(clr.g,clr.b));
		float mn    = min(clr.r,min(clr.g,clr.b));
		float delta = 1.0f/(mx-mn);
		float sat   = (mx==0 ? 0.0f : 1.0f-(mn/mx))*fSaturation;
		float val	= mx;
		
		float f;
		// hue
		if (mx==mn)				return	vec3(val,val,val);	// achromatic case	OK	 
		else if (mx==clr.r) {
			if (clr.g>=clr.b)	{ f=(clr.g-clr.b)*delta;		return val*vec3(1.0f, (1.0f-(1.0f-f)*sat), (1.0f-sat)); }
			else				{ f=1.0f+(clr.g-clr.b)*delta;	return val*vec3(1.0f, (1.0f-sat),(1.0f-f*sat)); }
		}
		else if (mx==clr.g) {
			if (clr.b>=clr.r)	{ f=(clr.b-clr.r)*delta;		return val*vec3((1.0f-sat), 1.0f, (1.0f-(1.0f-f)*sat)); }
			else				{ f=1.0f+(clr.b-clr.r)*delta;	return val*vec3((1.0f-f*sat), 1.0f,(1.0f-sat)); }
		}
		else {
			if (clr.r>=clr.g)	{ f=(clr.r-clr.g)*delta;		return val*vec3((1.0f-(1.0f-f)*sat), (1.0f-sat), 1.0f); }
			else				{ f=1.0f+(clr.r-clr.g)*delta;	return val*vec3((1.0f-sat),(1.0f-f*sat), 1.0f); }
		}
	}

	vec4 SAT_MODIFY(vec4 rgba) {
		return vec4(SAT_MODIFY(rgba.xyz),rgba.w);
	}
	
	void PSSatModify()
	{
		vColor = SAT_MODIFY(texture(txTerrain, vTexCoord));
	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 151 ".\\terrainLib\\tileRendering.gfx"
	}

#line 167 ".\\\\terrainLib\\\\tileRendering.gfx"
 void main() { PSSatModify(); }


*/

#include <stdio.h>
#include <GL/glew.h>
#include "tileRendering.h"

using namespace Renderer;

static const char *codeForPasses[][3] = {
// pass 0
{
"\n"
"#line 2 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\t\n"
"\n"
"#line 13 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t#line 17 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\n"
"\tuint PackVertex(uvec3 v)\n"
"\t{\n"
"\t\tuint result;\n"
"\n"
"\t\tresult = v.x;\n"
"\t\tresult |= v.y << 10u;\n"
"\t\tresult |= v.z << (2u * 10u);\n"
"/* Use this for debugging GPU decompression code\n"
"\t\tresult = v.x & ((1u << NUM_XY_BITS) - 1u);\n"
"\t\tresult |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;\n"
"\t\tresult |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);\n"
"*/\n"
"\t\treturn result;\n"
"\t}\n"
"\n"
"\tuvec3 UnpackVertex(uint v)\n"
"\t{\n"
"\t\tuvec3 result;\n"
"\n"
"\t\tresult.x = v & ((1u << 10u) - 1u);\n"
"\t\tresult.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;\n"
"\t\tresult.z = v >> (2u * 10u);\n"
"\t\treturn result;\n"
"\t}\n"
"#line 4 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tconst vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;\n"
"uniform vec3 vScale ;\n"
"uniform float fBorderHeight ;\n"
"uniform vec2 vTexCoordOffset ;\n"
"uniform vec2 vTexCoordSize ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform sampler2D txTerrain ;\n"
"\n"
"#line 22 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"#ifndef INDEXED_DRAWING\n"
"\tin uint pos;\n"
"#else\n"
"#\tdefine pos uint( gl_VertexID )\n"
"#endif\n"
"\n"
"\tout vec2 vTexCoord;\n"
"\t\n"
"\t\tout vec3 vPosCS;\n"
"#line 44 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\t\n"
"\t// f = exp(-(density * dist)^n) => density = (-ln(f))^(1/n) / dist\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t#line 53 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\n"
"\t//--------------------------------------------------------------------------------------\n"
"\t// Vertex Shader\n"
"\t//--------------------------------------------------------------------------------------\n"
"\tvoid VS()\n"
"\t{ \n"
"\t\tuvec3 vuPos = UnpackVertex( pos );\n"
"\t    \n"
"\t\tbool bBorder = false;\n"
"\t\tif (vuPos.x > 512u)\n"
"\t\t{\n"
"\t\t\tvuPos.x = (vuPos.x << 2u) & ((1u << 10u) - 1u);\n"
"\t\t\tbBorder = true;\n"
"\t\t}\n"
"\t\telse if (vuPos.y > 512u)\n"
"\t\t{\n"
"\t\t\tvuPos.y = (vuPos.y << 2u) & ((1u << 10u) - 1u);\n"
"\t\t\tbBorder = true;\n"
"\t\t}\n"
"\t    \n"
"\t\tvec4 vPos = vec4(vuPos, 1.0f);\n"
"\n"
"\t\tvTexCoord = vTexCoordOffset + (vPos.xy / vec2(512u, 512u)) * vTexCoordSize;\n"
"\t    \n"
"\t\tvPos.z += 0.5f;\n"
"\t\tvPos.xyz *= vScale;\n"
"\n"
"\t\tif (bBorder)\n"
"\t\t{\n"
"\t\t\tvPos.z -= fBorderHeight;\n"
"\t\t}\n"
"\t    \n"
"\t\tvPos = mWorldView * vPos;\n"
"\t\n"
"\t\tvPosCS = vPos.xyz / vPos.w;\n"
"\t#line 89 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t\tgl_Position = mProjection * vPos;\n"
"\t}uniform float fSaturation ;\n"
"\n"
"\n"
"#line 156 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
" void main() { VS(); }"
,
"\n"
"#line 2 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\t\n"
"\n"
"#line 13 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t#line 17 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\n"
"\tuint PackVertex(uvec3 v)\n"
"\t{\n"
"\t\tuint result;\n"
"\n"
"\t\tresult = v.x;\n"
"\t\tresult |= v.y << 10u;\n"
"\t\tresult |= v.z << (2u * 10u);\n"
"/* Use this for debugging GPU decompression code\n"
"\t\tresult = v.x & ((1u << NUM_XY_BITS) - 1u);\n"
"\t\tresult |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;\n"
"\t\tresult |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);\n"
"*/\n"
"\t\treturn result;\n"
"\t}\n"
"\n"
"\tuvec3 UnpackVertex(uint v)\n"
"\t{\n"
"\t\tuvec3 result;\n"
"\n"
"\t\tresult.x = v & ((1u << 10u) - 1u);\n"
"\t\tresult.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;\n"
"\t\tresult.z = v >> (2u * 10u);\n"
"\t\treturn result;\n"
"\t}\n"
"#line 4 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tconst vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;\n"
"uniform vec3 vScale ;\n"
"uniform float fBorderHeight ;\n"
"uniform vec2 vTexCoordOffset ;\n"
"uniform vec2 vTexCoordSize ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform sampler2D txTerrain ;\n"
"\n"
"#line 35 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tin vec2 vTexCoord;\n"
"\t\n"
"\t\tin vec3 vPosCS;\n"
"\t#line 41 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\n"
"\tout vec4 vColor;\n"
"#line 96 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tvoid PS()\n"
"\t{\n"
"\t\tvColor = texture( txTerrain, vTexCoord );\n"
"\n"
"\t\n"
"\t\tfloat fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);\n"
"\t\tfFog = exp(- fFog * fFog);\n"
"\t\tvColor = fFog * vColor + (1.0f - fFog) * vFogColor;\t\n"
"\t#line 107 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t}uniform float fSaturation ;\n"
"\n"
"#line 113 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tvec3 SAT_MODIFY(vec3 clr) {\n"
"\t\tfloat mx    = max(clr.r,max(clr.g,clr.b));\n"
"\t\tfloat mn    = min(clr.r,min(clr.g,clr.b));\n"
"\t\tfloat delta = 1.0f/(mx-mn);\n"
"\t\tfloat sat   = (mx==0 ? 0.0f : 1.0f-(mn/mx))*fSaturation;\n"
"\t\tfloat val\t= mx;\n"
"\t\t\n"
"\t\tfloat f;\n"
"\t\t// hue\n"
"\t\tif (mx==mn)\t\t\t\treturn\tvec3(val,val,val);\t// achromatic case\tOK\t \n"
"\t\telse if (mx==clr.r) {\n"
"\t\t\tif (clr.g>=clr.b)\t{ f=(clr.g-clr.b)*delta;\t\treturn val*vec3(1.0f, (1.0f-(1.0f-f)*sat), (1.0f-sat)); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.g-clr.b)*delta;\treturn val*vec3(1.0f, (1.0f-sat),(1.0f-f*sat)); }\n"
"\t\t}\n"
"\t\telse if (mx==clr.g) {\n"
"\t\t\tif (clr.b>=clr.r)\t{ f=(clr.b-clr.r)*delta;\t\treturn val*vec3((1.0f-sat), 1.0f, (1.0f-(1.0f-f)*sat)); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.b-clr.r)*delta;\treturn val*vec3((1.0f-f*sat), 1.0f,(1.0f-sat)); }\n"
"\t\t}\n"
"\t\telse {\n"
"\t\t\tif (clr.r>=clr.g)\t{ f=(clr.r-clr.g)*delta;\t\treturn val*vec3((1.0f-(1.0f-f)*sat), (1.0f-sat), 1.0f); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.r-clr.g)*delta;\treturn val*vec3((1.0f-sat),(1.0f-f*sat), 1.0f); }\n"
"\t\t}\n"
"\t}\n"
"\n"
"\tvec4 SAT_MODIFY(vec4 rgba) {\n"
"\t\treturn vec4(SAT_MODIFY(rgba.xyz),rgba.w);\n"
"\t}\n"
"\t\n"
"\tvoid PSSatModify()\n"
"\t{\n"
"\t\tvColor = SAT_MODIFY(texture(txTerrain, vTexCoord));\n"
"\t\n"
"\t\tfloat fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);\n"
"\t\tfFog = exp(- fFog * fFog);\n"
"\t\tvColor = fFog * vColor + (1.0f - fFog) * vFogColor;\t\n"
"\t#line 151 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t}\n"
"\n"
"#line 157 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
" void main() { PS(); }"
,
NULL
},
// pass 1
{
"\n"
"#line 2 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\t\n"
"\n"
"#line 13 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t#line 17 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\n"
"\tuint PackVertex(uvec3 v)\n"
"\t{\n"
"\t\tuint result;\n"
"\n"
"\t\tresult = v.x;\n"
"\t\tresult |= v.y << 10u;\n"
"\t\tresult |= v.z << (2u * 10u);\n"
"/* Use this for debugging GPU decompression code\n"
"\t\tresult = v.x & ((1u << NUM_XY_BITS) - 1u);\n"
"\t\tresult |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;\n"
"\t\tresult |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);\n"
"*/\n"
"\t\treturn result;\n"
"\t}\n"
"\n"
"\tuvec3 UnpackVertex(uint v)\n"
"\t{\n"
"\t\tuvec3 result;\n"
"\n"
"\t\tresult.x = v & ((1u << 10u) - 1u);\n"
"\t\tresult.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;\n"
"\t\tresult.z = v >> (2u * 10u);\n"
"\t\treturn result;\n"
"\t}\n"
"#line 4 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tconst vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;\n"
"uniform vec3 vScale ;\n"
"uniform float fBorderHeight ;\n"
"uniform vec2 vTexCoordOffset ;\n"
"uniform vec2 vTexCoordSize ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform sampler2D txTerrain ;\n"
"\n"
"#line 22 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"#ifndef INDEXED_DRAWING\n"
"\tin uint pos;\n"
"#else\n"
"#\tdefine pos uint( gl_VertexID )\n"
"#endif\n"
"\n"
"\tout vec2 vTexCoord;\n"
"\t\n"
"\t\tout vec3 vPosCS;\n"
"#line 44 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\t\n"
"\t// f = exp(-(density * dist)^n) => density = (-ln(f))^(1/n) / dist\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t#line 53 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\n"
"\t//--------------------------------------------------------------------------------------\n"
"\t// Vertex Shader\n"
"\t//--------------------------------------------------------------------------------------\n"
"\tvoid VS()\n"
"\t{ \n"
"\t\tuvec3 vuPos = UnpackVertex( pos );\n"
"\t    \n"
"\t\tbool bBorder = false;\n"
"\t\tif (vuPos.x > 512u)\n"
"\t\t{\n"
"\t\t\tvuPos.x = (vuPos.x << 2u) & ((1u << 10u) - 1u);\n"
"\t\t\tbBorder = true;\n"
"\t\t}\n"
"\t\telse if (vuPos.y > 512u)\n"
"\t\t{\n"
"\t\t\tvuPos.y = (vuPos.y << 2u) & ((1u << 10u) - 1u);\n"
"\t\t\tbBorder = true;\n"
"\t\t}\n"
"\t    \n"
"\t\tvec4 vPos = vec4(vuPos, 1.0f);\n"
"\n"
"\t\tvTexCoord = vTexCoordOffset + (vPos.xy / vec2(512u, 512u)) * vTexCoordSize;\n"
"\t    \n"
"\t\tvPos.z += 0.5f;\n"
"\t\tvPos.xyz *= vScale;\n"
"\n"
"\t\tif (bBorder)\n"
"\t\t{\n"
"\t\t\tvPos.z -= fBorderHeight;\n"
"\t\t}\n"
"\t    \n"
"\t\tvPos = mWorldView * vPos;\n"
"\t\n"
"\t\tvPosCS = vPos.xyz / vPos.w;\n"
"\t#line 89 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t\tgl_Position = mProjection * vPos;\n"
"\t}uniform float fSaturation ;\n"
"\n"
"\n"
"#line 166 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
" void main() { VS(); }"
,
"\n"
"#line 2 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\t\n"
"\n"
"#line 13 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\t\n"
"\t#line 17 \"c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\common.gfx\"\n"
"\n"
"\t\n"
"\n"
"\tuint PackVertex(uvec3 v)\n"
"\t{\n"
"\t\tuint result;\n"
"\n"
"\t\tresult = v.x;\n"
"\t\tresult |= v.y << 10u;\n"
"\t\tresult |= v.z << (2u * 10u);\n"
"/* Use this for debugging GPU decompression code\n"
"\t\tresult = v.x & ((1u << NUM_XY_BITS) - 1u);\n"
"\t\tresult |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;\n"
"\t\tresult |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);\n"
"*/\n"
"\t\treturn result;\n"
"\t}\n"
"\n"
"\tuvec3 UnpackVertex(uint v)\n"
"\t{\n"
"\t\tuvec3 result;\n"
"\n"
"\t\tresult.x = v & ((1u << 10u) - 1u);\n"
"\t\tresult.y = (v & (((1u << 10u) - 1u) << 10u)) >> 10u;\n"
"\t\tresult.z = v >> (2u * 10u);\n"
"\t\treturn result;\n"
"\t}\n"
"#line 4 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tconst vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);uniform mat4x4 mWorldView ;\n"
"uniform vec3 vScale ;\n"
"uniform float fBorderHeight ;\n"
"uniform vec2 vTexCoordOffset ;\n"
"uniform vec2 vTexCoordSize ;\n"
"uniform mat4x4 mProjection ;\n"
"uniform sampler2D txTerrain ;\n"
"\n"
"#line 35 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tin vec2 vTexCoord;\n"
"\t\n"
"\t\tin vec3 vPosCS;\n"
"\t#line 41 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\n"
"\tout vec4 vColor;\n"
"#line 96 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tvoid PS()\n"
"\t{\n"
"\t\tvColor = texture( txTerrain, vTexCoord );\n"
"\n"
"\t\n"
"\t\tfloat fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);\n"
"\t\tfFog = exp(- fFog * fFog);\n"
"\t\tvColor = fFog * vColor + (1.0f - fFog) * vFogColor;\t\n"
"\t#line 107 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t}uniform float fSaturation ;\n"
"\n"
"#line 113 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
"\n"
"\tvec3 SAT_MODIFY(vec3 clr) {\n"
"\t\tfloat mx    = max(clr.r,max(clr.g,clr.b));\n"
"\t\tfloat mn    = min(clr.r,min(clr.g,clr.b));\n"
"\t\tfloat delta = 1.0f/(mx-mn);\n"
"\t\tfloat sat   = (mx==0 ? 0.0f : 1.0f-(mn/mx))*fSaturation;\n"
"\t\tfloat val\t= mx;\n"
"\t\t\n"
"\t\tfloat f;\n"
"\t\t// hue\n"
"\t\tif (mx==mn)\t\t\t\treturn\tvec3(val,val,val);\t// achromatic case\tOK\t \n"
"\t\telse if (mx==clr.r) {\n"
"\t\t\tif (clr.g>=clr.b)\t{ f=(clr.g-clr.b)*delta;\t\treturn val*vec3(1.0f, (1.0f-(1.0f-f)*sat), (1.0f-sat)); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.g-clr.b)*delta;\treturn val*vec3(1.0f, (1.0f-sat),(1.0f-f*sat)); }\n"
"\t\t}\n"
"\t\telse if (mx==clr.g) {\n"
"\t\t\tif (clr.b>=clr.r)\t{ f=(clr.b-clr.r)*delta;\t\treturn val*vec3((1.0f-sat), 1.0f, (1.0f-(1.0f-f)*sat)); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.b-clr.r)*delta;\treturn val*vec3((1.0f-f*sat), 1.0f,(1.0f-sat)); }\n"
"\t\t}\n"
"\t\telse {\n"
"\t\t\tif (clr.r>=clr.g)\t{ f=(clr.r-clr.g)*delta;\t\treturn val*vec3((1.0f-(1.0f-f)*sat), (1.0f-sat), 1.0f); }\n"
"\t\t\telse\t\t\t\t{ f=1.0f+(clr.r-clr.g)*delta;\treturn val*vec3((1.0f-sat),(1.0f-f*sat), 1.0f); }\n"
"\t\t}\n"
"\t}\n"
"\n"
"\tvec4 SAT_MODIFY(vec4 rgba) {\n"
"\t\treturn vec4(SAT_MODIFY(rgba.xyz),rgba.w);\n"
"\t}\n"
"\t\n"
"\tvoid PSSatModify()\n"
"\t{\n"
"\t\tvColor = SAT_MODIFY(texture(txTerrain, vTexCoord));\n"
"\t\n"
"\t\tfloat fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);\n"
"\t\tfFog = exp(- fFog * fFog);\n"
"\t\tvColor = fFog * vColor + (1.0f - fFog) * vFogColor;\t\n"
"\t#line 151 \".\\\\terrainLib\\\\tileRendering.gfx\"\n"
"\t}\n"
"\n"
"#line 167 \".\\\\\\\\terrainLib\\\\\\\\tileRendering.gfx\"\n"
" void main() { PSSatModify(); }"
,
NULL
}
};


void Renderer::checkGLError();


void tileRendering::SetupPass( unsigned pass ) const {
	if( pass >= 2 ) {
		// error?
		return;
	}
	glUseProgram( programs[ pass ] );
	checkGLError();

	switch( pass ) {

		case 0: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 6 ][ 0 ] != -1 ) {
			txTerrain.SetupPass( 0, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 158 ".\\\\terrainLib\\\\tileRendering.gfx"

		glEnable( GL_DEPTH_TEST );
			}

			return;
		}

		case 1: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 6 ][ 1 ] != -1 ) {
			txTerrain.SetupPass( 1, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 168 ".\\\\terrainLib\\\\tileRendering.gfx"

		glEnable( GL_DEPTH_TEST );
			}

			return;
		}

	}
}

void tileRendering::SetupBinding( unsigned stride, unsigned offset ) const {
	unsigned totalSize = stride;
	if( !totalSize ) {
		totalSize = 0 + 1 * 4;
	}

		glVertexAttribIPointer( 0, 1, GL_UNSIGNED_INT, totalSize, (const GLvoid*) offset );
		glEnableVertexAttribArray( 0 );
		offset += 4 * 1;


	// disable unused vertex arrays
	for( unsigned binding = 1 ; binding < MAX_NUM_BINDINGS ; binding++ ) {
		glDisableVertexAttribArray( binding );
	}

	checkGLError();
}

static const char *PrintableNameForShaderType( GLenum shaderType ) {
	switch( shaderType ) {
	case GL_VERTEX_SHADER:
		return "Vertex Shader";
	case GL_GEOMETRY_SHADER:
		return "Geometry Shader";
	case GL_FRAGMENT_SHADER:
		return "Fragment Shader";
	}
	return "Unknown Shader Type";
}

static bool CompileLog( unsigned pass, GLenum shaderType, GLuint shader ) {
	GLint infoSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoSize);

	char *message = new char[infoSize];

	GLsizei length;
	glGetShaderInfoLog(shader, infoSize, &length, message);

	if( length > 0 ) {
		printf( "Pass %i - %s Log:\n%s\n", pass, PrintableNameForShaderType( shaderType ), message );
	}

	delete[] message;

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	checkGLError();

	return compileStatus == GL_TRUE;
}

static bool LinkLog( unsigned pass, GLuint program ) {
	GLint infoSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoSize);

	char *message = new char[infoSize];

	GLsizei length;
	glGetProgramInfoLog(program, infoSize, &length, message);

	if( length > 0 ) {
		printf( "Pass %i - Linker Log:\n%s\n", pass, message );
	}

	delete[] message;

	GLint compileStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &compileStatus);
	checkGLError();

	return compileStatus == GL_TRUE;
}

bool tileRendering::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique tileRendering\n\n" );

	bool success = true;

	const char *vertexShaderSources[] = { 
		"#version 150\n"
""
		"#define VERTEX_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numVertexShaderSources = sizeof( vertexShaderSources ) / sizeof( *vertexShaderSources );

	const char *fragmentShaderSources[] = { 
		"#version 150\n"
""
		"#define FRAGMENT_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numFragmentShaderSources = sizeof( fragmentShaderSources ) / sizeof( *fragmentShaderSources );

	const char *geometryShaderSources[] = { 
		"#version 150\n"
""
		"#define GEOMETRY_SHADER\r\n",
		customCode,
		NULL
	};
	const unsigned int numGeometryShaderSources = sizeof( geometryShaderSources ) / sizeof( *geometryShaderSources );

	GLuint program;

	// initialize the program for pass 0
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[0][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 0, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[0][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 0, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "pos" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "vColor" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 0, program ) && success;

	programs[ 0 ] = program;

	// initialize the program for pass 1
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[1][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 1, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[1][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 1, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "pos" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "vColor" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 1, program ) && success;

	programs[ 1 ] = program;

	// init uniform locations

	uniformLocations[ 0][ 0 ] = glGetUniformLocation( programs[ 0 ], "mWorldView" );
uniformLocations[ 0][ 1 ] = glGetUniformLocation( programs[ 1 ], "mWorldView" );


	uniformLocations[ 1][ 0 ] = glGetUniformLocation( programs[ 0 ], "vScale" );
uniformLocations[ 1][ 1 ] = glGetUniformLocation( programs[ 1 ], "vScale" );


	uniformLocations[ 2][ 0 ] = glGetUniformLocation( programs[ 0 ], "fBorderHeight" );
uniformLocations[ 2][ 1 ] = glGetUniformLocation( programs[ 1 ], "fBorderHeight" );


	uniformLocations[ 3][ 0 ] = glGetUniformLocation( programs[ 0 ], "vTexCoordOffset" );
uniformLocations[ 3][ 1 ] = glGetUniformLocation( programs[ 1 ], "vTexCoordOffset" );


	uniformLocations[ 4][ 0 ] = glGetUniformLocation( programs[ 0 ], "vTexCoordSize" );
uniformLocations[ 4][ 1 ] = glGetUniformLocation( programs[ 1 ], "vTexCoordSize" );


	uniformLocations[ 5][ 0 ] = glGetUniformLocation( programs[ 0 ], "mProjection" );
uniformLocations[ 5][ 1 ] = glGetUniformLocation( programs[ 1 ], "mProjection" );


	uniformLocations[ 6][ 0 ] = glGetUniformLocation( programs[ 0 ], "txTerrain" );
uniformLocations[ 6][ 1 ] = glGetUniformLocation( programs[ 1 ], "txTerrain" );


	uniformLocations[ 7][ 0 ] = glGetUniformLocation( programs[ 0 ], "fSaturation" );
uniformLocations[ 7][ 1 ] = glGetUniformLocation( programs[ 1 ], "fSaturation" );


	checkGLError();

	// init the samplers
	{
		glUseProgram( programs[ 0 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 6 ][ 0 ] != -1 ) {
			glUniform1i( uniformLocations[ 6 ][ 0 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 1 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 6 ][ 1 ] != -1 ) {
			glUniform1i( uniformLocations[ 6 ][ 1 ], texUnit ); texUnit++;
		}

	}


	glUseProgram( 0 );
	checkGLError();

	return success;
}

tileRendering::~tileRendering() {
	SafeRelease();
}

void tileRendering::SafeRelease() {
	for( int i = 0 ; i < 2 ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}
