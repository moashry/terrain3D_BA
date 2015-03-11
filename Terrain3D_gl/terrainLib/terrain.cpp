/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/


/*
TODO Wenn im statischen Modus das Benchmarken beendet wird, stürzt das Programm ab
Grund: Die Tiles in m_pTilesToRender sind nicht im Grafikspeicher
*/

#include "terrain.h"

#include <algorithm>

#include "profiler/profiler.hpp"

#include "debug.h"
#include "dataloader.h"
#include "resourcepool.h"
#include "tile.h"


void FrustumCulling::SetParameters(const RawMat4f pmProjectionMatrix)
{
	m_mProjectionMatrix = pmProjectionMatrix;
}

void FrustumCulling::Update(const RawMat4f pmViewMatrix)
{
	tum3D::Mat4f mViewProjectionMatrix;
	mViewProjectionMatrix = tum3D::Mat4f( pmViewMatrix ) * m_mProjectionMatrix;

	// Bereche die Clip-Ebenen im World Space
	// Die Normalen zeigen in das Innere des View Frustums

	// Berechne die rechte Clip-Ebene
	m_Planes[0] = -mViewProjectionMatrix.getCol(0) + mViewProjectionMatrix.getCol(3);
	// Berechne die linke Clip-Ebene
	m_Planes[1] = mViewProjectionMatrix.getCol(0) + mViewProjectionMatrix.getCol(3);
	// Berechne die obere Clip-Ebene
	m_Planes[2] = -mViewProjectionMatrix.getCol(1) + mViewProjectionMatrix.getCol(3);
	// Berechne die untere Clip-Ebene
	m_Planes[3] = mViewProjectionMatrix.getCol(1) + mViewProjectionMatrix.getCol(3);
	// Berechne die hintere Clip-Ebene
	m_Planes[4] = -mViewProjectionMatrix.getCol(2) + mViewProjectionMatrix.getCol(3);
	// Berechne die vordere Clip-Ebene
	m_Planes[5] = mViewProjectionMatrix.getCol(2) + mViewProjectionMatrix.getCol(3);
}

bool FrustumCulling::IsVisible(const Box* pBox)
{
	tum3D::Vec3f vCenter = pBox->m_vCenter;
	tum3D::Vec3f vHalfExtent = pBox->m_vHalfExtent;

	for (uint uiPlane = 0; uiPlane < 6; uiPlane++)
	{
		tum3D::Vec4f plane = m_Planes[uiPlane];
		if (
			tum3D::dotProd( plane, tum3D::Vec4f( vCenter, 1.0f ) )
			<= -tum3D::dotProd( vHalfExtent, tum3D::Vec3f( plane ).abs() )
			)
		{
			return false;
		}
	}
	return true;

}


const float fLOG2 = log(2.0f);

void LOD::SetParameters(uint uiViewportHeight, float fFovy, float fAspectRatio, float fZNear, float fScreenSpaceError, float fLevelZeroWorldSpaceError)
{
	m_f = fLevelZeroWorldSpaceError / (2.0f * tan(DEG2RAD(fFovy / 2.0f)) * fScreenSpaceError / static_cast<float>(uiViewportHeight));

	m_fZNear = fZNear;
}


void LOD::Update(const RawVec3f pvPosition, const RawVec3f pvViewDirection)
{
	m_vPosition = pvPosition;
	m_vNormalizedViewDirection = pvViewDirection;
	tum3D::normalize( m_vNormalizedViewDirection );
}


float LOD::ComputeLevel(const Box* pBox)
{
	tum3D::Vec3f vDiff = pBox->m_vCenter - m_vPosition;
	tum3D::Vec3f vHalfExtent = pBox->m_vHalfExtent;
	float zcenter = tum3D::dotProd( m_vNormalizedViewDirection, vDiff );
	float reff = tum3D::dotProd( vHalfExtent, m_vNormalizedViewDirection.abs() );
	
	float z = std::max(m_fZNear, zcenter - reff);
	
	return log(m_f / z) / fLOG2; // Aufrunden ist nicht notwendig, weil der Wert mit ganzen Zahlen verglichen wird: k >= ceil(x) gdw. k >= x, falls k ganzzahlig
}


Terrain::Terrain(DataLoader* pDataLoader, uint uiInitNumVBs, uint uiInitNumTextures, float fLevelZeroTileExtent, uint uiNumLevels, float fMinVerticalTolerance, TEXTURE_COMPRESSION eTextureCompression)
: m_pDataLoader(pDataLoader),
  m_pVertexBufferBB(0), m_pIndexBufferBB(0),
  m_fSaturation(1.0f), m_pDevice(0),
  m_pProfiler(0)
{
	m_uiGPUIndex = m_pDataLoader->GetNewGPUIndex();

#if TILE_SIZE == 512
	uint uiVBBlockLevelZeroSize = 4 * 387;
	uint uiNumVBBlockLevels = 13;
#elif TILE_SIZE == 256
	uint uiVBBlockLevelZeroSize = 8 * 195;
	uint uiNumVBBlockLevels = 11;
#else
	#error Unsupported tile size
#endif
	m_pResourcePool = new ResourcePool(uiVBBlockLevelZeroSize, uiNumVBBlockLevels, uiInitNumVBs, eTextureCompression, uiTILE_SIZE, uiNUM_MIPMAP_LEVELS, uiInitNumTextures);

	m_uiNumLevels = uiNumLevels;
	m_uiMaxLevel = m_uiNumLevels - 1;
	m_fMinVerticalTolerance = fMinVerticalTolerance;
	m_eTextureCompression = eTextureCompression;
	m_fTileExtents.resize(m_uiNumLevels);
	float fTileExtent = fLevelZeroTileExtent;
	for (uint uiLevel = 0; uiLevel < m_uiNumLevels; uiLevel++)
	{
		m_fTileExtents[uiLevel] = fTileExtent;
		fTileExtent /= 2.0f;
	}

	m_uiStatNumPagedInTiles = 0;

	m_uiStatGeoSize = 0;
	m_uiStatTexSize = 0;
	m_ui64StatNumTrianglesRendered = 0;
	m_ui64StatNumTilesTransferred = 0;
	m_ui64StatNumBytesTransferred = 0;
	m_ui64StatNumBytesTransferredDecompressed = 0;

	m_puiGeometryDecompressionBuffer = new uint[(uiTILE_SIZE * uiTILE_SIZE * 2 + 4 * uiTILE_SIZE * 2) * 3];
}


bool Terrain::Create(Renderer::Device* pDevice)
{
	m_pDevice = pDevice;

	V_RETURN( m_pResourcePool->Create( m_pDevice ) );

#ifdef INDEXED_DRAWING
    V_RETURN( m_tileRenderingEffect.Create( "#define INDEXED_DRAWING" ) );
#else
    V_RETURN( m_tileRenderingEffect.Create() );
#endif
	V_RETURN( m_tileRenderingBBEffect.Create() );

	// Bounding Boxes Vertex Buffer anlegen
	const VertexBB verticesBB[] =
	{
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f }
	};

	Renderer::Buffer::Create( m_pVertexBufferBB, 8 * sizeof(VertexBB), GL_STATIC_DRAW, verticesBB );

	// Bounding Boxes Index Buffer anlegen
	const uint indicesBB[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

	Renderer::Buffer::Create( m_pIndexBufferBB, sizeof(indicesBB), GL_STATIC_DRAW, indicesBB );

	V_RETURN( m_geometryDecompressor.Create( m_pDevice ) );

	m_tileRenderingEffect.fSaturation.Set(m_fSaturation);

	return true;
}


void Terrain::SetParameters(const RawMat4f pmProjection, uint uiViewportHeight, float fFovy, float fAspectRatio, float fZNear, float fScreenSpaceError)
{
	m_frustumCulling.SetParameters(pmProjection);
	m_LOD.SetParameters(uiViewportHeight, fFovy, fAspectRatio, fZNear, fScreenSpaceError, m_fTileExtents[0] / float(uiTILE_SIZE));
	m_occlusionCulling.SetParameters(fZNear);

	m_tileRenderingEffect.mProjection.Set(pmProjection);
	m_tileRenderingBBEffect.mProjection.Set(pmProjection);
}


void Terrain::Render(bool bDrawBBox, bool bSatModify)
{
	ScopedProfileSample sample(m_pProfiler, "Terrain::Render");

	m_uiStatNumTriangles = 0;

	for (uint ui = 0; ui < m_pTilesToRender.size(); ui++)
	{
		Tile* pTile = m_pTilesToRender[ui];
		pTile->Render(this, bDrawBBox, bSatModify);
	}

	m_ui64StatNumTilesRendered += static_cast<ullong>(m_pTilesToRender.size());

}


struct TileTreeSortItem
{
	TileTree*		m_pTileTree;
	uint			m_uiDistance;
};


inline bool TileTreeSortItemComp(const TileTreeSortItem& item1, const TileTreeSortItem& item2)
{
	return item1.m_uiDistance < item2.m_uiDistance;
}


void Terrain::Update(const RawMat4f pmView, const RawVec3f pvPosition, const RawVec3f pvViewDirection, bool bOcclusionCulling)
{
	ScopedProfileSample sample(m_pProfiler, "Terrain::Update");

	m_frustumCulling.Update(pmView);
	m_LOD.Update(pvPosition, pvViewDirection);
	m_occlusionCulling.Update(pvPosition, pvViewDirection);

	m_mView = pmView;

	m_vPosition = pvPosition;
	m_bOcclusionCulling = bOcclusionCulling;

	m_pTilesToPageOut.clear();
	m_pTilesToPageIn.clear();
	m_pTilesToRender.clear();

	int iPositionX = static_cast<int>(floor(m_vPosition.x() / m_fTileExtents[0]));
	int iPositionY = static_cast<int>(floor(m_vPosition.y() / m_fTileExtents[0]));

	std::vector<TileTreeSortItem> tileTrees;

	const TreesInfo& info = m_pDataLoader->GetTreesInfo();
	for (int iY = info.m_iTreeY0; iY <= info.m_iTreeY1; iY++)
	{
		for (int iX = info.m_iTreeX0; iX <= info.m_iTreeX1; iX++)
		{
			TileTree* pTileTree = m_pDataLoader->GetTileTree(iX, iY);
			if (pTileTree != 0)
			{
				if (pTileTree->m_root->GetChildren()[0] != 0)
				{
					TileTreeSortItem item;
					item.m_pTileTree = pTileTree;
					uint dx = static_cast<uint>(abs(iX - iPositionX));
					uint dy = static_cast<uint>(abs(iY - iPositionY));
					item.m_uiDistance = dx + dy; // 1-Norm
					tileTrees.push_back(item);
				}
			}
		}
	}

	std::sort(tileTrees.begin(), tileTrees.end(), TileTreeSortItemComp);

	for (uint ui = 0; ui < tileTrees.size(); ui++)
	{
		Update(tileTrees[ui].m_pTileTree->m_root->GetChildren()[0]);
	}

	for (uint ui = 0; ui < m_pTilesToPageOut.size(); ui++)
	{
		m_pTilesToPageOut[ui]->PageOut(this);
	}
	m_uiStatNumPagedInTiles -= static_cast<uint>(m_pTilesToPageOut.size());

	for (uint ui = 0; ui < m_pTilesToPageIn.size(); ui++)
	{
		m_pTilesToPageIn[ui]->PageIn(this);
	}
	m_uiStatNumPagedInTiles += static_cast<uint>(m_pTilesToPageIn.size());
	m_ui64StatNumTilesTransferred += static_cast<uint>(m_pTilesToPageIn.size());

	m_geometryDecompressor.FlushJobs();

}


void Terrain::UpdateViewOnly(const RawMat4f pmView)
{
	m_mView = pmView;
}


void Terrain::Update(Tile* pTile)
{
	if (pTile->m_uiStripHeadersSize != 0)
	{
		if (m_frustumCulling.IsVisible(pTile->GetBoundingBox()))
		{
			// Die Kachel ist sichtbar
			float fLOD = m_LOD.ComputeLevel(pTile->GetBoundingBox());
			bool bIsLODSufficient = (static_cast<float>(pTile->GetLevel()) >= fLOD);
			if (bIsLODSufficient || pTile->m_uiLevel >= m_uiMaxLevel)
			{
				// Das Level dieser Kachel reicht aus oder maximales Level erreicht
				if (!pTile->IsPagedIn(m_uiGPUIndex))
				{
					m_pTilesToPageIn.push_back(pTile);
				}
				if (!m_bOcclusionCulling || pTile->m_uiStripHeadersSize == 0 || m_occlusionCulling.IsVisible(&(pTile->m_BBox)))
				{
					m_pTilesToRender.push_back(pTile);
					pTile->m_perGPU[m_uiGPUIndex].m_bIsLODSufficient = bIsLODSufficient;
					pTile->m_perGPU[m_uiGPUIndex].m_fRequiredLOD = fLOD;
				}

				if (pTile->GetChildren()[0] != 0)
				{
					for (uint uiChild = 0; uiChild < 4; uiChild++)
					{
						PageOutDescendantTiles(pTile->GetChildren()[uiChild]);
					}
				}
			}
			else
			{
				// Das Level dieser Kachel reicht nicht aus
				// Prüfe, ob die Kachel Kinder hat
				// Beachte: Wenn ein Kind vorhanden ist, sind alle vier Kinder vorhanden
				if (pTile->GetChildren()[0] != 0)
				{
					// Kinder sind vorhanden
					if (pTile->IsPagedIn(m_uiGPUIndex))
					{
						m_pTilesToPageOut.push_back(pTile);
					}

					assert(pTile->GetChildren()[0] != 0);
					assert(pTile->GetChildren()[1] != 0);
					assert(pTile->GetChildren()[2] != 0);
					assert(pTile->GetChildren()[3] != 0);

					if (m_vPosition.x() > pTile->m_BBox.m_vCenter.x())
					{
						if (m_vPosition.y() > pTile->m_BBox.m_vCenter.y())
						{
							Update(pTile->GetChildren()[3]);
							Update(pTile->GetChildren()[1]);
							Update(pTile->GetChildren()[2]);
							Update(pTile->GetChildren()[0]);
						}
						else
						{
							Update(pTile->GetChildren()[1]);
							Update(pTile->GetChildren()[0]);
							Update(pTile->GetChildren()[3]);
							Update(pTile->GetChildren()[2]);
						}
					}
					else
					{
						if (m_vPosition.y() > pTile->m_BBox.m_vCenter.y())
						{
							Update(pTile->GetChildren()[2]);
							Update(pTile->GetChildren()[0]);
							Update(pTile->GetChildren()[3]);
							Update(pTile->GetChildren()[1]);
						}
						else
						{
							Update(pTile->GetChildren()[0]);
							Update(pTile->GetChildren()[1]);
							Update(pTile->GetChildren()[2]);
							Update(pTile->GetChildren()[3]);
						}
					}

				}
				else
				{
					// Kinder sind nicht vorhanden
					// Nehme diese Kachel
					if (!pTile->IsPagedIn(m_uiGPUIndex))
					{
						m_pTilesToPageIn.push_back(pTile);
					}
					if (!m_bOcclusionCulling || pTile->m_uiStripHeadersSize == 0 || m_occlusionCulling.IsVisible(&(pTile->m_BBox)))
					{
						m_pTilesToRender.push_back(pTile);
						pTile->m_perGPU[m_uiGPUIndex].m_bIsLODSufficient = false;
						pTile->m_perGPU[m_uiGPUIndex].m_fRequiredLOD = fLOD;
					}
				}
			}

		}
		else
		{
			// Die Kachel ist nicht sichtbar
			PageOutDescendantTiles(pTile);
		}
	} // if (pTile->m_uiStripHeadersSize != 0)
}


void Terrain::PageOutDescendantTiles(Tile* pTile)
{
	if (pTile->AreAnyDescendantTilesPagedIn(m_uiGPUIndex))
	{
		if (pTile->IsPagedIn(m_uiGPUIndex))
		{
			m_pTilesToPageOut.push_back(pTile);
		}
		if (pTile->GetChildren()[0] != 0)
		{
			for (uint uiChild = 0; uiChild < 4; uiChild++)
			{
				PageOutDescendantTiles(pTile->GetChildren()[uiChild]);
			}
		}
	}
}


void Terrain::PageOutAllAndFlushCache()
{
	m_pTilesToPageOut.clear();

	const TreesInfo& info = m_pDataLoader->GetTreesInfo();
	for (int iY = info.m_iTreeY0; iY <= info.m_iTreeY1; iY++)
	{
		for (int iX = info.m_iTreeX0; iX <= info.m_iTreeX1; iX++)
		{
			TileTree* pTileTree = m_pDataLoader->GetTileTree(iX, iY);
			if (pTileTree != 0)
			{
				Tile* pTile = pTileTree->m_root->GetChildren()[0];
				if (pTile != 0)
				{
					PageOutDescendantTiles(pTile);
				}
			}
		}
	}

	for (uint ui = 0; ui < m_pTilesToPageOut.size(); ui++)
	{
		Tile* pTile = m_pTilesToPageOut[ui];
		pTile->PageOut(this);
	}
	m_uiStatNumPagedInTiles -= static_cast<uint>(m_pTilesToPageOut.size());

	m_pResourcePool->FlushCache();

}


void Terrain::SafeRelease()
{
	PageOutAllAndFlushCache();

	m_pResourcePool->SafeRelease();

	m_geometryDecompressor.SafeRelease();

	SAFE_DELETE( m_pIndexBufferBB );
	SAFE_DELETE( m_pVertexBufferBB );

	m_tileRenderingEffect.SafeRelease();
	m_tileRenderingBBEffect.SafeRelease();

	m_pDevice = 0;
}


Terrain::~Terrain()
{
	delete m_pResourcePool;
	delete[] m_puiGeometryDecompressionBuffer;
}
