/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__DATALOADER_H__
#define __TUM3D__DATALOADER_H__

#include <list>
#include <map>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <windows.h>

#include "global.h"

#include "box.h"
#include "terrainConfiguration.h"
#include "tiletree.h"
#include "Vec.h"


const uint uiNUM_LOADING_SLOTS = 4;

// Forward Declarations
class Profiler;

class Terrain;

struct Page;
struct PageTree;
struct PageDataRequest;

class FilePool;
struct FilePoolFileDesc;


struct ViewInfo {
	uint     viewIndex;
	RawVec3f position;
	RawVec3f viewDirection;
};
typedef std::vector<ViewInfo> ViewInfoVector;


class DataLoader
{
public:
	DataLoader(float fLevelZeroTileExtent, uint uiNumLevels, const std::vector<FileDesc>& fileDescs, const std::vector<TreeDesc>& treeDescs, uint uiNumOpenFilesLimit, DWORDLONG dwlSystemMemoryLimit);
	~DataLoader();

	// get a new gpu index. may only be called before the first Update call! (done automatically by the Terrain)
	uint GetNewGPUIndex();

	// get a new view index (can be used when calling ComputeLoadingRadiuses) 
	uint GetNewViewIndex();

	void ComputeLoadingRadiuses(uint uiViewportHeight, float fFovy, float fAspectRatio, float fZFar, float fScreenSpaceError, float fPrefetchingFactor, uint viewIndex = 0);

	void SetMaxLevel(uint uiMaxLevel) { m_uiMaxLevel = uiMaxLevel; }

	void Update(const ViewInfoVector &viewInfos, bool bExecuteAllIO = false);
	// Bei bExecuteAllIO == true kehrt die Funktion erst zurück, wenn alle IO Requests abgearbeitet sind

	void Render(Terrain* pTerrain); // Stellt die Bounding Boxes der Pages dar

	const TreesInfo& GetTreesInfo() const;
	TileTree* GetTileTree(int iX, int iY);
	const TileTree* GetTileTree(int iX, int iY) const;

	bool GetHeight(float fX, float fY, float* pfHeight) const;

	bool ComputeRayTerrainIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, tum3D::Vec3f* pvIntersectionPoint) const;

	uint GetStatRequestQueueSize() const;
	uint GetStatNumPages() const;
	uint GetStatNumTiles() const;
	ullong GetStatNumBytesTransferred() const;

	DWORDLONG GetSystemMemoryUsage() const;
	uint GetStatUnloadQueueSize() const;


	void SetProfiler(Profiler* pProfiler) { m_pProfiler = pProfiler; }

private:
	bool IsWithinLoadingRadius(const Box* pBox, uint uiLevel);

	void Update(Page* page);
	bool UnloadPageSubtree(Page* pPage);
	void CommitPageDataRequest(PageDataRequest* pPageDataRequest);
	std::vector<PageDataRequest*> GetPageDataRequests(uint uiNumRequests);
	void SynchronizeAsyncIO();

	void ExecutePendingIO();
	void ExecuteRequestQueueIO();

	void Render(Terrain* pTerrain, Page* pPage);

	void AddTiles(Page* pPage);
	bool RemoveTiles(Page* pPage, bool bForce = false);

	float ComputePriority(const Box* pBox);

	void CleanUp(Page* pPage);

	struct TileSortItem
	{
		Tile*	m_pTile;
		float	m_fDistance;
		bool	operator <(const TileSortItem& other) { return m_fDistance < other.m_fDistance; }
	};

	static void ComputeTilesToTestForIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, std::vector<TileSortItem>& tilesToTest, Tile* pTile);
	static bool ComputeRayBoxIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, const Box* pBox, float* pfDistance);


	std::vector<std::vector<PageTree*> >	m_pPageTrees;
	std::vector<std::vector<TileTree*> >	m_pTileTrees;
	TreesInfo						m_treesInfo;
	int								m_iTreeMinX, m_iTreeMinY, m_iTreeMaxX, m_iTreeMaxY;
	int								m_iTreePositionX, m_iTreePositionY;
	bool							m_bUpdateTrees;

	uint							m_uiNumLevels;
	uint							m_uiMaxLevel;

	std::vector<float>				m_fTileExtents;


	uint							m_uiNumGPUs;


	struct View {
		tum3D::Vec3f position;
		tum3D::Vec3f normalizedViewDirection;

		std::vector< float > fLoadingRadiusesSquared;
		//TODO uint uiLoadingRadiusLevelZeroInPageTrees; = static_cast<unsigned int>(ceil(sqrt(fLoadingRadiusesSquared[0]) / m_fTileExtents[0]));
	};
	std::vector<View>				m_views;

	std::vector<std::list<PageDataRequest*> >	m_requestQueue;
	uint							m_uiStatRequestQueueSize;

	std::list<Page*>				m_pageCache;

	DWORDLONG						m_dwlSystemMemoryLimit;
	DWORDLONG						m_dwlSystemMemoryUsage;

	uint							m_uiStatNumPages;
	uint							m_uiStatNumTiles;

	DWORD							m_dwPageSize;

	ullong							m_ui64StatNumBytesTransferred;

#ifdef _DEBUG
	llong							m_i64NumObjectsCreated;
	llong							m_i64NumObjectsDestroyed;
	llong							m_i64NumObjectsCreatedLoaderThread;
#endif

	PageDataRequest*				m_loadingSlots[uiNUM_LOADING_SLOTS];
	OVERLAPPED						m_overlappeds[uiNUM_LOADING_SLOTS];
	HANDLE							m_hCompletionEvents[uiNUM_LOADING_SLOTS];

	FilePool*						m_pFilePool;

	std::list<PageTree*>			m_unloadQueue;
			// Das Auslagern eines Page Trees kann evtl. nicht komplett erfolgen, da ein
			// Request innerhalb des Page Trees gerade in Bearbeitung sein könnte
			// Daher wird ein Page Tree zunächst in die Unload Queue eingetragen,
			// und das Auslagern ggf. mehrfach wiederholt


	Profiler*						m_pProfiler;
};


inline const TreesInfo& DataLoader::GetTreesInfo() const
{
	return m_treesInfo;
}

inline TileTree* DataLoader::GetTileTree(int iX, int iY)
{
	return m_pTileTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
}

inline const TileTree* DataLoader::GetTileTree(int iX, int iY) const
{
	return m_pTileTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
}

inline uint DataLoader::GetStatRequestQueueSize() const
{
	return m_uiStatRequestQueueSize;
}

inline uint DataLoader::GetStatNumPages() const
{
	return m_uiStatNumPages;
}

inline uint DataLoader::GetStatNumTiles() const
{
	return m_uiStatNumTiles;
}

inline ullong DataLoader::GetStatNumBytesTransferred() const
{
	return m_ui64StatNumBytesTransferred;
}

inline DWORDLONG DataLoader::GetSystemMemoryUsage() const
{
	return m_dwlSystemMemoryUsage;
}

inline uint DataLoader::GetStatUnloadQueueSize() const
{
	return static_cast<uint>(m_unloadQueue.size());
}

#endif
