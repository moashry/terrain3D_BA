/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__TERRAIN_H__
#define __TUM3D__TERRAIN_H__

#include <vector>
#include <list>

#include "global.h"

#include "box.h"
#include "geometrydecompressor.h"
#include "occlusionculling.h"
#include "tiletree.h"

#include "tileRendering.h"
#include "tileRenderingBB.h"

//Forward Declaration
class Profiler;

class DataLoader;
class ResourcePool;
class Tile;


class FrustumCulling
{
public:
	void SetParameters(const RawMat4f pmProjectionMatrix);
	void Update(const RawMat4f pmViewMatrix);
	bool IsVisible(const Box* pBox);

private:
	tum3D::Mat4f m_mProjectionMatrix;
	tum3D::Vec4f m_Planes[6];
};


class LOD
{
public:
	void SetParameters(uint uiViewportHeight, float fFovy, float fAspectRatio, float fZNear, float fScreenSpaceError, float fLevelZeroWorldSpaceError);
	void Update(const RawVec3f pvPosition, const RawVec3f pvViewDirection);
	float ComputeLevel(const Box* pBox);

private:
	float			m_f;
	tum3D::Vec3f	m_vPosition;
	tum3D::Vec3f	m_vNormalizedViewDirection;
	float			m_fZNear;
};


class Terrain
{
	friend class DataLoader;
	friend class Tile;
public:
	Terrain(DataLoader* pDataLoader, uint uiInitNumVBs, uint uiInitNumTextures, float fLevelZeroTileExtent, uint uiNumLevels, float fMinVerticalTolerance, TEXTURE_COMPRESSION eTextureCompression);

	bool Create(Renderer::Device* pDevice);
	void SafeRelease();

	~Terrain();

	void SetParameters(const RawMat4f pmProjection, uint uiViewportHeight, float fFovy, float fAspectRatio, float fZNear, float fScreenSpaceError);
	void Update(const RawMat4f pmView, const RawVec3f pvPosition, const RawVec3f pvViewDirection, bool bOcclusionCulling);
	void UpdateViewOnly(const RawMat4f pmView); // F�r "Static"-View, aktualisiert lediglich die View-Matrix
	void Render(bool bDrawBBox, bool bSatModify);
	void SetSaturation(float fSaturation) { m_fSaturation = fSaturation; m_tileRenderingEffect.fSaturation.Set(m_fSaturation); };

	uint GetStatGeoSize() { return m_uiStatGeoSize; }
	uint GetStatTexSize() { return m_uiStatTexSize; }
	uint GetStatNumPagedInTiles() { return m_uiStatNumPagedInTiles; }
	uint GetStatNumVisibleTiles() { return static_cast<uint>(m_pTilesToRender.size()); }
	uint GetStatNumTriangles() { return m_uiStatNumTriangles; }
	ullong GetStatNumTrianglesRendered() { return m_ui64StatNumTrianglesRendered; }
	ullong GetStatNumTilesRendered() { return m_ui64StatNumTilesRendered; }
	ullong GetStatNumTilesTransferred() { return m_ui64StatNumTilesTransferred; }
	ullong GetStatNumBytesTransferred() { return m_ui64StatNumBytesTransferred; }
	ullong GetStatNumBytesTransferredDecompressed() { return m_ui64StatNumBytesTransferredDecompressed; }

	void PageOutAllAndFlushCache(); // Entfernt alle Tiles aus dem Grafikspeicher (Tiles werden tats�chlich entfernt, kein Caching!)

	void SetMaxLevel(uint uiMaxLevel) { m_uiMaxLevel = uiMaxLevel; }


	void SetProfiler(Profiler* pProfiler) { m_pProfiler = pProfiler; }

private:
	void Update(Tile* pTile);
	void PageOutDescendantTiles(Tile* pTile);

	DataLoader*						m_pDataLoader;

	uint							m_uiGPUIndex;

	ResourcePool*					m_pResourcePool;
	FrustumCulling					m_frustumCulling;
	LOD								m_LOD;
	OcclusionCulling				m_occlusionCulling; //TODO remove? [20/4/2010 Marc Treib]
	GeometryDecompressor			m_geometryDecompressor;


	tum3D::Vec3f					m_vPosition;
	tum3D::Mat4f					m_mView;


	std::vector<Tile*>				m_pTilesToPageOut;
	std::vector<Tile*>				m_pTilesToPageIn;
	std::vector<Tile*>				m_pTilesToRender;

	uint							m_uiStatNumPagedInTiles;
	uint							m_uiStatGeoSize;
	uint							m_uiStatTexSize;
	uint							m_uiStatNumTriangles;
	ullong							m_ui64StatNumTrianglesRendered;
	ullong							m_ui64StatNumTilesRendered;
	ullong							m_ui64StatNumTilesTransferred;
	ullong							m_ui64StatNumBytesTransferred;
	ullong							m_ui64StatNumBytesTransferredDecompressed;

	uint							m_uiNumLevels;
	uint							m_uiMaxLevel;
	float							m_fMinVerticalTolerance;
	TEXTURE_COMPRESSION				m_eTextureCompression;

	std::vector<float>				m_fTileExtents;

	bool							m_bOcclusionCulling;

	float							m_fSaturation;

	Renderer::Device*				m_pDevice;
	tileRendering					m_tileRenderingEffect;
	tileRenderingBB					m_tileRenderingBBEffect;

	Renderer::Buffer*				m_pVertexBufferBB;
	Renderer::Buffer*				m_pIndexBufferBB;

	uint*							m_puiGeometryDecompressionBuffer;


	Profiler*						m_pProfiler;
};


#endif
