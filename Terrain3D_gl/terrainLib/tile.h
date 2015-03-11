/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__TILE_H__
#define __TUM3D__TILE_H__

#include <vector>
#include <list>

#include "Vec.h"

#include "global.h"
#include "box.h"

//Forward Declaration
class Terrain;
struct TextureHandle;
struct VBHandle;

class Tile
{
	friend class DataLoader;
	friend class Terrain;
public:
	Tile(uint uiNumGPUs);
	~Tile();

	Tile*const* GetChildren();
	const Box* GetBoundingBox();
	uint GetLevel();

	void PageIn(Terrain* pTerrain);
	void PageOut(Terrain* pTerrain);
	bool IsPagedIn(uint uiGPUIndex);
	bool IsAnyGPUPagedIn();
	bool AreAnyDescendantTilesPagedIn(uint uiGPUIndex);
	void Render(Terrain* pTerrain, bool bDrawBBox, bool bSatModify);

	uint GetStatNumTriangles();
	uint GetStatGeoSize();
	uint GetStatTexSize();

	bool GetHeight(float fX, float fY, float* pfHeight) const;
	bool ComputeRayTerrainIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, tum3D::Vec3f* pvIntersectionPoint) const;
	void WriteASY() const;

	void DecompressGeometryOnCPU(uint* puiDecompressionBuffer) const;

private:
	uint			m_uiX, m_uiY, m_uiLevel, m_uiChild;
	Tile*			m_parent;
	Tile*			m_children[4];
	Box				m_BBox;

	float			m_fMinHeight;
	float			m_fMaxHeight;
	uint			m_uiStripHeadersSize;
	byte*			m_pStripHeaders;
	uint			m_uiStripDataFormat;
	uint			m_uiStripDataSize;
	byte*			m_pStripData;
	uint			m_uiNumTriangles;
	uint			m_uiNumBoundaryEdges;
	uint			m_uiTextureDataSize;
	byte*			m_pTextureData;

	Tile*			m_pTextureTile;
	// Die Textur einer Vorfahren-Kachel kann von den Nachfahren geshared werden
	// m_pTextureTile gibt die Kachel an, deren Textur diese Kachel verwendet
	// (Wenn die Kachel ihre eigene Textur besitzt, ist m_pTextureTile == this)
	tum3D::Vec2f	m_vTexCoordOffset;
	tum3D::Vec2f	m_vTexCoordSize;

	uint			m_uiStatGeoSize;
	uint			m_uiStatTexSize;

	uint ReadData(uint index) const;

	struct PerGPU
	{
		bool			m_bIsPagedIn;
		uint			m_uiPagedInCounter; // Zählt von 0 bis 5; Beiträge: 1 falls Tile paged in, 1 für jede Child Tile deren Subtree eine paged in Tile enthält

		TextureHandle*	m_pTextureHandle;
		VBHandle*		m_pVBHandle;

		bool			m_bIsLODSufficient;
		float			m_fRequiredLOD;
	};
	PerGPU*			m_perGPU;
	uint			m_uiNumGPUs;

	static const uint uiNUM_SAMPLER_STATES = 16;
};


inline Tile*const* Tile::GetChildren()
{
	return m_children;
}

inline const Box* Tile::GetBoundingBox()
{
	return &m_BBox;
}

inline uint Tile::GetLevel()
{
	return m_uiLevel;
}

inline bool Tile::IsPagedIn(uint uiGPUIndex)
{
	assert(uiGPUIndex < m_uiNumGPUs);

	return m_perGPU[uiGPUIndex].m_bIsPagedIn;
}

inline bool Tile::IsAnyGPUPagedIn()
{
	for (uint i = 0; i < m_uiNumGPUs; i++)
	{
		if (IsPagedIn(i))
		{
			return true;
		}
	}

	return false;
}

inline bool Tile::AreAnyDescendantTilesPagedIn(uint uiGPUIndex)
{
	assert(uiGPUIndex < m_uiNumGPUs);

	assert(0 <= m_perGPU[uiGPUIndex].m_uiPagedInCounter && m_perGPU[uiGPUIndex].m_uiPagedInCounter <= 5);
	return m_perGPU[uiGPUIndex].m_uiPagedInCounter != 0;
}

#endif
