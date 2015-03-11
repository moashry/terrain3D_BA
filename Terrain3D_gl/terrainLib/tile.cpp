/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "tile.h"

#include <float.h>

#include "profiler/profiler.hpp"

#include "resourcepool.h"
#include "terrain.h"

using namespace Renderer;

//#define NO_GEOMETRY
//#define NO_TEXTURES
//#define CPU_GEOMETRY_DECOMPRESSION

Tile::Tile(uint uiNumGPUs)
{
	memset(this, 0, sizeof(Tile));
	m_uiNumGPUs = uiNumGPUs;

	if (m_uiNumGPUs > 0)
	{
		m_perGPU = new PerGPU[m_uiNumGPUs];
		memset(m_perGPU, 0, m_uiNumGPUs * sizeof(PerGPU));
	}
}

void Tile::PageIn(Terrain* pTerrain)
{
	ScopedProfileSample sample(pTerrain->m_pProfiler, "Tile::PageIn");

	uint uiGPUIndex = pTerrain->m_uiGPUIndex;
	assert(uiGPUIndex < m_uiNumGPUs);
	PerGPU& gpu = m_perGPU[uiGPUIndex];

	assert(!gpu.m_bIsPagedIn);

	if (m_uiStripHeadersSize != 0) // Prüfe ob die Kachel leer ist
	{
		// ***** Geometrie *****
		if (gpu.m_pVBHandle != 0)
		{
			// Cache Hit
			pTerrain->m_pResourcePool->IncrementVBUsageCounter(gpu.m_pVBHandle);
			pTerrain->m_uiStatGeoSize += m_uiStatGeoSize;
		}
		else
		{
#ifndef NO_GEOMETRY
			// Cache Miss
			pTerrain->m_pResourcePool->GetVB((m_uiNumTriangles + m_uiNumBoundaryEdges * 2 + 3) * 3 * 4, &gpu.m_pVBHandle);
			pTerrain->m_pResourcePool->IncrementVBUsageCounter(gpu.m_pVBHandle);
			// + 3 für Dummy-Dreieck mit Randdreiecken (Shader erzeugt Dreiecke stets paarweise!)
			if (m_uiStripHeadersSize != -1) // m_uiStripHeadersSize == -1 bedeutet, dass die Geometriedaten unkomprimiert gespeichert sind
			{
#ifndef CPU_GEOMETRY_DECOMPRESSION
				pTerrain->m_geometryDecompressor.AddJob(m_uiStripDataFormat, m_uiStripHeadersSize, m_pStripHeaders, m_uiStripDataSize, m_pStripData, 
					m_uiNumTriangles, gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer, gpu.m_pVBHandle->m_uiOffset, m_uiX, m_uiY, m_uiLevel);

				pTerrain->m_ui64StatNumBytesTransferred += m_uiStripHeadersSize + m_uiStripDataSize;
				pTerrain->m_ui64StatNumBytesTransferredDecompressed += (m_uiNumTriangles + m_uiNumBoundaryEdges * 2) * 3 * 4;
#else // #ifndef CPU_GEOMETRY_DECOMPRESSION
				DecompressGeometryOnCPU(pTerrain->m_puiGeometryDecompressionBuffer);
				uint offset = gpu.m_pVBHandle->m_uiOffset;
				gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer->SubData(offset, (m_uiNumTriangles + 2 * m_uiNumBoundaryEdges) * 3 * 4, pTerrain->m_puiGeometryDecompressionBuffer);
				
				pTerrain->m_ui64StatNumBytesTransferred += (m_uiNumTriangles + 2 * m_uiNumBoundaryEdges) * 3 * 4;
				pTerrain->m_ui64StatNumBytesTransferredDecompressed += (m_uiNumTriangles + 2 * m_uiNumBoundaryEdges) * 3 * 4;
#endif // #ifndef CPU_GEOMETRY_DECOMPRESSION
			}
			else // if (m_uiStripHeadersSize != -1)
			{
				uint offset = gpu.m_pVBHandle->m_uiOffset;
				gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer->SubData(offset, m_uiStripDataSize, m_pStripData);
				pTerrain->m_ui64StatNumBytesTransferred += m_uiStripDataSize;
				pTerrain->m_ui64StatNumBytesTransferredDecompressed += m_uiStripDataSize;
			}
			m_uiStatGeoSize = (m_uiNumTriangles + 2 * m_uiNumBoundaryEdges) * 3 * 4;
			pTerrain->m_uiStatGeoSize += m_uiStatGeoSize;
#else // #ifndef NO_GEOMETRY
			pTerrain->m_pResourcePool->GetVB(6 * 4, &gpu.m_pVBHandle);
			pTerrain->m_pResourcePool->IncrementVBUsageCounter(gpu.m_pVBHandle);
			uint offset = gpu.m_pVBHandle->m_uiOffset;

			const uint vertexData[6] = { (0 << uiNUM_XY_BITS) | 0, (0 << uiNUM_XY_BITS) | uiTILE_SIZE, (uiTILE_SIZE << uiNUM_XY_BITS) | 0,
				(0 << uiNUM_XY_BITS) | uiTILE_SIZE, (uiTILE_SIZE << uiNUM_XY_BITS) | uiTILE_SIZE, (uiTILE_SIZE << uiNUM_XY_BITS) | 0 };
			gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer->SubData( offset, 6 * 4, vertexData );  

			pTerrain->m_ui64StatNumBytesTransferred += 6 * 4;
			pTerrain->m_ui64StatNumBytesTransferredDecompressed += 6 * 4;

			m_uiStatGeoSize = 6 * 4;
			pTerrain->m_uiStatGeoSize += m_uiStatGeoSize;
#endif // #ifndef NO_GEOMETRY
		}

		// ***** Textur *****
		PerGPU& gpuTex = m_pTextureTile->m_perGPU[uiGPUIndex];
		if (gpuTex.m_pTextureHandle != 0)
		{
			// Cache Hit
			if (pTerrain->m_pResourcePool->IncrementTextureUsageCounter(gpuTex.m_pTextureHandle) == 1)
			{
				pTerrain->m_uiStatTexSize += m_pTextureTile->m_uiStatTexSize;
			}
		}
		else
		{
			// Cache Miss
			pTerrain->m_pResourcePool->GetTexture(&gpuTex.m_pTextureHandle);
			pTerrain->m_pResourcePool->IncrementTextureUsageCounter(gpuTex.m_pTextureHandle);
#ifndef NO_TEXTURES
			if (m_pTextureTile->m_uiTextureDataSize != 0)
			{
				if (pTerrain->m_eTextureCompression == TEX_COMP_VQ)
				{
					printf_s("ERROR: VQ texture compression is no longer supported!\n");
					exit(-1);
				}
				else if (pTerrain->m_eTextureCompression == TEX_COMP_S3TC)
				{
					// *** S3TC ***
					byte* ptr = m_pTextureTile->m_pTextureData;
					uint uiRes = uiTILE_SIZE;
					for (uint uiMipMapLevel = 0; uiMipMapLevel < uiNUM_MIPMAP_LEVELS; uiMipMapLevel++)
					{
						gpuTex.m_pTextureHandle->m_pTexture->CompressedImage( uiMipMapLevel, ptr );
						ptr += (uiRes * uiRes) / 2;
						if (uiRes > 4) { uiRes /= 2; }
					}

					pTerrain->m_ui64StatNumBytesTransferred += (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					pTerrain->m_ui64StatNumBytesTransferredDecompressed += (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					
					m_pTextureTile->m_uiStatTexSize = (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					pTerrain->m_uiStatTexSize += m_pTextureTile->m_uiStatTexSize;
				}
				else // if (pTerrain->m_eTextureCompression == ...)
				{
					// *** S3TC Shared (Mipmap-Levels aus den Levels des Tile Trees holen) ***
					assert(IsPowerOfTwo(uiTILE_SIZE));
					uint x = 0;
					uint y = 0;
					Tile* pTile = m_pTextureTile;
					byte* ptr = m_pTextureTile->m_pTextureData;
					uint sourceSize = uiTILE_SIZE;
					for (uint uiMipMapLevel = 0; uiMipMapLevel < uiNUM_MIPMAP_LEVELS; uiMipMapLevel++)
					{
						if (uiMipMapLevel >= uiNUM_MIPMAP_LEVELS - 2) // 2x2 und 1x1 müssen aus einem 4x4 Block geholt werden
						{
							uint* srcBlock = reinterpret_cast<uint*>(ptr + ((sourceSize / 4) * (y / 4) + x / 4) * 8);
							uint block[2];
							block[0] = srcBlock[0]; // Farben übernehmen
							uint offset = 8 * (y % 4) + 2 * (x % 4); // Bit-Offset in die 16 x 2 Bits
							block[1] = (srcBlock[1] >> offset) & 0xF0F;
							gpuTex.m_pTextureHandle->m_pTexture->CompressedImage(uiMipMapLevel, block);
						}
						else
						{
							// compressed textures need an adapted row pitch - see http://forums.xna.com/forums/t/1904.aspx)
							// mip-mapping requires adapted texture extents
							const uint textureSize = uiTILE_SIZE >> uiMipMapLevel;
							const uint realSize = textureSize > 4 ? textureSize : 4;

							byte * const data = ptr + ((sourceSize / 4) * (y / 4) + x / 4) * 8;

							if( realSize < sourceSize ) {
								// realSize < sourceSize <= uiTILE_SIZE => imageSize < sizeof( textureDataBuffer )
								char textureDataBuffer[ uiTILE_SIZE * uiTILE_SIZE / 2 ];

								for( uint i = 0 ; i < realSize / 4; i++ ) {
									memcpy( textureDataBuffer + i * realSize * 2, data + i * sourceSize * 2, realSize * 2 );
								}

								gpuTex.m_pTextureHandle->m_pTexture->CompressedImage( uiMipMapLevel, textureDataBuffer );
							}
							else {
								gpuTex.m_pTextureHandle->m_pTexture->CompressedImage( uiMipMapLevel, data );
							}
						}
						
						if (pTile->m_uiLevel == 0)
						{
							x /= 2;
							y /= 2;
							ptr += (sourceSize * sourceSize) / 2;
							if (sourceSize > 4) { sourceSize /= 2; }
						}
						else
						{
							x = (pTile->m_uiChild % 2) * (uiTILE_SIZE / 2) + x / 2;
							y = (pTile->m_uiChild / 2) * (uiTILE_SIZE / 2) + y / 2;
							pTile = pTile->m_parent;
							ptr = pTile->m_pTextureData;
						}
					}

					pTerrain->m_ui64StatNumBytesTransferred += (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					pTerrain->m_ui64StatNumBytesTransferredDecompressed += (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					
					m_pTextureTile->m_uiStatTexSize = (uiTILE_SIZE * uiTILE_SIZE * 2) / 3;
					pTerrain->m_uiStatTexSize += m_pTextureTile->m_uiStatTexSize;
				}
			} // if (m_uiTextureDataSize != 0)
#endif // #ifndef NO_TEXTURES
		}
	} // if (m_uiStripHeadersSize != 0)



	gpu.m_bIsPagedIn = true;

	Tile* current = this;
	while (current != 0)
	{
		if (!current->m_perGPU)
		{
			break;
		}

		current->m_perGPU[uiGPUIndex].m_uiPagedInCounter++;
		if (current->m_perGPU[uiGPUIndex].m_uiPagedInCounter != 1)
		{
			break;
		}
		// Tile ist erstes Tile im Subtree; Propagiere nach oben
		current = current->m_parent;
	}

}


void Tile::PageOut(Terrain* pTerrain)
{
	ScopedProfileSample sample(pTerrain->m_pProfiler, "Tile::PageOut");

	uint uiGPUIndex = pTerrain->m_uiGPUIndex;
	assert(uiGPUIndex < m_uiNumGPUs);
	PerGPU& gpu = m_perGPU[uiGPUIndex];

	assert(gpu.m_bIsPagedIn);

	if (m_uiStripHeadersSize != 0)
	{
		pTerrain->m_pResourcePool->DecrementVBUsageCounter(gpu.m_pVBHandle);

		pTerrain->m_uiStatGeoSize -= m_uiStatGeoSize;

		PerGPU& gpuTex = m_pTextureTile->m_perGPU[uiGPUIndex];
		if (pTerrain->m_pResourcePool->DecrementTextureUsageCounter(gpuTex.m_pTextureHandle) == 0)
		{
			pTerrain->m_uiStatTexSize -= m_pTextureTile->m_uiStatTexSize;
		}
	} // if (m_uiStripHeadersSize != 0)


	gpu.m_bIsPagedIn = false;

	Tile* current = this;
	while (current != 0)
	{
		if (!current->m_perGPU)
		{
			break;
		}

		current->m_perGPU[uiGPUIndex].m_uiPagedInCounter--;
		if (current->m_perGPU[uiGPUIndex].m_uiPagedInCounter != 0)
		{
			break;
		}
		// Tile ist letztes Tile im Subtree; Propagiere nach oben
		current = current->m_parent;
	}

}


void Tile::Render(Terrain* pTerrain, bool bDrawBBox, bool bSatModify)
{
	ScopedProfileSample sample(pTerrain->m_pProfiler, "Tile::Render");

	if (m_uiStripHeadersSize != 0)
	{
		uint uiGPUIndex = pTerrain->m_uiGPUIndex;
		assert(uiGPUIndex < m_uiNumGPUs);
		const PerGPU& gpu = m_perGPU[uiGPUIndex];
		const PerGPU& gpuTex = m_pTextureTile->m_perGPU[uiGPUIndex];

		tum3D::Mat4f mTranslate;
		tum3D::translationMat( m_BBox.m_vCenter - m_BBox.m_vHalfExtent, mTranslate );
		tum3D::Mat4f transposed;
		mTranslate.transpose( transposed );
		mTranslate = transposed;

		assert( gpuTex.m_pTextureHandle->m_pTexture );
		Renderer::FramebufferTexture2D &terrainTexture = *gpuTex.m_pTextureHandle->m_pTexture;
		pTerrain->m_tileRenderingEffect.txTerrain.Set( &terrainTexture );

		float fScaleZ = 2.0f * m_BBox.m_vHalfExtent.z() / static_cast<float>(1 << m_uiStripDataFormat);

		float vScale[] = { m_BBox.m_vHalfExtent.x() / static_cast<float>(uiTILE_SIZE / 2), m_BBox.m_vHalfExtent.y() / static_cast<float>(uiTILE_SIZE / 2), fScaleZ };
		pTerrain->m_tileRenderingEffect.vScale.Set(vScale);

		pTerrain->m_tileRenderingEffect.vTexCoordOffset.Set(m_vTexCoordOffset);
		pTerrain->m_tileRenderingEffect.vTexCoordSize.Set(m_vTexCoordSize);

		tum3D::Mat4f mWorldView;
		mWorldView = mTranslate * pTerrain->m_mView;

		pTerrain->m_tileRenderingEffect.mWorldView.Set(mWorldView);

		pTerrain->m_tileRenderingEffect.fBorderHeight.Set(std::max(m_BBox.m_vHalfExtent.x() / static_cast<float>(uiTILE_SIZE / 2), pTerrain->m_fMinVerticalTolerance) * 3.0f); // 3-facher World Space Error als Höhe des Saums

#if defined( INDEXED_DRAWING )
		gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer->Bind( GL_ELEMENT_ARRAY_BUFFER );
		pTerrain->m_pDevice->SetNoVertexBuffer();
#else
		gpu.m_pVBHandle->m_pVBDesc->m_pVertexBuffer->Bind( GL_ARRAY_BUFFER );
		pTerrain->m_tileRenderingEffect.SetupBinding( 4, m_pVBHandle->m_uiOffset );
#endif

		pTerrain->m_tileRenderingEffect.SetupPass(bSatModify ? 1 : 0);

		uint uiSamplerState;
		if (gpu.m_bIsLODSufficient)
		{
			uiSamplerState = static_cast<uint>((static_cast<float>(m_uiLevel) - gpu.m_fRequiredLOD) * static_cast<float>(uiNUM_SAMPLER_STATES));
			if (uiSamplerState >= uiNUM_SAMPLER_STATES) { uiSamplerState = uiNUM_SAMPLER_STATES - 1; }
		}
		else
		{
			uiSamplerState = 0;
		}

		Texture2DScope scope( terrainTexture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, static_cast<float>(uiSamplerState+1) / static_cast<float>(uiNUM_SAMPLER_STATES) );

#ifndef NO_GEOMETRY
#	if defined( INDEXED_DRAWING )
		glDrawElements( GL_TRIANGLES, (m_uiNumTriangles + m_uiNumBoundaryEdges * 2) * 3, GL_UNSIGNED_INT, (GLbyte*) gpu.m_pVBHandle->m_uiOffset );
		checkGLError();
#	else
		glDrawArrays( GL_TRIANGLES, 0, (m_uiNumTriangles + m_uiNumBoundaryEdges * 2) * 3 );
		checkGLError();
#	endif
		pTerrain->m_uiStatNumTriangles += m_uiNumTriangles + m_uiNumBoundaryEdges * 2;
		pTerrain->m_ui64StatNumTrianglesRendered += m_uiNumTriangles + m_uiNumBoundaryEdges * 2;
#else
#	if defined( INDEXED_DRAWING )
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLbyte*) gpu.m_pVBHandle->m_uiOffset );
		checkGLError();
#	else
		glDrawArrays( GL_TRIANGLES, 0, 6 );
		checkGLError();
#	endif
		pTerrain->m_uiStatNumTriangles += 2;
		pTerrain->m_ui64StatNumTrianglesRendered += 2;
#endif

		if (bDrawBBox)
		{
			tum3D::Mat4f mWorldView;
			tum3D::scalingMat( m_BBox.m_vHalfExtent * 2.0f, mWorldView );
			mWorldView *= mTranslate * pTerrain->m_mView;
			
			pTerrain->m_tileRenderingBBEffect.mWorldView.Set(mWorldView);
			
			float fColor[4];
			if (gpu.m_bIsLODSufficient)
			{
				float f;
				if (pTerrain->m_uiNumLevels > 1)
				{
					f = static_cast<float>(m_uiLevel) / static_cast<float>(pTerrain->m_uiNumLevels - 1);
				}
				else
				{
					f = 1.0f;
				}
				fColor[0] = 1.0f - f; fColor[1] = f; fColor[2] = 0.0f; fColor[3] = 1.0f;
			}
			else
			{
				fColor[0] = 0.0f; fColor[1] = 0.0f; fColor[2] = 1.0f; fColor[3] = 1.0f;
			}

			pTerrain->m_tileRenderingBBEffect.vColorBB.Set(fColor);

			pTerrain->m_pVertexBufferBB->Bind( GL_ARRAY_BUFFER );
			pTerrain->m_tileRenderingBBEffect.SetupBinding( sizeof( VertexBB ), 0 );
			
			pTerrain->m_pIndexBufferBB->Bind( GL_ELEMENT_ARRAY_BUFFER );

			uint numPasses = pTerrain->m_tileRenderingBBEffect.GetNumPasses();
			for (uint p = 0; p < numPasses; p++)
			{
				pTerrain->m_tileRenderingBBEffect.SetupPass(p);
				glDrawElements( GL_LINES, 24, GL_UNSIGNED_INT, (GLbyte*) 0 );
				checkGLError();
			}

		} // if (bDrawBBox)

	} // if (m_uiStripHeadersSize != 0)

}


class Point
{
public:
	int m_x, m_y, m_z;
	Point()
	{
	}
	Point(int x, int y)
	{
		m_x = x;
		m_y = y;
	}
};

inline Point operator+(const Point& p, const Point& q)
{
	return Point(p.m_x + q.m_x, p.m_y + q.m_y);
}

inline Point operator-(const Point& p, const Point& q)
{
	return Point(p.m_x - q.m_x, p.m_y - q.m_y);
}

inline Point operator-(const Point& p)
{
	return Point(-p.m_x, -p.m_y);
}

inline Point operator*(int a, const Point& p)
{
	return Point(a * p.m_x, a * p.m_y);
}

class Matrix
{
public:
	int m_m11, m_m12, m_m21, m_m22, m_div;
	Matrix()
	{
	}
	Matrix(int m11, int m12, int m21, int m22, int div = 1)
	{
		m_m11 = m11; m_m12 = m12; m_m21 = m21; m_m22 = m22; m_div = div;
	}
};

inline Point operator*(const Matrix& m, const Point& p)
{
	return Point((m.m_m11 * p.m_x + m.m_m12 * p.m_y) / m.m_div, (m.m_m21 * p.m_x + m.m_m22 * p.m_y) / m.m_div);
}

const uint A_L = 0;
const uint A_R = 1;
const uint B_L = 2;
const uint B_R = 3;
const uint C_L = 4;
const uint C_R = 5;

const uint typeTable[6][3] =
{
	{ A_L, B_R, C_L },
	{ A_R, B_L, C_R },
	{ A_L, B_R, C_L },
	{ A_R, B_L, C_R },
	{ A_R, B_L, C_R },
	{ A_L, B_R, C_L }
};

const Matrix thirdVertexMatrices[6] =
{
	Matrix(0, 1, -1, 0),
	Matrix(1, 1, -1, 1),
	Matrix(1, 1, -1, 1),
	Matrix(0, 1, -1, 0),
	Matrix(1, 1, -1, 1, 2),
	Matrix(1, 1, -1, 1, 2)
};

const Point dirTable[8] =
{
	Point(0, 1),
	Point(1, 1),
	Point(1, 0),
	Point(1, -1),
	Point(0, -1),
	Point(-1, -1),
	Point(-1, 0),
	Point(-1, 1)
};

const uint uiSTRIP_LENGTH = 16;

uint Tile::ReadData(uint index) const
{
	uint bitPos = index * (m_uiStripDataFormat + 2);
	uint readPos = bitPos >> 4;
	uint data = (reinterpret_cast<ushort*>(m_pStripData)[readPos + 1] << 16)
		| reinterpret_cast<ushort*>(m_pStripData)[readPos];
	uint relBitPos = bitPos & 0xF;
	return (data >> relBitPos) & ( (1 << (m_uiStripDataFormat + 2)) - 1 );
}

inline uint PackVertex(Point v)
{
	uint result;
	result = static_cast<uint>(v.m_x);
	result |= static_cast<uint>(v.m_y) << uiNUM_XY_BITS;
	result |= static_cast<uint>(v.m_z) << (2 * uiNUM_XY_BITS);
	return result;
}

inline Point UnpackVertex(uint v)
{
	Point result;
	result.m_x = v & ((1 << uiNUM_XY_BITS) - 1);
	result.m_y = (v & (((1 << uiNUM_XY_BITS) - 1) << uiNUM_XY_BITS)) >> uiNUM_XY_BITS;
	result.m_z = v >> (2 * uiNUM_XY_BITS);
	return result;
}

bool Tile::GetHeight(float fX, float fY, float* pfHeight) const
{
	float fScaleX = m_BBox.m_vHalfExtent.x() / static_cast<float>(uiTILE_SIZE / 2);
	float fScaleY = m_BBox.m_vHalfExtent.y() / static_cast<float>(uiTILE_SIZE / 2);
	float fScaleZ = 2.0f * m_BBox.m_vHalfExtent.z() / static_cast<float>(1 << m_uiStripDataFormat);
	float fBiasZ = 0.5f * fScaleZ;

	if (m_uiStripHeadersSize != -1) // Prüfe ob Geometriedaten komprimiert vorliegen
	{
		for (uint uiStrip = 0; uiStrip < m_uiStripHeadersSize / 4; uiStrip++)
		{
			uint uiStripHeader = reinterpret_cast<uint*>(m_pStripHeaders)[uiStrip];
			Point v1, v2, v3;
			v1.m_x = uiStripHeader & ((1 << (uiNUM_XY_BITS - 1)) - 1); // 9 Bit
			v1.m_y = (uiStripHeader & (((1 << (uiNUM_XY_BITS - 1)) - 1) << 9)) >> 9; // 9 Bit
			uint logLength = (uiStripHeader & 0x003C0000) >> 18; // 4 Bit
			uint dir = (uiStripHeader & 0x01C00000) >> 22; // 3 Bit
			uint LR = (uiStripHeader & 0x02000000) >> 25; // 1 Bit
			uint dindex = ((uiStripHeader & 0x1C000000) >> 26) + 2; // 3 Bit
			uint iindex = (uiStripHeader & 0x20000000) >> 29; // 1 Bit

			uint length = 1 << logLength;
			v2 = v1 + static_cast<int>(length) * dirTable[dir];

			uint index = uiStrip * uiSTRIP_LENGTH + 2;

			if (iindex == 0)
			{
				v1.m_z = ReadData(index - dindex) >> 2;
				v2.m_z = ReadData(index - 1) >> 2;
			}
			else
			{
				v1.m_z = ReadData(index - 1) >> 2;
				v2.m_z = ReadData(index - dindex) >> 2;
			}

			uint type = 0;
			for (uint i = 0; i < uiSTRIP_LENGTH; i++)
			{
				uint data = ReadData(index + i);
				uint ABC = data & 0x3;
				if (ABC > 2)
				{
					break;
				}
				if (i == 0)
				{
					type = ABC << 1 | LR;
				}
				else
				{
					type = typeTable[type][ABC];
				}
				v3 = v1 + thirdVertexMatrices[type] * (v2 - v1);
				v3.m_z = data >> 2;

				// Berechne baryzentrische Koordinaten alpha, beta, gamma
				float q2x = static_cast<float>(v2.m_x - v1.m_x);
				float q2y = static_cast<float>(v2.m_y - v1.m_y);
				float q3x = static_cast<float>(v3.m_x - v1.m_x);
				float q3y = static_cast<float>(v3.m_y - v1.m_y);
				float qx = fX / fScaleX - static_cast<float>(v1.m_x);
				float qy = fY / fScaleY - static_cast<float>(v1.m_y);
				float d = q2x * q3y - q2y * q3x;
				float beta = (q3y * qx - q3x * qy) / d;
				if (0.0f <= beta && beta <= 1.0f)
				{
					float gamma = (q2x * qy - q2y * qx) / d;
					float alpha = beta + gamma;
					if (0.0f <= gamma && alpha <= 1.0f)
					{
						alpha = 1.0f - alpha;
						*pfHeight = (alpha * static_cast<float>(v1.m_z) + beta * static_cast<float>(v2.m_z) + gamma * static_cast<float>(v3.m_z)) * fScaleZ + fBiasZ + m_BBox.m_vCenter.z() - m_BBox.m_vHalfExtent.z();
						return true;
					}
				}
				
				if ((type & 0x1) == 0)
				{
					v2 = v3;
				}
				else
				{
					v1 = v3;
				}

			}

		}

		return false;
	}
	else // if (m_uiStripHeadersSize != -1)
	{
		for (uint uiTriangle = 0; uiTriangle < m_uiStripDataSize / (3 * 4); uiTriangle++)
		{
			Point v2 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle]);
			Point v1 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle + 1]);
			Point v3 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle + 2]);
			if (v2.m_x <= uiTILE_SIZE && v2.m_y <= uiTILE_SIZE && v3.m_x <= uiTILE_SIZE && v3.m_y <= uiTILE_SIZE) // Saum-Dreiecke ausschließen
			{
				// Berechne baryzentrische Koordinaten alpha, beta, gamma
				float q2x = static_cast<float>(v2.m_x - v1.m_x);
				float q2y = static_cast<float>(v2.m_y - v1.m_y);
				float q3x = static_cast<float>(v3.m_x - v1.m_x);
				float q3y = static_cast<float>(v3.m_y - v1.m_y);
				float qx = fX / fScaleX - static_cast<float>(v1.m_x);
				float qy = fY / fScaleY - static_cast<float>(v1.m_y);
				float d = q2x * q3y - q2y * q3x;
				float beta = (q3y * qx - q3x * qy) / d;
				if (0.0f <= beta && beta <= 1.0f)
				{
					float gamma = (q2x * qy - q2y * qx) / d;
					float alpha = beta + gamma;
					if (0.0f <= gamma && alpha <= 1.0f)
					{
						alpha = 1.0f - alpha;
						*pfHeight = (alpha * static_cast<float>(v1.m_z) + beta * static_cast<float>(v2.m_z) + gamma * static_cast<float>(v3.m_z)) * fScaleZ + fBiasZ + m_BBox.m_vCenter.z() - m_BBox.m_vHalfExtent.z();
						return true;
					}
				}

			}

		}

		return false;
	}

}


bool Tile::ComputeRayTerrainIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, tum3D::Vec3f* pvIntersectionPoint) const
{
	tum3D::Vec3f vLocalRayOrigin = vRayOrigin;

	vLocalRayOrigin += m_BBox.m_vHalfExtent - m_BBox.m_vCenter;

	float fScaleX = m_BBox.m_vHalfExtent.x() / static_cast<float>(uiTILE_SIZE / 2);
	float fScaleY = m_BBox.m_vHalfExtent.y() / static_cast<float>(uiTILE_SIZE / 2);
	float fScaleZ = 2.0f * m_BBox.m_vHalfExtent.z() / static_cast<float>(1 << m_uiStripDataFormat);
	float fBiasZ = 0.5f * fScaleZ;

	bool bIsIntersecting = false;
	float fMinDistance = FLT_MAX;
	
	if (m_uiStripHeadersSize != -1) // Prüfe ob Geometriedaten komprimiert vorliegen
	{
		for (uint uiStrip = 0; uiStrip < m_uiStripHeadersSize / 4; uiStrip++)
		{
			uint uiStripHeader = reinterpret_cast<uint*>(m_pStripHeaders)[uiStrip];
			Point v1, v2, v3;
			v1.m_x = uiStripHeader & ((1 << (uiNUM_XY_BITS - 1)) - 1); // 9 Bit
			v1.m_y = (uiStripHeader & (((1 << (uiNUM_XY_BITS - 1)) - 1) << 9)) >> 9; // 9 Bit
			uint logLength = (uiStripHeader & 0x003C0000) >> 18; // 4 Bit
			uint dir = (uiStripHeader & 0x01C00000) >> 22; // 3 Bit
			uint LR = (uiStripHeader & 0x02000000) >> 25; // 1 Bit
			uint dindex = ((uiStripHeader & 0x1C000000) >> 26) + 2; // 3 Bit
			uint iindex = (uiStripHeader & 0x20000000) >> 29; // 1 Bit

			uint length = 1 << logLength;
			v2 = v1 + static_cast<int>(length) * dirTable[dir];

			uint index = uiStrip * uiSTRIP_LENGTH + 2;

			if (iindex == 0)
			{
				v1.m_z = ReadData(index - dindex) >> 2;
				v2.m_z = ReadData(index - 1) >> 2;
			}
			else
			{
				v1.m_z = ReadData(index - 1) >> 2;
				v2.m_z = ReadData(index - dindex) >> 2;
			}

			uint type = 0;
			for (uint i = 0; i < uiSTRIP_LENGTH; i++)
			{
				uint data = ReadData(index + i);
				uint ABC = data & 0x3;
				if (ABC > 2)
				{
					break;
				}
				if (i == 0)
				{
					type = ABC << 1 | LR;
				}
				else
				{
					type = typeTable[type][ABC];
				}
				v3 = v1 + thirdVertexMatrices[type] * (v2 - v1);
				v3.m_z = data >> 2;

				// Berechne Schnittpunkt Strahl - Dreieck
				tum3D::Vec3f q2(static_cast<float>(v2.m_x - v1.m_x) * fScaleX,
					static_cast<float>(v2.m_y - v1.m_y) * fScaleY,
					static_cast<float>(v2.m_z - v1.m_z) * fScaleZ);
				tum3D::Vec3f q3(static_cast<float>(v3.m_x - v1.m_x) * fScaleX,
					static_cast<float>(v3.m_y - v1.m_y) * fScaleY,
					static_cast<float>(v3.m_z - v1.m_z) * fScaleZ);

				tum3D::Vec3f v;
				tum3D::crossProd( vRayDirection, q3, v );
				float det = tum3D::dotProd(q2, v);
				if (det < 0.0f)
				{
					tum3D::Vec3f q(vLocalRayOrigin.x() - static_cast<float>(v1.m_x) * fScaleX,
						vLocalRayOrigin.y() - static_cast<float>(v1.m_y) * fScaleY,
						vLocalRayOrigin.z() - (static_cast<float>(v1.m_z) * fScaleZ + fBiasZ));

					float beta = tum3D::dotProd(q, v);
					if (det <= beta && beta <= 0.0f)
					{
						tum3D::Vec3f w;
						tum3D::crossProd( q, q2, w );
						float gamma = tum3D::dotProd(vRayDirection, w);
						if (det <= beta + gamma && gamma <= 0.0f)
						{
							float t = tum3D::dotProd(q3, w);
							if (t <= 0.0f)
							{
								bIsIntersecting = true;
								t /= det;
								if (t < fMinDistance)
								{
									if (pvIntersectionPoint) { *pvIntersectionPoint = vRayOrigin + t * vRayDirection; }
									else { return true; }
									fMinDistance = t;
								}
							}
						}
					}
				}

				if ((type & 0x1) == 0)
				{
					v2 = v3;
				}
				else
				{
					v1 = v3;
				}

			}

		}

	}
	else // if (m_uiStripHeadersSize != -1)
	{
		for (uint uiTriangle = 0; uiTriangle < m_uiStripDataSize / (3 * 4); uiTriangle++)
		{
			Point v2 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle]);
			Point v1 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle + 1]);
			Point v3 = UnpackVertex(reinterpret_cast<uint*>(m_pStripData)[3 * uiTriangle + 2]);
			if (v2.m_x <= uiTILE_SIZE && v2.m_y <= uiTILE_SIZE && v3.m_x <= uiTILE_SIZE && v3.m_y <= uiTILE_SIZE) // Saum-Dreiecke ausschließen
			{
				// Berechne Schnittpunkt Strahl - Dreieck
				tum3D::Vec3f q2(static_cast<float>(v2.m_x - v1.m_x) * fScaleX,
					static_cast<float>(v2.m_y - v1.m_y) * fScaleY,
					static_cast<float>(v2.m_z - v1.m_z) * fScaleZ);
				tum3D::Vec3f q3(static_cast<float>(v3.m_x - v1.m_x) * fScaleX,
					static_cast<float>(v3.m_y - v1.m_y) * fScaleY,
					static_cast<float>(v3.m_z - v1.m_z) * fScaleZ);

				tum3D::Vec3f v;
				tum3D::crossProd( vRayDirection, q3, v );
				float det = tum3D::dotProd(q2, v);
				if (det < 0.0f)
				{
					tum3D::Vec3f q(vLocalRayOrigin.x() - static_cast<float>(v1.m_x) * fScaleX,
						vLocalRayOrigin.y() - static_cast<float>(v1.m_y) * fScaleY,
						vLocalRayOrigin.z() - (static_cast<float>(v1.m_z) * fScaleZ + fBiasZ));

					float beta = tum3D::dotProd(q, v);
					if (det <= beta && beta <= 0.0f)
					{
						tum3D::Vec3f w;
						tum3D::crossProd(q, q2, w);
						float gamma = tum3D::dotProd(vRayDirection, w);
						if (det <= beta + gamma && gamma <= 0.0f)
						{
							float t = tum3D::dotProd(q3, w);
							if (t <= 0.0f)
							{
								bIsIntersecting = true;
								t /= det;
								if (t < fMinDistance)
								{
									if (pvIntersectionPoint) { *pvIntersectionPoint = vRayOrigin + t * vRayDirection; }
									else { return true; }
									fMinDistance = t;
								}
							}
						}
					}
				}

			}

		}

	}

	return bIsIntersecting;

}


void Tile::WriteASY() const
{
	assert(m_uiStripHeadersSize != -1);

	FILE* hFile;
	fopen_s(&hFile, "tile.asy", "wt");

	for (uint uiStrip = 0; uiStrip < m_uiStripHeadersSize / 4; uiStrip++)
	{
		uint uiStripHeader = reinterpret_cast<uint*>(m_pStripHeaders)[uiStrip];
		Point v1, v2, v3;
		v1.m_x = uiStripHeader & ((1 << (uiNUM_XY_BITS - 1)) - 1); // 9 Bit
		v1.m_y = (uiStripHeader & (((1 << (uiNUM_XY_BITS - 1)) - 1) << 9)) >> 9; // 9 Bit
		uint logLength = (uiStripHeader & 0x003C0000) >> 18; // 4 Bit
		uint dir = (uiStripHeader & 0x01C00000) >> 22; // 3 Bit
		uint LR = (uiStripHeader & 0x02000000) >> 25; // 1 Bit
		uint dindex = ((uiStripHeader & 0x1C000000) >> 26) + 2; // 3 Bit
		uint iindex = (uiStripHeader & 0x20000000) >> 29; // 1 Bit

		uint length = 1 << logLength;
		v2 = v1 + static_cast<int>(length) * dirTable[dir];

		uint index = uiStrip * uiSTRIP_LENGTH + 2;

		if (iindex == 0)
		{
			v1.m_z = ReadData(index - dindex) >> 2;
			v2.m_z = ReadData(index - 1) >> 2;
		}
		else
		{
			v1.m_z = ReadData(index - 1) >> 2;
			v2.m_z = ReadData(index - dindex) >> 2;
		}

		if (uiStrip == 0)
		{
			fprintf_s(hFile, "v1 = (%f, %f);\n", static_cast<float>(v1.m_x) / static_cast<float>(uiTILE_SIZE), 1.0f - static_cast<float>(v1.m_y) / static_cast<float>(uiTILE_SIZE));
			fprintf_s(hFile, "v2 = (%f, %f);\n", static_cast<float>(v2.m_x) / static_cast<float>(uiTILE_SIZE), 1.0f - static_cast<float>(v2.m_y) / static_cast<float>(uiTILE_SIZE));
			fprintf_s(hFile, "p1 = (%f, %f);\n", static_cast<float>(v1.m_x + v2.m_x) / static_cast<float>(2 * uiTILE_SIZE), 1.0f - static_cast<float>(v1.m_y + v2.m_y) / static_cast<float>(2 * uiTILE_SIZE));
			fprintf_s(hFile, "d1 = %i;\n\n", static_cast<int>(dir) * 45);
		}

		uint type = 0;
		for (uint i = 0; i < uiSTRIP_LENGTH; i++)
		{
			uint data = ReadData(index + i);
			uint ABC = data & 0x3;
			if (ABC > 2)
			{
				break;
			}
			if (i == 0)
			{
				type = ABC << 1 | LR;
			}
			else
			{
				type = typeTable[type][ABC];
			}

			switch (type)
			{
				case A_L:
				{
					fprintf_s(hFile, "DrawTriangle(A_L, false);\n");
					break;
				}
				case A_R:
				{
					fprintf_s(hFile, "DrawTriangle(A_R, false);\n");
					break;
				}
				case B_L:
				{
					fprintf_s(hFile, "DrawTriangle(B_L, false);\n");
					break;
				}
				case B_R:
				{
					fprintf_s(hFile, "DrawTriangle(B_R, false);\n");
					break;
				}
				case C_L:
				{
					fprintf_s(hFile, "DrawTriangle(C_L, false);\n");
					break;
				}
				case C_R:
				{
					fprintf_s(hFile, "DrawTriangle(C_R, false);\n");
					break;
				}
			}

		}

	}

	fclose(hFile);

}


void Tile::DecompressGeometryOnCPU(uint* puiDecompressionBuffer) const
{
	assert(m_uiStripHeadersSize != -1);

	uint uiWritePos = 0;

	for (uint uiStrip = 0; uiStrip < m_uiStripHeadersSize / 4; uiStrip++)
	{
		uint uiStripHeader = reinterpret_cast<uint*>(m_pStripHeaders)[uiStrip];
		Point v1, v2, v3;
		uint p1, p2, p3;
		v1.m_x = uiStripHeader & ((1 << (uiNUM_XY_BITS - 1)) - 1); // 9 Bit
		v1.m_y = (uiStripHeader & (((1 << (uiNUM_XY_BITS - 1)) - 1) << 9)) >> 9; // 9 Bit
		uint logLength = (uiStripHeader & 0x003C0000) >> 18; // 4 Bit
		uint dir = (uiStripHeader & 0x01C00000) >> 22; // 3 Bit
		uint LR = (uiStripHeader & 0x02000000) >> 25; // 1 Bit
		uint dindex = ((uiStripHeader & 0x1C000000) >> 26) + 2; // 3 Bit
		uint iindex = (uiStripHeader & 0x20000000) >> 29; // 1 Bit

		if (v1.m_x == 0 && 4 <= dir)
		{
			v1.m_x = uiTILE_SIZE;
		}
		if (v1.m_y == 0 && 2 <= dir && dir <= 5)
		{
			v1.m_y = uiTILE_SIZE;
		}

		uint length = 1 << logLength;
		v2 = v1 + static_cast<int>(length) * dirTable[dir];

		uint index = uiStrip * uiSTRIP_LENGTH + 2;

		if (iindex == 0)
		{
			v1.m_z = ReadData(index - dindex) >> 2;
			v2.m_z = ReadData(index - 1) >> 2;
		}
		else
		{
			v1.m_z = ReadData(index - 1) >> 2;
			v2.m_z = ReadData(index - dindex) >> 2;
		}

		p1 = PackVertex(v1);
		p2 = PackVertex(v2);

		uint type = 0;
		for (uint i = 0; i < uiSTRIP_LENGTH; i++)
		{
			uint data = ReadData(index + i);
			uint ABC = data & 0x3;
			if (ABC > 2)
			{
				break;
			}
			if (i == 0)
			{
				type = ABC << 1 | LR;
			}
			else
			{
				type = typeTable[type][ABC];
			}
			v3 = v1 + thirdVertexMatrices[type] * (v2 - v1);
			v3.m_z = data >> 2;

			p3 = PackVertex(v3);
			
			puiDecompressionBuffer[uiWritePos++] = p2;
			puiDecompressionBuffer[uiWritePos++] = p1;
			puiDecompressionBuffer[uiWritePos++] = p3;

			if ((type & 0x1) == 0)
			{
				// Stitching
				uint a = 0;

				if (v2.m_x == 0)
				{
					if (v3.m_x == 0)
					{
						a = 0x3 << (uiNUM_XY_BITS - 2);
					}
				}
				else if (v2.m_x == uiTILE_SIZE)
				{
					if (v3.m_x == uiTILE_SIZE)
					{
						a = 0x3 << (uiNUM_XY_BITS - 3);
					}
				}
				if (v2.m_y == 0)
				{
					if (v3.m_y == 0)
					{
						a = (0x3 << (uiNUM_XY_BITS - 2)) << uiNUM_XY_BITS;
					}
				}
				else if (v2.m_y == uiTILE_SIZE)
				{
					if (v3.m_y == uiTILE_SIZE)
					{
						a = (0x3 << (uiNUM_XY_BITS - 3)) << uiNUM_XY_BITS;
					}
				}

				if (a != 0)
				{
					uint p2_, p3_;
					p2_ = p2 | a;
					p3_ = p3 | a;

					puiDecompressionBuffer[uiWritePos++] = p2;
					puiDecompressionBuffer[uiWritePos++] = p3;
					puiDecompressionBuffer[uiWritePos++] = p2_;

					puiDecompressionBuffer[uiWritePos++] = p2_;
					puiDecompressionBuffer[uiWritePos++] = p3;
					puiDecompressionBuffer[uiWritePos++] = p3_;
				}

				v2 = v3;
				p2 = p3;
			}
			else
			{
				v1 = v3;
				p1 = p3;
			}

		}

	}

	assert(uiWritePos == (m_uiNumTriangles + m_uiNumBoundaryEdges * 2) * 3);

}



Tile::~Tile()
{
	for (uint i = 0; i < m_uiNumGPUs; i++)
	{
		PerGPU& gpu = m_perGPU[i];
		assert(!gpu.m_bIsPagedIn);
		if (gpu.m_pVBHandle != NULL)
		{
			gpu.m_pVBHandle->m_pResourcePool->ReleaseVB(&gpu.m_pVBHandle);
		}
		PerGPU& gpuTex = m_pTextureTile->m_perGPU[i];
		if (gpuTex.m_pTextureHandle != NULL)
		{
			gpuTex.m_pTextureHandle->m_pResourcePool->ReleaseTexture(&gpuTex.m_pTextureHandle);
		}
	}

	delete[] m_perGPU;
}
