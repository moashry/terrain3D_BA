#line 1 ".\\terrainLib\\tileRendering.gfx"
#line 1 "c:\\users\\work\\documents\\project\\svn\\terrain3d\\terrain3d_gl\\terrainlib\\common.gfx"
version 150

shared {
	
	
	
	
	

	

#line 13 "c:\\users\\work\\documents\\project\\svn\\terrain3d\\terrain3d_gl\\terrainlib\\common.gfx"

	
	
	#line 17 "c:\\users\\work\\documents\\project\\svn\\terrain3d\\terrain3d_gl\\terrainlib\\common.gfx"

	

	uint PackVertex(uvec3 v)
	{
		uint result;

		result = v.x;
		result |= v.y << 10u;
		result |= v.z << (2u * 10u);
/* Use this for debugging GPU decompression code
		result = v.x & ((1u << NUM_XY_BITS) - 1u);
		result |= (v.y & ((1u << NUM_XY_BITS) - 1u)) << NUM_XY_BITS;
		result |= (v.z & ((1u << (32u - NUM_XY_BITS)) - 1u)) << (2u * NUM_XY_BITS);
*/
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
}
#line 2 ".\\terrainLib\\tileRendering.gfx"
	


shared {
	const vec4 vFogColor = vec4(56.0f / 255.0f, 112.0f / 255.0f, 168.0f / 255.0f, 1.0f);
}

uniform {
	mat4x4 mWorldView;
	vec3 vScale;
	float fBorderHeight;
	vec2 vTexCoordOffset;
	vec2 vTexCoordSize;
	
	mat4x4 mProjection;
	
	sampler2D txTerrain;
}

inputlayout { pos : UNSIGNED_INT }

vertex {
$ifndef INDEXED_DRAWING
	in uint pos;
$else
$	define pos uint( gl_VertexID )
$endif

	out vec2 vTexCoord;
	
		out vec3 vPosCS;
	#line 34 ".\\terrainLib\\tileRendering.gfx"
}

fragment {
	in vec2 vTexCoord;
	
		in vec3 vPosCS;
	#line 41 ".\\terrainLib\\tileRendering.gfx"

	out vec4 vColor;
}

vertex {
	
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
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

fragment {
	void PS()
	{
		vColor = texture( txTerrain, vTexCoord );
			
	
		float fFog = ((pow(-log(0.01f), 0.25f) / 400000.0f) * (pow(-log(0.01f), 0.25f) / 400000.0f)) * dot(vPosCS, vPosCS);
		fFog = exp(- fFog * fFog);
		vColor = fFog * vColor + (1.0f - fFog) * vFogColor;	
	#line 107 ".\\terrainLib\\tileRendering.gfx"
	}
}

uniform {
	float fSaturation;
}

fragment {
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
}

pass {
	fragDataName vColor;

	vertex { void main() { VS(); } };
	fragment { void main() { PS(); } };
	state {
		depthTest: true
	}
}

pass {
	fragDataName vColor;

	vertex { void main() { VS(); } };
	fragment { void main() { PSSatModify(); } };
	state {
		depthTest: true
	}
}
