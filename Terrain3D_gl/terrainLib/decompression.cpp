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
uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

** Block 4 - for vertex shaders **:

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
** Block 5 - for vertex shaders **:

#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
** Block 6 - for fragment shaders **:

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
** Block 7 - for fragment shaders **:

#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
** Block 8 - for vertex shaders **:

#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}
** Block 9 - for fragment shaders **:

#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 224 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 224 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle0(); }


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 225 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 225 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 1u); }


Pass 2:

** Vertex Shader Code for Pass 2 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 226 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 2 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 226 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 2u); }


Pass 3:

** Vertex Shader Code for Pass 3 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 227 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 3 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 227 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 3u); }


Pass 4:

** Vertex Shader Code for Pass 4 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 228 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 4 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 228 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 4u); }


Pass 5:

** Vertex Shader Code for Pass 5 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 229 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 5 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 229 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 5u); }


Pass 6:

** Vertex Shader Code for Pass 6 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 230 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 6 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 230 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 6u); }


Pass 7:

** Vertex Shader Code for Pass 7 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 231 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 7 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 231 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 7u); }


Pass 8:

** Vertex Shader Code for Pass 8 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 232 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 8 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 232 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 8u); }


Pass 9:

** Vertex Shader Code for Pass 9 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 233 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 9 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 233 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 9u); }


Pass 10:

** Vertex Shader Code for Pass 10 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 234 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 10 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 234 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 10u); }


Pass 11:

** Vertex Shader Code for Pass 11 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 235 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 11 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 235 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 11u); }


Pass 12:

** Vertex Shader Code for Pass 12 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 236 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 12 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 236 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 12u); }


Pass 13:

** Vertex Shader Code for Pass 13 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 237 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 13 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 237 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 13u); }


Pass 14:

** Vertex Shader Code for Pass 14 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 238 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 14 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 238 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(8u, 14u); }


Pass 15:

** Vertex Shader Code for Pass 15 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 31 ".\\\\terrainLib\\\\decompression.gfx"

	in vec2 vPosIn;
	in uint offsetIn;
	in uint numBitsIn;
#line 38 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_Position
	flat out uint offset;
	flat out uint numBits;
#line 59 ".\\\\terrainLib\\\\decompression.gfx"

	void VSDecompressGeometry()
	{
		vPos = vec4(vPosIn, 0.5f, 1.0f);
		offset = offsetIn;
		numBits = numBitsIn;
	}

#line 239 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { VSDecompressGeometry(); }

** Fragment Shader Code for Pass 15 **


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
	);uniform usamplerBuffer stripHeadersBuffer ;
uniform usamplerBuffer stripDataBuffer ;
uniform usampler2D texPingPong ;

#line 45 ".\\\\terrainLib\\\\decompression.gfx"

	$define vPos gl_FragCoord
	flat in uint offset;
	flat in uint numBits;
#line 52 ".\\\\terrainLib\\\\decompression.gfx"

	out uvec4 result;
#line 71 ".\\\\terrainLib\\\\decompression.gfx"

	uint ReadData(uint index, uint offset, uint numBits)
	{
		uint bitPos = offset + index * numBits;
		uint readPos = bitPos >> 4u;
		uint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];
		uint relBitPos = bitPos & 0xFu;
		return (data >> relBitPos) & ( (1u << numBits) - 1u );
	}

	void PSTriangle0()
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		
		uint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];
		
		ivec3 v1, v2, v3;
		
		v1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit
		v1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit
		
		uint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit
		uint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit
		uint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit
		uint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit
		uint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit
		
		uint length = 1u << logLength;
		v2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];
		
		uint dindex1, dindex2;
		
		if (iindex == 0u)
		{
			dindex1 = dindex;
			dindex2 = 1u;
		}
		else
		{
			dindex2 = dindex;
			dindex1 = 1u;
		}
		
		uint index = 2u + stripID * 16u;
		v1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );
		v2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );
		
		uint data = ReadData(index, offset, numBits);
		uint ABC = data & 0x3u;
		uint type = (ABC << 1u) | LR;

		v3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);
		v3.z = int( data >> 2u );
		
		uint p1 = PackVertex( uvec3( v1 ) );
		uint p2 = PackVertex( uvec3( v2 ) );
		uint p3 = PackVertex( uvec3( v3 ) );
		
		uint a = 0u;
		if( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	
		result = uvec4(p1, p2, p3, a | type);
	}


	void PSTriangle(uint rowOffset, uint stripDataOffset)
	{
		uint column = uint( vPos.x );
		uint row = uint( vPos.y );
		uint stripID = column * 8u + row % 8u;
		uvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);
		uint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);
		uint ABC = data & 0x3u;
		uint p1, p2;
		if ((vPreviousTriangle.a & 0x1u) == 0u) // L
		{
			p1 = vPreviousTriangle.r; // v1
			p2 = vPreviousTriangle.b; // v3
		}
		else
		{
			p1 = vPreviousTriangle.b; // v3
			p2 = vPreviousTriangle.g; // v2
		}
		int type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];
		ivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);
		ivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));
		uint p3 = PackVertex( uvec3( v3 ));
		
		uint a = 0u;
		if (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)
		{
			a = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));
		}
		else if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)
		{
			a = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;
		}
	/ *
		if (v2.x == 0)
		{
			if (v3.x == 0)
			{
				a = 0x3 << (NUM_XY_BITS - 2);
			}
		}
		else if (v2.x == TILE_SIZE)
		{
			if (v3.x == TILE_SIZE)
			{
				a = 0x3 << (NUM_XY_BITS - 3);
			}
		}
		if (v2.y == 0)
		{
			if (v3.y == 0)
			{
				a = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;
			}
		}
		else if (v2.y == TILE_SIZE)
		{
			if (v3.y == TILE_SIZE)
			{
				a = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;
			}
		}
	* /	
		result = uvec4(p1, p2, p3, a | uint( type ));
	}

#line 239 ".\\\\terrainLib\\\\decompression.gfx"
 void main() { PSTriangle(0u, 15u ); }


*/

#include <stdio.h>
#include <GL/glew.h>
#include "decompression.h"

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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 224 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 224 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle0(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 225 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 225 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 1u); }"
,
NULL
},
// pass 2
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 226 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 226 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 2u); }"
,
NULL
},
// pass 3
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 227 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 227 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 3u); }"
,
NULL
},
// pass 4
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 228 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 228 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 4u); }"
,
NULL
},
// pass 5
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 229 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 229 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 5u); }"
,
NULL
},
// pass 6
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 230 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 230 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 6u); }"
,
NULL
},
// pass 7
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 231 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 231 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 7u); }"
,
NULL
},
// pass 8
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 232 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 232 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 8u); }"
,
NULL
},
// pass 9
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 233 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 233 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 9u); }"
,
NULL
},
// pass 10
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 234 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 234 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 10u); }"
,
NULL
},
// pass 11
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 235 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 235 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 11u); }"
,
NULL
},
// pass 12
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 236 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 236 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 12u); }"
,
NULL
},
// pass 13
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 237 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 237 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 13u); }"
,
NULL
},
// pass 14
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 238 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 238 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(8u, 14u); }"
,
NULL
},
// pass 15
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 31 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tin vec2 vPosIn;\n"
"\tin uint offsetIn;\n"
"\tin uint numBitsIn;\n"
"#line 38 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_Position\n"
"\tflat out uint offset;\n"
"\tflat out uint numBits;\n"
"#line 59 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tvoid VSDecompressGeometry()\n"
"\t{\n"
"\t\tvPos = vec4(vPosIn, 0.5f, 1.0f);\n"
"\t\toffset = offsetIn;\n"
"\t\tnumBits = numBitsIn;\n"
"\t}\n"
"\n"
"#line 239 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { VSDecompressGeometry(); }"
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
"\t);uniform usamplerBuffer stripHeadersBuffer ;\n"
"uniform usamplerBuffer stripDataBuffer ;\n"
"uniform usampler2D texPingPong ;\n"
"\n"
"#line 45 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\t#define vPos gl_FragCoord\n"
"\tflat in uint offset;\n"
"\tflat in uint numBits;\n"
"#line 52 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tout uvec4 result;\n"
"#line 71 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
"\n"
"\tuint ReadData(uint index, uint offset, uint numBits)\n"
"\t{\n"
"\t\tuint bitPos = offset + index * numBits;\n"
"\t\tuint readPos = bitPos >> 4u;\n"
"\t\tuint data = (texelFetch(stripDataBuffer, int(readPos + 1u))[0] << 16u) | texelFetch(stripDataBuffer, int( readPos ))[0];\n"
"\t\tuint relBitPos = bitPos & 0xFu;\n"
"\t\treturn (data >> relBitPos) & ( (1u << numBits) - 1u );\n"
"\t}\n"
"\n"
"\tvoid PSTriangle0()\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\t\n"
"\t\tuint stripHeader = texelFetch( stripHeadersBuffer, int( stripID ) )[0];\n"
"\t\t\n"
"\t\tivec3 v1, v2, v3;\n"
"\t\t\n"
"\t\tv1.x = int( stripHeader & ((1u << (10u - 1u)) - 1u) ); // 9 Bit\n"
"\t\tv1.y = int( (stripHeader & (((1u << (10u - 1u)) - 1u) << 9u)) >> 9u ); // 9 Bit\n"
"\t\t\n"
"\t\tuint logLength = (stripHeader & 0x003C0000u) >> 18u; // 4 Bit\n"
"\t\tuint dir =(stripHeader & 0x01C00000u) >> 22u; // 3 Bit\n"
"\t\tuint LR = (stripHeader & 0x02000000u) >> 25u; // 1 Bit\n"
"\t\tuint dindex = ((stripHeader & 0x1C000000u) >> 26u) + 2u; // 3 Bit\n"
"\t\tuint iindex = (stripHeader & 0x20000000u) >> 29u; // 1 Bit\n"
"\t\t\n"
"\t\tuint length = 1u << logLength;\n"
"\t\tv2.xy = v1.xy + int( length ) * dirTable[ int( dir ) ];\n"
"\t\t\n"
"\t\tuint dindex1, dindex2;\n"
"\t\t\n"
"\t\tif (iindex == 0u)\n"
"\t\t{\n"
"\t\t\tdindex1 = dindex;\n"
"\t\t\tdindex2 = 1u;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tdindex2 = dindex;\n"
"\t\t\tdindex1 = 1u;\n"
"\t\t}\n"
"\t\t\n"
"\t\tuint index = 2u + stripID * 16u;\n"
"\t\tv1.z = int( ReadData(index - dindex1, offset, numBits) >> 2u );\n"
"\t\tv2.z = int( ReadData(index - dindex2, offset, numBits) >> 2u );\n"
"\t\t\n"
"\t\tuint data = ReadData(index, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint type = (ABC << 1u) | LR;\n"
"\n"
"\t\tv3.xy = v1.xy + (ivec2(thirdVertexMatrices[int( type )] * ( v2.xy - v1.xy )) >> 1);\n"
"\t\tv3.z = int( data >> 2u );\n"
"\t\t\n"
"\t\tuint p1 = PackVertex( uvec3( v1 ) );\n"
"\t\tuint p2 = PackVertex( uvec3( v2 ) );\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ) );\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif( ( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x )\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if( (uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y )\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | type);\n"
"\t}\n"
"\n"
"\n"
"\tvoid PSTriangle(uint rowOffset, uint stripDataOffset)\n"
"\t{\n"
"\t\tuint column = uint( vPos.x );\n"
"\t\tuint row = uint( vPos.y );\n"
"\t\tuint stripID = column * 8u + row % 8u;\n"
"\t\tuvec4 vPreviousTriangle = texelFetch( texPingPong, ivec2(column, row - rowOffset), 0);\n"
"\t\tuint data = ReadData(2u + stripDataOffset + stripID * 16u, offset, numBits);\n"
"\t\tuint ABC = data & 0x3u;\n"
"\t\tuint p1, p2;\n"
"\t\tif ((vPreviousTriangle.a & 0x1u) == 0u) // L\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.r; // v1\n"
"\t\t\tp2 = vPreviousTriangle.b; // v3\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tp1 = vPreviousTriangle.b; // v3\n"
"\t\t\tp2 = vPreviousTriangle.g; // v2\n"
"\t\t}\n"
"\t\tint type = typeTable[int( (vPreviousTriangle.a & 0x7u) * 3u + ABC )];\n"
"\t\tivec2 v1 = ivec2(p1 & ((1u << 10u) - 1u), (p1 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec2 v2 = ivec2(p2 & ((1u << 10u) - 1u), (p2 & (((1u << 10u) - 1u) << 10u)) >> 10u);\n"
"\t\tivec3 v3 = ivec3(v1 + ( ivec2( thirdVertexMatrices[ type ] * (v2 - v1) ) >> 1 ), int(data >> 2u));\n"
"\t\tuint p3 = PackVertex( uvec3( v3 ));\n"
"\t\t\n"
"\t\tuint a = 0u;\n"
"\t\tif (( uint( v2.x ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.x == v3.x)\n"
"\t\t{\n"
"\t\t\ta = (uint( v2.x ) >> 2u) | (0x3u << (10u - 2u));\n"
"\t\t}\n"
"\t\telse if ((uint( v2.y ) & ((1u << (10u - 1u)) - 1u)) == 0u && v2.y == v3.y)\n"
"\t\t{\n"
"\t\t\ta = ((uint( v2.y ) >> 2u) | (0x3u << (10u - 2u))) << 10u;\n"
"\t\t}\n"
"\t/*\n"
"\t\tif (v2.x == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.x == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 2);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.x == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.x == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = 0x3 << (NUM_XY_BITS - 3);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tif (v2.y == 0)\n"
"\t\t{\n"
"\t\t\tif (v3.y == 0)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 2)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\telse if (v2.y == TILE_SIZE)\n"
"\t\t{\n"
"\t\t\tif (v3.y == TILE_SIZE)\n"
"\t\t\t{\n"
"\t\t\t\ta = (0x3 << (NUM_XY_BITS - 3)) << NUM_XY_BITS;\n"
"\t\t\t}\n"
"\t\t}\n"
"\t*/\t\n"
"\t\tresult = uvec4(p1, p2, p3, a | uint( type ));\n"
"\t}\n"
"\n"
"#line 239 \".\\\\\\\\terrainLib\\\\\\\\decompression.gfx\"\n"
" void main() { PSTriangle(0u, 15u ); }"
,
NULL
}
};


void Renderer::checkGLError();


void decompression::SetupPass( unsigned pass ) const {
	if( pass >= 16 ) {
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
			stripHeadersBuffer.SetupPass( 0, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 0 ] != -1 ) {
			stripDataBuffer.SetupPass( 0, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 0 ] != -1 ) {
			texPingPong.SetupPass( 0, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 224 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 1: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 1 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 1, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 1 ] != -1 ) {
			stripDataBuffer.SetupPass( 1, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 1 ] != -1 ) {
			texPingPong.SetupPass( 1, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 225 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 2: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 2 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 2, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 2 ] != -1 ) {
			stripDataBuffer.SetupPass( 2, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 2 ] != -1 ) {
			texPingPong.SetupPass( 2, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 226 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 3: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 3 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 3, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 3 ] != -1 ) {
			stripDataBuffer.SetupPass( 3, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 3 ] != -1 ) {
			texPingPong.SetupPass( 3, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 227 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 4: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 4 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 4, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 4 ] != -1 ) {
			stripDataBuffer.SetupPass( 4, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 4 ] != -1 ) {
			texPingPong.SetupPass( 4, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 228 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 5: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 5 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 5, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 5 ] != -1 ) {
			stripDataBuffer.SetupPass( 5, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 5 ] != -1 ) {
			texPingPong.SetupPass( 5, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 229 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 6: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 6 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 6, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 6 ] != -1 ) {
			stripDataBuffer.SetupPass( 6, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 6 ] != -1 ) {
			texPingPong.SetupPass( 6, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 230 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 7: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 7 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 7, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 7 ] != -1 ) {
			stripDataBuffer.SetupPass( 7, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 7 ] != -1 ) {
			texPingPong.SetupPass( 7, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 231 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 8: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 8 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 8, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 8 ] != -1 ) {
			stripDataBuffer.SetupPass( 8, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 8 ] != -1 ) {
			texPingPong.SetupPass( 8, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 232 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 9: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 9 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 9, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 9 ] != -1 ) {
			stripDataBuffer.SetupPass( 9, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 9 ] != -1 ) {
			texPingPong.SetupPass( 9, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 233 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 10: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 10 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 10, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 10 ] != -1 ) {
			stripDataBuffer.SetupPass( 10, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 10 ] != -1 ) {
			texPingPong.SetupPass( 10, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 234 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 11: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 11 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 11, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 11 ] != -1 ) {
			stripDataBuffer.SetupPass( 11, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 11 ] != -1 ) {
			texPingPong.SetupPass( 11, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 235 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 12: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 12 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 12, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 12 ] != -1 ) {
			stripDataBuffer.SetupPass( 12, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 12 ] != -1 ) {
			texPingPong.SetupPass( 12, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 236 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 13: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 13 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 13, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 13 ] != -1 ) {
			stripDataBuffer.SetupPass( 13, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 13 ] != -1 ) {
			texPingPong.SetupPass( 13, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 237 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 14: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 14 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 14, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 14 ] != -1 ) {
			stripDataBuffer.SetupPass( 14, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 14 ] != -1 ) {
			texPingPong.SetupPass( 14, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 238 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

		case 15: {

			// set the texture bindings
			GLuint texUnit = 0;
			
		if( uniformLocations[ 0 ][ 15 ] != -1 ) {
			stripHeadersBuffer.SetupPass( 15, texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 15 ] != -1 ) {
			stripDataBuffer.SetupPass( 15, texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 15 ] != -1 ) {
			texPingPong.SetupPass( 15, texUnit ); texUnit++;
		}


			// disable unused texture units
			for( ; texUnit < MAX_NUM_TEXTURE_UNITS ; texUnit++ ) {
				Sampler::DisableTextureUnit( texUnit );
			}

			checkGLError();


			{
				
#line 239 ".\\\\terrainLib\\\\decompression.gfx"
 glDisable( GL_DEPTH_TEST );
			}

			return;
		}

	}
}

void decompression::SetupBinding( unsigned stride, unsigned offset ) const {
	unsigned totalSize = stride;
	if( !totalSize ) {
		totalSize = 0 + 2 * 4 + 1 * 4 + 1 * 4;
	}

		glVertexAttribPointer( 0, 2, GL_FLOAT, false, totalSize, (const GLvoid*) offset );
		glEnableVertexAttribArray( 0 );
		offset += 4 * 2;

		glVertexAttribIPointer( 1, 1, GL_UNSIGNED_INT, totalSize, (const GLvoid*) offset );
		glEnableVertexAttribArray( 1 );
		offset += 4 * 1;

		glVertexAttribIPointer( 2, 1, GL_UNSIGNED_INT, totalSize, (const GLvoid*) offset );
		glEnableVertexAttribArray( 2 );
		offset += 4 * 1;


	// disable unused vertex arrays
	for( unsigned binding = 3 ; binding < MAX_NUM_BINDINGS ; binding++ ) {
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

bool decompression::Create( const char *customCode /* = NULL */ ) {
	// a bit of an hack, if customCode is NULL, set it to an empty string instead
	if( !customCode ) {
		customCode = "";
	}

	printf( "Compiling technique decompression\n\n" );

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
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

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
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 1, program ) && success;

	programs[ 1 ] = program;

	// initialize the program for pass 2
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[2][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 2, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[2][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 2, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 2, program ) && success;

	programs[ 2 ] = program;

	// initialize the program for pass 3
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[3][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 3, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[3][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 3, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 3, program ) && success;

	programs[ 3 ] = program;

	// initialize the program for pass 4
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[4][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 4, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[4][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 4, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 4, program ) && success;

	programs[ 4 ] = program;

	// initialize the program for pass 5
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[5][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 5, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[5][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 5, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 5, program ) && success;

	programs[ 5 ] = program;

	// initialize the program for pass 6
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[6][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 6, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[6][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 6, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 6, program ) && success;

	programs[ 6 ] = program;

	// initialize the program for pass 7
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[7][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 7, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[7][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 7, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 7, program ) && success;

	programs[ 7 ] = program;

	// initialize the program for pass 8
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[8][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 8, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[8][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 8, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 8, program ) && success;

	programs[ 8 ] = program;

	// initialize the program for pass 9
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[9][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 9, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[9][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 9, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 9, program ) && success;

	programs[ 9 ] = program;

	// initialize the program for pass 10
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[10][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 10, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[10][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 10, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 10, program ) && success;

	programs[ 10 ] = program;

	// initialize the program for pass 11
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[11][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 11, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[11][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 11, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 11, program ) && success;

	programs[ 11 ] = program;

	// initialize the program for pass 12
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[12][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 12, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[12][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 12, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 12, program ) && success;

	programs[ 12 ] = program;

	// initialize the program for pass 13
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[13][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 13, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[13][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 13, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 13, program ) && success;

	programs[ 13 ] = program;

	// initialize the program for pass 14
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[14][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 14, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[14][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 14, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 14, program ) && success;

	programs[ 14 ] = program;

	// initialize the program for pass 15
	program = glCreateProgram();
	checkGLError();

	{
		GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
		vertexShaderSources[ numVertexShaderSources - 1 ] = codeForPasses[15][0];
		glShaderSource( vertexShader, numVertexShaderSources, vertexShaderSources, NULL );
		glCompileShader( vertexShader );
		checkGLError();

		success = CompileLog( 15, GL_VERTEX_SHADER, vertexShader ) && success;

		glAttachShader( program, vertexShader );
		glDeleteShader( vertexShader );
		checkGLError();
	}
	{
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		fragmentShaderSources[ numFragmentShaderSources - 1 ] = codeForPasses[15][1];
		glShaderSource( fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL );
		glCompileShader( fragmentShader );
		checkGLError();

		success = CompileLog( 15, GL_FRAGMENT_SHADER, fragmentShader ) && success;

		glAttachShader( program, fragmentShader );
		glDeleteShader( fragmentShader );
		checkGLError();
	}
	

	// link the attributes
	glBindAttribLocation( program, 0, "vPosIn" );
	checkGLError();
	glBindAttribLocation( program, 1, "offsetIn" );
	checkGLError();
	glBindAttribLocation( program, 2, "numBitsIn" );
	checkGLError();

	// link the frag data name
	glBindFragDataLocation( program, 0, "result" );

	glLinkProgram( program );
	checkGLError();
	success = LinkLog( 15, program ) && success;

	programs[ 15 ] = program;

	// init uniform locations

	uniformLocations[ 0][ 0 ] = glGetUniformLocation( programs[ 0 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 1 ] = glGetUniformLocation( programs[ 1 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 2 ] = glGetUniformLocation( programs[ 2 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 3 ] = glGetUniformLocation( programs[ 3 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 4 ] = glGetUniformLocation( programs[ 4 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 5 ] = glGetUniformLocation( programs[ 5 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 6 ] = glGetUniformLocation( programs[ 6 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 7 ] = glGetUniformLocation( programs[ 7 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 8 ] = glGetUniformLocation( programs[ 8 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 9 ] = glGetUniformLocation( programs[ 9 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 10 ] = glGetUniformLocation( programs[ 10 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 11 ] = glGetUniformLocation( programs[ 11 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 12 ] = glGetUniformLocation( programs[ 12 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 13 ] = glGetUniformLocation( programs[ 13 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 14 ] = glGetUniformLocation( programs[ 14 ], "stripHeadersBuffer" );
uniformLocations[ 0][ 15 ] = glGetUniformLocation( programs[ 15 ], "stripHeadersBuffer" );


	uniformLocations[ 1][ 0 ] = glGetUniformLocation( programs[ 0 ], "stripDataBuffer" );
uniformLocations[ 1][ 1 ] = glGetUniformLocation( programs[ 1 ], "stripDataBuffer" );
uniformLocations[ 1][ 2 ] = glGetUniformLocation( programs[ 2 ], "stripDataBuffer" );
uniformLocations[ 1][ 3 ] = glGetUniformLocation( programs[ 3 ], "stripDataBuffer" );
uniformLocations[ 1][ 4 ] = glGetUniformLocation( programs[ 4 ], "stripDataBuffer" );
uniformLocations[ 1][ 5 ] = glGetUniformLocation( programs[ 5 ], "stripDataBuffer" );
uniformLocations[ 1][ 6 ] = glGetUniformLocation( programs[ 6 ], "stripDataBuffer" );
uniformLocations[ 1][ 7 ] = glGetUniformLocation( programs[ 7 ], "stripDataBuffer" );
uniformLocations[ 1][ 8 ] = glGetUniformLocation( programs[ 8 ], "stripDataBuffer" );
uniformLocations[ 1][ 9 ] = glGetUniformLocation( programs[ 9 ], "stripDataBuffer" );
uniformLocations[ 1][ 10 ] = glGetUniformLocation( programs[ 10 ], "stripDataBuffer" );
uniformLocations[ 1][ 11 ] = glGetUniformLocation( programs[ 11 ], "stripDataBuffer" );
uniformLocations[ 1][ 12 ] = glGetUniformLocation( programs[ 12 ], "stripDataBuffer" );
uniformLocations[ 1][ 13 ] = glGetUniformLocation( programs[ 13 ], "stripDataBuffer" );
uniformLocations[ 1][ 14 ] = glGetUniformLocation( programs[ 14 ], "stripDataBuffer" );
uniformLocations[ 1][ 15 ] = glGetUniformLocation( programs[ 15 ], "stripDataBuffer" );


	uniformLocations[ 2][ 0 ] = glGetUniformLocation( programs[ 0 ], "texPingPong" );
uniformLocations[ 2][ 1 ] = glGetUniformLocation( programs[ 1 ], "texPingPong" );
uniformLocations[ 2][ 2 ] = glGetUniformLocation( programs[ 2 ], "texPingPong" );
uniformLocations[ 2][ 3 ] = glGetUniformLocation( programs[ 3 ], "texPingPong" );
uniformLocations[ 2][ 4 ] = glGetUniformLocation( programs[ 4 ], "texPingPong" );
uniformLocations[ 2][ 5 ] = glGetUniformLocation( programs[ 5 ], "texPingPong" );
uniformLocations[ 2][ 6 ] = glGetUniformLocation( programs[ 6 ], "texPingPong" );
uniformLocations[ 2][ 7 ] = glGetUniformLocation( programs[ 7 ], "texPingPong" );
uniformLocations[ 2][ 8 ] = glGetUniformLocation( programs[ 8 ], "texPingPong" );
uniformLocations[ 2][ 9 ] = glGetUniformLocation( programs[ 9 ], "texPingPong" );
uniformLocations[ 2][ 10 ] = glGetUniformLocation( programs[ 10 ], "texPingPong" );
uniformLocations[ 2][ 11 ] = glGetUniformLocation( programs[ 11 ], "texPingPong" );
uniformLocations[ 2][ 12 ] = glGetUniformLocation( programs[ 12 ], "texPingPong" );
uniformLocations[ 2][ 13 ] = glGetUniformLocation( programs[ 13 ], "texPingPong" );
uniformLocations[ 2][ 14 ] = glGetUniformLocation( programs[ 14 ], "texPingPong" );
uniformLocations[ 2][ 15 ] = glGetUniformLocation( programs[ 15 ], "texPingPong" );


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

		if( uniformLocations[ 2 ][ 0 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 0 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 1 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 1 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 1 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 1 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 1 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 1 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 1 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 2 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 2 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 2 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 2 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 2 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 2 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 2 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 3 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 3 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 3 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 3 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 3 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 3 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 3 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 4 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 4 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 4 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 4 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 4 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 4 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 4 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 5 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 5 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 5 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 5 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 5 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 5 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 5 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 6 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 6 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 6 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 6 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 6 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 6 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 6 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 7 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 7 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 7 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 7 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 7 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 7 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 7 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 8 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 8 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 8 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 8 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 8 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 8 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 8 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 9 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 9 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 9 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 9 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 9 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 9 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 9 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 10 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 10 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 10 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 10 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 10 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 10 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 10 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 11 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 11 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 11 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 11 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 11 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 11 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 11 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 12 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 12 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 12 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 12 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 12 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 12 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 12 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 13 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 13 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 13 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 13 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 13 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 13 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 13 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 14 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 14 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 14 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 14 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 14 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 14 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 14 ], texUnit ); texUnit++;
		}

	}
	{
		glUseProgram( programs[ 15 ] );
		GLuint texUnit = 0;
		
		if( uniformLocations[ 0 ][ 15 ] != -1 ) {
			glUniform1i( uniformLocations[ 0 ][ 15 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 1 ][ 15 ] != -1 ) {
			glUniform1i( uniformLocations[ 1 ][ 15 ], texUnit ); texUnit++;
		}

		if( uniformLocations[ 2 ][ 15 ] != -1 ) {
			glUniform1i( uniformLocations[ 2 ][ 15 ], texUnit ); texUnit++;
		}

	}


	glUseProgram( 0 );
	checkGLError();

	return success;
}

decompression::~decompression() {
	SafeRelease();
}

void decompression::SafeRelease() {
	for( int i = 0 ; i < 16 ; i++ ) {
		glDeleteProgram( programs[ i ] );
		programs[ i ] = 0;
	}
	checkGLError();
}
