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

#line 13 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\commonCompression.gfx"

	

	
	
	
	
	
	

	const int typeTable[6 * 3] = int[]
	(
		0, 3, 4,
		1, 2, 5,
		0, 3, 4,
		1, 2, 5,
		1, 2, 5,
		0, 3, 4
	);

	// HLSL uses row-major
	// GLSL uses column-major
	// => transpose the matrices compared to common.fx
	
	const mat2 thirdVertexMatrices[6] = mat2[](
		mat2( 0, -2, 2, 0 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 0, -2, 2, 0 ),
		mat2( 1, -1, 1, 1 ),
		mat2( 1, -1, 1, 1 )
	);

	const ivec2 dirTable[8] = ivec2[](
		ivec2(0, 1),
		ivec2(1, 1),
		ivec2(1, 0),
		ivec2(1, -1),
		ivec2(0, -1),
		ivec2(-1, -1),
		ivec2(-1, 0),
		ivec2(-1, 1)
	);
** Block 3 - uniform block (uniform declarations) **:
uniform usampler2D triangleDataTexture0 ;
uniform usampler2D triangleDataTexture1 ;

** Block 4 - for vertex shaders **:

#line 26 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

$ifndef INDEXED_DRAWING
	in uint index;
$else
$	define index uint( gl_VertexID )
$endif
** Block 5 - for vertex shaders **:

#line 35 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	out uvec4 vertexOutA;
	out uvec4 vertexOutB;
** Block 6 - for geometry shaders **:

#line 41 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	in uvec4 vertexOutA[1];
	in uvec4 vertexOutB[1];
** Block 7 - for geometry shaders **:

#line 47 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	out uvec3 vertexA;
	out uvec3 vertexB;
** Block 8 - uniform block (uniform declarations) **:
uniform uint firstStrip ;
uniform uint base ;

** Block 9 - for vertex shaders **:

#line 61 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

$ifdef INDEXED_DRAWING
	$define base 0u
$endif

	void VSDecompressStripP1()
	{
		uint realIndex = index + base;
		
		uint stripID = firstStrip + realIndex / 8u;
		uint triangleID = realIndex % 8u;
		uint column = stripID / 8u;
		uint row = triangleID * 8u + stripID % 8u;
		vertexOutA = texelFetch( triangleDataTexture0, ivec2(column, row), 0 );
		vertexOutB = texelFetch( triangleDataTexture1, ivec2(column, row), 0 );
	}
** Block 10 - for geometry shaders **:

#line 81 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	void GSDecompressStripP1()
	{	
		// for some reason gl_Position needs to be set
		gl_Position = vec4(0);
		
		// v2 v1 v3
		vertexA.rgb = vertexOutA[0].grb;

		uint a = vertexOutA[0].a & 0xFFFFFFF8u;
		if (a != 0u)
		{
			uvec2 p_ = vertexOutA[0].gb | a; // r: v2_, g: v3_
			vertexB.r = vertexOutA[0].g; // v2
			vertexB.g = vertexOutA[0].b; // v3
			vertexB.b = p_.r; // v2_
			
			EmitVertex();
			
			vertexA.r = p_.r; // v2_
			vertexA.g = vertexOutA[0].b; // v3
			vertexA.b = p_.g; // v3_
		}
		
		vertexB.rgb = vertexOutB[0].grb; // v2 v1 v3
		
		EmitVertex();
		
		a = vertexOutB[0].a & 0xFFFFFFF8u;
		if (a != 0u)
		{
			uvec2 p_ = vertexOutB[0].gb | a; // r: v2_, g: v3_
			vertexA.r = vertexOutB[0].g; // v2
			vertexA.g = vertexOutB[0].b; // v3
			vertexA.b = p_.r; // v2_
			
			vertexB.r = p_.r; // v2_
			vertexB.g = vertexOutB[0].b; // v3
			vertexB.b = p_.g; // v3_
			
			EmitVertex();
		}
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
#line 13 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\commonCompression.gfx"

	

	
	
	
	
	
	

	const int typeTable[6 * 3] = int[]
	(
		0, 3, 4,
		1, 2, 5,
		0, 3, 4,
		1, 2, 5,
		1, 2, 5,
		0, 3, 4
	);

	// HLSL uses row-major
	// GLSL uses column-major
	// => transpose the matrices compared to common.fx
	
	const mat2 thirdVertexMatrices[6] = mat2[](
		mat2( 0, -2, 2, 0 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 0, -2, 2, 0 ),
		mat2( 1, -1, 1, 1 ),
		mat2( 1, -1, 1, 1 )
	);

	const ivec2 dirTable[8] = ivec2[](
		ivec2(0, 1),
		ivec2(1, 1),
		ivec2(1, 0),
		ivec2(1, -1),
		ivec2(0, -1),
		ivec2(-1, -1),
		ivec2(-1, 0),
		ivec2(-1, 1)
	);uniform usampler2D triangleDataTexture0 ;
uniform usampler2D triangleDataTexture1 ;

#line 26 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

$ifndef INDEXED_DRAWING
	in uint index;
$else
$	define index uint( gl_VertexID )
$endif
#line 35 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	out uvec4 vertexOutA;
	out uvec4 vertexOutB;uniform uint firstStrip ;
uniform uint base ;

#line 61 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

$ifdef INDEXED_DRAWING
	$define base 0u
$endif

	void VSDecompressStripP1()
	{
		uint realIndex = index + base;
		
		uint stripID = firstStrip + realIndex / 8u;
		uint triangleID = realIndex % 8u;
		uint column = stripID / 8u;
		uint row = triangleID * 8u + stripID % 8u;
		vertexOutA = texelFetch( triangleDataTexture0, ivec2(column, row), 0 );
		vertexOutB = texelFetch( triangleDataTexture1, ivec2(column, row), 0 );
	}

#line 127 ".\\\\terrainLib\\\\decompressionStreamout.gfx"
 void main() { VSDecompressStripP1(); }

** Geometry Shader Code for Pass 0 **


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
#line 13 "c:\\\\code\\\\kirschan\\\\terrain3d_gl\\\\terrainlib\\\\commonCompression.gfx"

	

	
	
	
	
	
	

	const int typeTable[6 * 3] = int[]
	(
		0, 3, 4,
		1, 2, 5,
		0, 3, 4,
		1, 2, 5,
		1, 2, 5,
		0, 3, 4
	);

	// HLSL uses row-major
	// GLSL uses column-major
	// => transpose the matrices compared to common.fx
	
	const mat2 thirdVertexMatrices[6] = mat2[](
		mat2( 0, -2, 2, 0 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 2, -2, 2, 2 ),
		mat2( 0, -2, 2, 0 ),
		mat2( 1, -1, 1, 1 ),
		mat2( 1, -1, 1, 1 )
	);

	const ivec2 dirTable[8] = ivec2[](
		ivec2(0, 1),
		ivec2(1, 1),
		ivec2(1, 0),
		ivec2(1, -1),
		ivec2(0, -1),
		ivec2(-1, -1),
		ivec2(-1, 0),
		ivec2(-1, 1)
	);uniform usampler2D triangleDataTexture0 ;
uniform usampler2D triangleDataTexture1 ;

#line 41 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	in uvec4 vertexOutA[1];
	in uvec4 vertexOutB[1];
#line 47 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	out uvec3 vertexA;
	out uvec3 vertexB;uniform uint firstStrip ;
uniform uint base ;

#line 81 ".\\\\terrainLib\\\\decompressionStreamout.gfx"

	void GSDecompressStripP1()
	{	
		// for some reason gl_Position needs to be set
		gl_Position = vec4(0);
		
		// v2 v1 v3
		vertexA.rgb = vertexOutA[0].grb;

		uint a = vertexOutA[0].a & 0xFFFFFFF8u;
		if (a != 0u)
		{
			uvec2 p_ = vertexOutA[0].gb | a; // r: v2_, g: v3_
			vertexB.r = vertexOutA[0].g; // v2
			vertexB.g = vertexOutA[0].b; // v3
			vertexB.b = p_.r; // v2_
			
			EmitVertex();
			
			vertexA.r = p_.r; // v2_
			vertexA.g = vertexOutA[0].b; // v3
			vertexA.b = p_.g; // v3_
		}
		
		vertexB.rgb = vertexOutB[0].grb; // v2 v1 v3
		
		EmitVertex();
		
		a = vertexOutB[0].a & 0xFFFFFFF8u;
		if (a != 0u)
		{
			uvec2 p_ = vertexOutB[0].gb | a; // r: v2_, g: v3_
			vertexA.r = vertexOutB[0].g; // v2
			vertexA.g = vertexOutB[0].b; // v3
			vertexA.b = p_.r; // v2_
			
			vertexB.r = p_.r; // v2_
			vertexB.g = vertexOutB[0].b; // v3
			vertexB.b = p_.g; // v3_
			
			EmitVertex();
		}
	}

#line 128 ".\\\\terrainLib\\\\decompressionStreamout.gfx"
 layout(POINTS) in; layout(POINTS, max_vertices = 3 ) out; void main() { GSDecompressStripP1(); }


*/

#include <stdio.h>
#include <GL/glew.h>
#include "decompressionStreamout.h"

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
"#line 13 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\commonCompression.gfx\"\n"
"\n"
"\t\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\tconst int typeTable[6 * 3] = int[]\n"
"\t(\n"
"\t\t0, 3, 4,\n"
"\t\t1, 2, 5,\n"
"\t\t0, 3, 4,\n"
"\t\t1, 2, 5,\n"
"\t\t1, 2, 5,\n"
"\t\t0, 3, 4\n"
"\t);\n"
"\n"
"\t// HLSL uses row-major\n"
"\t// GLSL uses column-major\n"
"\t// => transpose the matrices compared to common.fx\n"
"\t\n"
"\tconst mat2 thirdVertexMatrices[6] = mat2[](\n"
"\t\tmat2( 0, -2, 2, 0 ),\n"
"\t\tmat2( 2, -2, 2, 2 ),\n"
"\t\tmat2( 2, -2, 2, 2 ),\n"
"\t\tmat2( 0, -2, 2, 0 ),\n"
"\t\tmat2( 1, -1, 1, 1 ),\n"
"\t\tmat2( 1, -1, 1, 1 )\n"
"\t);\n"
"\n"
"\tconst ivec2 dirTable[8] = ivec2[](\n"
"\t\tivec2(0, 1),\n"
"\t\tivec2(1, 1),\n"
"\t\tivec2(1, 0),\n"
"\t\tivec2(1, -1),\n"
"\t\tivec2(0, -1),\n"
"\t\tivec2(-1, -1),\n"
"\t\tivec2(-1, 0),\n"
"\t\tivec2(-1, 1)\n"
"\t);uniform usampler2D triangleDataTexture0 ;\n"
"uniform usampler2D triangleDataTexture1 ;\n"
"\n"
"#line 26 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"#ifndef INDEXED_DRAWING\n"
"\tin uint index;\n"
"#else\n"
"#\tdefine index uint( gl_VertexID )\n"
"#endif\n"
"#line 35 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"\tout uvec4 vertexOutA;\n"
"\tout uvec4 vertexOutB;uniform uint firstStrip ;\n"
"uniform uint base ;\n"
"\n"
"#line 61 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"#ifdef INDEXED_DRAWING\n"
"\t#define base 0u\n"
"#endif\n"
"\n"
"\tvoid VSDecompressStripP1()\n"
"\t{\n"
"\t\tuint realIndex = index + base;\n"
"\t\t\n"
"\t\tuint stripID = firstStrip + realIndex / 8u;\n"
"\t\tuint triangleID = realIndex % 8u;\n"
"\t\tuint column = stripID / 8u;\n"
"\t\tuint row = triangleID * 8u + stripID % 8u;\n"
"\t\tvertexOutA = texelFetch( triangleDataTexture0, ivec2(column, row), 0 );\n"
"\t\tvertexOutB = texelFetch( triangleDataTexture1, ivec2(column, row), 0 );\n"
"\t}\n"
"\n"
"#line 127 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
" void main() { VSDecompressStripP1(); }"
,
NULL
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
"#line 13 \"c:\\\\\\\\code\\\\\\\\kirschan\\\\\\\\terrain3d_gl\\\\\\\\terrainlib\\\\\\\\commonCompression.gfx\"\n"
"\n"
"\t\n"
"\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\t\n"
"\n"
"\tconst int typeTable[6 * 3] = int[]\n"
"\t(\n"
"\t\t0, 3, 4,\n"
"\t\t1, 2, 5,\n"
"\t\t0, 3, 4,\n"
"\t\t1, 2, 5,\n"
"\t\t1, 2, 5,\n"
"\t\t0, 3, 4\n"
"\t);\n"
"\n"
"\t// HLSL uses row-major\n"
"\t// GLSL uses column-major\n"
"\t// => transpose the matrices compared to common.fx\n"
"\t\n"
"\tconst mat2 thirdVertexMatrices[6] = mat2[](\n"
"\t\tmat2( 0, -2, 2, 0 ),\n"
"\t\tmat2( 2, -2, 2, 2 ),\n"
"\t\tmat2( 2, -2, 2, 2 ),\n"
"\t\tmat2( 0, -2, 2, 0 ),\n"
"\t\tmat2( 1, -1, 1, 1 ),\n"
"\t\tmat2( 1, -1, 1, 1 )\n"
"\t);\n"
"\n"
"\tconst ivec2 dirTable[8] = ivec2[](\n"
"\t\tivec2(0, 1),\n"
"\t\tivec2(1, 1),\n"
"\t\tivec2(1, 0),\n"
"\t\tivec2(1, -1),\n"
"\t\tivec2(0, -1),\n"
"\t\tivec2(-1, -1),\n"
"\t\tivec2(-1, 0),\n"
"\t\tivec2(-1, 1)\n"
"\t);uniform usampler2D triangleDataTexture0 ;\n"
"uniform usampler2D triangleDataTexture1 ;\n"
"\n"
"#line 41 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"\tin uvec4 vertexOutA[1];\n"
"\tin uvec4 vertexOutB[1];\n"
"#line 47 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"\tout uvec3 vertexA;\n"
"\tout uvec3 vertexB;uniform uint firstStrip ;\n"
"uniform uint base ;\n"
"\n"
"#line 81 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
"\n"
"\tvoid GSDecompressStripP1()\n"
"\t{\t\n"
"\t\t// for some reason gl_Position needs to be set\n"
"\t\tgl_Position = vec4(0);\n"
"\t\t\n"
"\t\t// v2 v1 v3\n"
"\t\tvertexA.rgb = vertexOutA[0].grb;\n"
"\n"
"\t\tuint a = vertexOutA[0].a & 0xFFFFFFF8u;\n"
"\t\tif (a != 0u)\n"
"\t\t{\n"
"\t\t\tuvec2 p_ = vertexOutA[0].gb | a; // r: v2_, g: v3_\n"
"\t\t\tvertexB.r = vertexOutA[0].g; // v2\n"
"\t\t\tvertexB.g = vertexOutA[0].b; // v3\n"
"\t\t\tvertexB.b = p_.r; // v2_\n"
"\t\t\t\n"
"\t\t\tEmitVertex();\n"
"\t\t\t\n"
"\t\t\tvertexA.r = p_.r; // v2_\n"
"\t\t\tvertexA.g = vertexOutA[0].b; // v3\n"
"\t\t\tvertexA.b = p_.g; // v3_\n"
"\t\t}\n"
"\t\t\n"
"\t\tvertexB.rgb = vertexOutB[0].grb; // v2 v1 v3\n"
"\t\t\n"
"\t\tEmitVertex();\n"
"\t\t\n"
"\t\ta = vertexOutB[0].a & 0xFFFFFFF8u;\n"
"\t\tif (a != 0u)\n"
"\t\t{\n"
"\t\t\tuvec2 p_ = vertexOutB[0].gb | a; // r: v2_, g: v3_\n"
"\t\t\tvertexA.r = vertexOutB[0].g; // v2\n"
"\t\t\tvertexA.g = vertexOutB[0].b; // v3\n"
"\t\t\tvertexA.b = p_.r; // v2_\n"
"\t\t\t\n"
"\t\t\tvertexB.r = p_.r; // v2_\n"
"\t\t\tvertexB.g = vertexOutB[0].b; // v3\n"
"\t\t\tvertexB.b = p_.g; // v3_\n"
"\t\t\t\n"
"\t\t\tEmitVertex();\n"
"\t\t}\n"
"\t}\n"
"\n"
"#line 128 \".\\\\\\\\terrainLib\\\\\\\\decompressionStreamout.gfx\"\n"
" layout(POINTS) in; layout(POINTS, max_vertices = 3 ) out; void main() { GSDecompressStripP1(); }"
}
};


void Renderer::checkGLError();


void decompressionStreamout::SetupPass( unsigned pass ) const {
	if( pass >= 1 ) {
		// error?
		return;
	}
	glUseProgram( programs[ pass ] );
	checkGLError();

	switch( pass ) {

		case 0: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 0 ] != -1 ) {
			triangleDataTexture0.SetupPass( 0, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 0 ] != -1 ) {
			triangleDataTexture1.SetupPass( 0, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			return;
		}

	}
}

void decompressionStreamout::SetupBinding( unsigned stride, unsigned offset ) const {
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

bool decompressionStreamout::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique decompressionStreamout\n\n" );

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
		GLuint geometryShader = glCreateShader( GL_GEOMETRY_SHADER_ARB );
		geometryShaderSources[ numGeometryShaderSources - 1 ] = codeForPasses[ 0][2];
		glShaderSource( geometryShader, numGeometryShaderSources, geometryShaderSources, NULL );
		glCompileShader( geometryShader );
		checkGLError();

		success = CompileLog( 0, GL_GEOMETRY_SHADER_ARB, geometryShader ) && success;

		glAttachShader( program, geometryShader );
		glDeleteShader( geometryShader );
		checkGLError();
	}

	// link the attributes
	glBindAttribLocation( program, 0, "index" );
	checkGLError();

	// link transform feedback varyings
	const char *varyings[] = { "vertexA", "vertexB" };
	const unsigned numVaryings = sizeof( varyings ) / sizeof( *varyings );
	glTransformFeedbackVaryings( program, numVaryings, varyings, GL_INTERLEAVED_ATTRIBS );
	checkGLError();
	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 0, program ) && success;

	programs[ 0 ] = program;

	// init uniform locations

	uniformLocations[ 0][ 0 ] = glGetUniformLocation( programs[ 0 ], "triangleDataTexture0" );


	uniformLocations[ 1][ 0 ] = glGetUniformLocation( programs[ 0 ], "triangleDataTexture1" );


	uniformLocations[ 2][ 0 ] = glGetUniformLocation( programs[ 0 ], "firstStrip" );


	uniformLocations[ 3][ 0 ] = glGetUniformLocation( programs[ 0 ], "base" );


	checkGLError();

	// init the samplers
	{
		glUseProgram( programs[ 0 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 0 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 0 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 0 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 0 ], texUnit ); texUnit++;
		}

	}


	glUseProgram( 0 );
	checkGLError();

	return success;
}

decompressionStreamout::~decompressionStreamout() {
	SafeRelease();
}

void decompressionStreamout::SafeRelease() {
	for( int i = 0 ; i < 1 ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}