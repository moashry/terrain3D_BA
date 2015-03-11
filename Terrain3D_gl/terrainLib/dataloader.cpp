/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "dataloader.h"

#include "profiler/profiler.hpp"

#include "debug.h"
#include "terrain.h"
#include "tile.h"
#include "tiletree.h"

using namespace tum3D;
using namespace Renderer;

#undef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))

#undef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))


// File Header
const char*const pcFILE_SIGNATURE = "TUM3D TERRAIN";
const uint uiFILE_SIGNATURE_LENGTH = static_cast<uint>(strlen(pcFILE_SIGNATURE));
const uint uiFILE_VERSION_MAJOR = 1;
const uint uiFILE_VERSION_MINOR = 1;


struct Page
{
	uint			m_uiX, m_uiY, m_uiLevel, m_uiChild;
	PageTree*		m_pPageTree;
	Page*			m_parent;
	Page*			m_children[4];
	Page*			m_cachedChildren[4];
	Box				m_childrenBBoxes[4];

	bool			m_bAreChildrenAvailable[4];
	float			m_fChildrenMinHeights[4];
	float			m_fChildrenMaxHeights[4];
	llong			m_i64ChildrenFilePtrs[4];
	uint			m_uiChildrenSizes[4];

	byte*			m_pPageData;
	uint			m_uiOffset;

	PageDataRequest*	m_pPageDataRequest;

	Tile*			m_pTiles[uiPAGE_EXTENT_IN_TILES * uiPAGE_EXTENT_IN_TILES]; // Zeiger auf die Tiles der Page

	std::list<PageDataRequest*>::iterator m_requestQueueLocator;
	bool			m_bIsInRequestQueue;

	std::list<Page*>::iterator m_pageCacheLocator;
	bool			m_bIsInPageCache; // Wird (bisher) nicht gebraucht (gelesen)

};


struct PageTree
{
	int				m_iX, m_iY;
	uint			m_uiFileID;
	Page*			m_root;

	std::list<PageTree*>::iterator m_unloadQueueLocator;
	bool			m_bIsInUnloadQueue;
};


struct PageDataRequest
{
	float			m_fPriority;
	Page*			m_pPage;
	uint			m_uiFileID;
	HANDLE			m_hFile;
	llong			m_i64FilePointer;
	uint			m_uiSize;
	byte*			m_pPageData;
	uint			m_uiOffset;
};


struct FilePoolFileDesc
{
	char*			m_ptcFileName;
	HANDLE			m_hFile;
	uint			m_uiUsageCounter;
	std::list<FilePoolFileDesc*>::iterator m_openFileCacheLocator;
	bool			m_bIsInOpenFileCache;
};
// m_uiUsageCounter == 0 && !m_bIsInOpenFileCache => Datei ist geschlossen
// m_uiUsageCounter != 0 || m_bIsInOpenFileCache => Datei ist geöffnet

class FilePool
{
public:
	FilePool(uint uiNumOpenFilesLimit);
	~FilePool();
	void AddFile(uint uiFileID, const char* ptcFileName);
	HANDLE GetFileHandle(uint uiFileID);
	void ReleaseFileHandle(uint uiFileID);

private:
	DWORD				m_dwPageSize;
	byte*				m_pBuffer; // Puffer zum Einlesen der Signatur einer Datei

	OVERLAPPED			m_overlapped;
	HANDLE				m_hCompletionEvent;

	uint				m_uiNumOpenFilesLimit;
	uint				m_uiNumOpenFiles;

	std::map<uint, FilePoolFileDesc*>	m_files;
	
	std::list<FilePoolFileDesc*>		m_openFileCache;
};


FilePool::FilePool(uint uiNumOpenFilesLimit)
{

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	m_dwPageSize = systemInfo.dwPageSize;

	m_pBuffer = (byte*)VirtualAlloc(NULL, m_dwPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (m_pBuffer == NULL)
	{
		// VirtualAlloc ist fehlgeschlagen
		MessageBox(NULL, "VirtualAlloc failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}
	
	m_uiNumOpenFilesLimit = uiNumOpenFilesLimit;
	m_uiNumOpenFiles = 0;

	m_hCompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}


void FilePool::AddFile(uint uiFileID, const char* ptcFileName)
{

	FilePoolFileDesc* pFileDesc = new FilePoolFileDesc;
	ZeroMemory(pFileDesc, sizeof(FilePoolFileDesc));
	pFileDesc->m_ptcFileName = new char[MAX_STRING_SIZE];
	strcpy_s(pFileDesc->m_ptcFileName, MAX_STRING_SIZE, ptcFileName);
	std::map<uint, FilePoolFileDesc*>::iterator p = m_files.find(uiFileID);
	if (p != m_files.end())
	{
		delete[] p->second->m_ptcFileName;
		delete p->second;
	}
	m_files.insert(std::pair<uint, FilePoolFileDesc*>(uiFileID, pFileDesc));

}


HANDLE FilePool::GetFileHandle(uint uiFileID)
{

	std::map<uint, FilePoolFileDesc*>::iterator p = m_files.find(uiFileID);
	if (p == m_files.end())
	{
		char str[MAX_STRING_SIZE];
		sprintf_s(str, MAX_STRING_SIZE, "File ID %u not found\n" __LOCATION__, uiFileID);
		MessageBox(NULL, str, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	FilePoolFileDesc* pFileDesc = p->second;

	if (pFileDesc->m_uiUsageCounter != 0)
	{
		pFileDesc->m_uiUsageCounter++;
		return pFileDesc->m_hFile;
	}

	if (pFileDesc->m_bIsInOpenFileCache)
	{
		m_openFileCache.erase(pFileDesc->m_openFileCacheLocator);
		pFileDesc->m_bIsInOpenFileCache = false;
		pFileDesc->m_uiUsageCounter++;
		return pFileDesc->m_hFile;
	}

	if (m_uiNumOpenFiles >= m_uiNumOpenFilesLimit && m_openFileCache.size() > 0)
	{
		FilePoolFileDesc* pFileToCloseDesc = m_openFileCache.front();
		m_openFileCache.pop_front();
		pFileToCloseDesc->m_bIsInOpenFileCache = false;
		CloseHandle(pFileToCloseDesc->m_hFile);
		m_uiNumOpenFiles--;
	}
	pFileDesc->m_hFile = CreateFile(pFileDesc->m_ptcFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
	if (pFileDesc->m_hFile == INVALID_HANDLE_VALUE) // Fehlerbehandlung
	{
		char str[MAX_STRING_SIZE];
		sprintf_s(str, MAX_STRING_SIZE, "Could not open file %s\n" __LOCATION__, pFileDesc->m_ptcFileName);
		MessageBox(NULL, str, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	// Prüfe Signatur der Datei
	ZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
	m_overlapped.Offset = 0;
	m_overlapped.OffsetHigh = 0;
	m_overlapped.hEvent = m_hCompletionEvent;

	DWORD dwNumBytesRead;
	BOOL bResult = ReadFile(pFileDesc->m_hFile, m_pBuffer, m_dwPageSize, &dwNumBytesRead, &m_overlapped);
	if (bResult)
	{
		// Read bereits fertig
		if (dwNumBytesRead < uiFILE_SIGNATURE_LENGTH * sizeof(char) + 2 * sizeof(uint)) // Bei einem Fehler
		{
			MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
			exit(-1);
		}
	}
	else
	{
		// Read noch nicht fertig
		DWORD error = GetLastError();
		if (error != ERROR_IO_PENDING) // Bei einem Fehler
		{
			MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
			exit(-1);
		}
		bResult = GetOverlappedResult(pFileDesc->m_hFile, &m_overlapped, &dwNumBytesRead, TRUE);
		if (!bResult || dwNumBytesRead < uiFILE_SIGNATURE_LENGTH * sizeof(char) + 2 * sizeof(uint)) // Bei einem Fehler
		{
			MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
			exit(-1);
		}
	}
	byte* ptr = m_pBuffer;
	char* pcSignature = reinterpret_cast<char*>(ptr);
	ptr += uiFILE_SIGNATURE_LENGTH * sizeof(char);
	uint uiVersionMajor = *reinterpret_cast<uint*>(ptr);
	ptr += sizeof(uint);
	uint uiVersionMinor = *reinterpret_cast<uint*>(ptr);
	ptr += sizeof(uint);
	if(strncmp(pcSignature, pcFILE_SIGNATURE, uiFILE_SIGNATURE_LENGTH) != 0
		|| uiVersionMajor > uiFILE_VERSION_MAJOR
		|| uiVersionMajor == uiFILE_VERSION_MAJOR && uiVersionMinor > uiFILE_VERSION_MINOR)
	{
		MessageBox(NULL, "File signature check failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}
	if (uiVersionMajor >= 1 && uiVersionMinor >= 1)
	{
		uint uiTileSize = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
		if (uiTileSize != uiTILE_SIZE)
		{
			char str[MAX_STRING_SIZE];
			sprintf_s(str, MAX_STRING_SIZE, "Tile size of file %s is %u, but program is compiled for a tile size of %u\n" __LOCATION__,
				pFileDesc->m_ptcFileName, uiTileSize, uiTILE_SIZE);
			MessageBox(NULL, str, "ERROR", MB_OK | MB_ICONERROR);
			exit(-1);
		}
		uint uiPageExtentInTiles = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
		if (uiPageExtentInTiles != uiPAGE_EXTENT_IN_TILES)
		{
			char str[MAX_STRING_SIZE];
			sprintf_s(str, MAX_STRING_SIZE, "Page extent in tiles of file %s is %u, but program is compiled for a page extent in tiles of %u\n" __LOCATION__,
				pFileDesc->m_ptcFileName, uiPageExtentInTiles, uiPAGE_EXTENT_IN_TILES);
			MessageBox(NULL, str, "ERROR", MB_OK | MB_ICONERROR);
			exit(-1);
		}
	}

	m_uiNumOpenFiles++;
	pFileDesc->m_uiUsageCounter++;
	return pFileDesc->m_hFile;

}


void FilePool::ReleaseFileHandle(uint uiFileID)
{

	std::map<uint, FilePoolFileDesc*>::iterator p = m_files.find(uiFileID);
	if (p == m_files.end())
	{
		char str[MAX_STRING_SIZE];
		sprintf_s(str, MAX_STRING_SIZE, "File ID %u not found\n" __LOCATION__, uiFileID);
		MessageBox(NULL, str, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	FilePoolFileDesc* pFileDesc = p->second;

	assert(pFileDesc->m_uiUsageCounter != 0);

	pFileDesc->m_uiUsageCounter--;
	if (pFileDesc->m_uiUsageCounter == 0)
	{
		if (m_uiNumOpenFiles <= m_uiNumOpenFilesLimit)
		{
			m_openFileCache.push_back(pFileDesc);
			pFileDesc->m_openFileCacheLocator = m_openFileCache.end();
			pFileDesc->m_openFileCacheLocator--;
			pFileDesc->m_bIsInOpenFileCache = true;
		}
		else
		{
			// Der Cache ist leer, denn erst wenn der Cache leer ist, kann das Limit überschritten werden
			CloseHandle(pFileDesc->m_hFile);
			m_uiNumOpenFiles--;
		}
	}

}


FilePool::~FilePool()
{

	std::map<uint, FilePoolFileDesc*>::iterator p;
	for (p = m_files.begin(); p != m_files.end(); p++)
	{
		FilePoolFileDesc* pFileDesc = p->second;
		if (pFileDesc->m_uiUsageCounter != 0 || pFileDesc->m_bIsInOpenFileCache)
		{
			CloseHandle(pFileDesc->m_hFile);
		}
		delete[] pFileDesc->m_ptcFileName;
		delete pFileDesc;
	}

	CloseHandle(m_hCompletionEvent);

	BOOL bResult = VirtualFree(m_pBuffer, 0, MEM_RELEASE);
	if (!bResult)
	{
		// VirtualFree ist fehlgeschlagen
		MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

}


DataLoader::DataLoader(float fLevelZeroTileExtent, uint uiNumLevels, const std::vector<FileDesc>& fileDescs, const std::vector<TreeDesc>& treeDescs, uint uiNumOpenFilesLimit, DWORDLONG dwlSystemMemoryLimit)
: m_uiNumGPUs(0), m_pProfiler(0)
{
	if (fileDescs.size() == 0 || treeDescs.size() == 0)
	{
		MessageBox(NULL, "No Files or Trees specified" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	m_pFilePool = new FilePool(uiNumOpenFilesLimit);
	for (uint ui = 0; ui < fileDescs.size(); ui++)
	{
		m_pFilePool->AddFile(fileDescs[ui].m_uiFileID, fileDescs[ui].m_ptcFileName.c_str());
	}

#ifdef _DEBUG
	m_i64NumObjectsCreated = 0;
	m_i64NumObjectsDestroyed = 0;
	m_i64NumObjectsCreatedLoaderThread = 0;
#endif

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	m_dwPageSize = systemInfo.dwPageSize;

	m_uiNumLevels = uiNumLevels;
	m_uiMaxLevel = m_uiNumLevels - 1;

	m_fTileExtents.resize(m_uiNumLevels);
	float fTileExtent = fLevelZeroTileExtent;
	for (uint uiLevel = 0; uiLevel < m_uiNumLevels; uiLevel++)
	{
		m_fTileExtents[uiLevel] = fTileExtent;
		fTileExtent /= 2.0f;
	}

	m_requestQueue.resize(m_uiNumLevels);
	m_uiStatRequestQueueSize = 0;

	
	m_dwlSystemMemoryLimit = dwlSystemMemoryLimit;
	m_dwlSystemMemoryUsage = 0;

	m_uiStatNumPages = 0;
	m_uiStatNumTiles = 0;

	for (uint uiSlot = 0; uiSlot < uiNUM_LOADING_SLOTS; uiSlot++)
	{
		m_loadingSlots[uiSlot] = NULL;
		m_hCompletionEvents[uiSlot] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	m_iTreeMinX = treeDescs[0].m_iX;
	m_iTreeMinY = treeDescs[0].m_iY;
	m_iTreeMaxX = m_iTreeMinX;
	m_iTreeMaxY = m_iTreeMinY;
	for (uint ui = 1; ui < treeDescs.size(); ui++)
	{
		int iX = treeDescs[ui].m_iX;
		int iY = treeDescs[ui].m_iY;
		if (iX < m_iTreeMinX) { m_iTreeMinX = iX; }
		else if (iX > m_iTreeMaxX) { m_iTreeMaxX = iX; }
		if (iY < m_iTreeMinY) { m_iTreeMinY = iY; }
		else if (iY > m_iTreeMaxY) { m_iTreeMaxY = iY; }
	}

	m_treesInfo.m_iTreeX0 = 0;
	m_treesInfo.m_iTreeY0 = 0;
	m_treesInfo.m_iTreeX1 = -1;
	m_treesInfo.m_iTreeY1 = -1;

	m_bUpdateTrees = true;

	m_treesInfo.m_uiNumTrees[0] = static_cast<uint>(m_iTreeMaxX - m_iTreeMinX + 1);
	m_treesInfo.m_uiNumTrees[1] = static_cast<uint>(m_iTreeMaxY - m_iTreeMinY + 1);

	m_pPageTrees.resize(m_treesInfo.m_uiNumTrees[0]);
	m_pTileTrees.resize(m_treesInfo.m_uiNumTrees[0]);
	for (uint uiX = 0; uiX < m_treesInfo.m_uiNumTrees[0]; uiX++)
	{
		m_pPageTrees[uiX].resize(m_treesInfo.m_uiNumTrees[1], NULL);
		m_pTileTrees[uiX].resize(m_treesInfo.m_uiNumTrees[1], NULL);
	}
	
	for (uint ui = 0; ui < treeDescs.size(); ui++)
	{
		uint uiX = static_cast<uint>(treeDescs[ui].m_iX - m_iTreeMinX);
		uint uiY = static_cast<uint>(treeDescs[ui].m_iY - m_iTreeMinY);

		m_pPageTrees[uiX][uiY] = new PageTree();
#ifdef _DEBUG
		m_i64NumObjectsCreated++;
#endif
		ZeroMemory(m_pPageTrees[uiX][uiY], sizeof(PageTree));
		m_pPageTrees[uiX][uiY]->m_iX = treeDescs[ui].m_iX;
		m_pPageTrees[uiX][uiY]->m_iY = treeDescs[ui].m_iY;
		m_pPageTrees[uiX][uiY]->m_uiFileID = treeDescs[ui].m_uiFileID;

		m_pPageTrees[uiX][uiY]->m_root = new Page(); // Dummy-Wurzel-Page erzeugen
#ifdef _DEBUG
		m_i64NumObjectsCreated++;
#endif
		ZeroMemory(m_pPageTrees[uiX][uiY]->m_root, sizeof(Page));
		m_pPageTrees[uiX][uiY]->m_root->m_uiLevel = -1;
		m_pPageTrees[uiX][uiY]->m_root->m_pPageTree = m_pPageTrees[uiX][uiY];
		m_pPageTrees[uiX][uiY]->m_root->m_bAreChildrenAvailable[0] = true;
		m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMinHeights[0] = treeDescs[ui].m_fRootMinHeight;
		m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMaxHeights[0] = treeDescs[ui].m_fRootMaxHeight;
		m_pPageTrees[uiX][uiY]->m_root->m_i64ChildrenFilePtrs[0] = treeDescs[ui].m_i64FilePtr;
		m_pPageTrees[uiX][uiY]->m_root->m_uiChildrenSizes[0] = treeDescs[ui].m_uiRootSize;

		// Bounding Box
		Box* pBox = &(m_pPageTrees[uiX][uiY]->m_root->m_childrenBBoxes[0]);
		pBox->m_vCenter = Vec3f((m_fTileExtents[0] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<0))) / 2.0f + m_fTileExtents[0] * static_cast<float>(m_pPageTrees[uiX][uiY]->m_iX), (m_fTileExtents[0] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<0))) / 2.0f + m_fTileExtents[0] * static_cast<float>(m_pPageTrees[uiX][uiY]->m_iY),
			(m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMinHeights[0] + m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMaxHeights[0]) / 2.0f
			);
		pBox->m_vHalfExtent = Vec3f((m_fTileExtents[0] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<0)))/2.0f, (m_fTileExtents[0] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<0)))/2.0f,
			(m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMaxHeights[0] - m_pPageTrees[uiX][uiY]->m_root->m_fChildrenMinHeights[0]) / 2.0f
			);

		m_pTileTrees[uiX][uiY] = new TileTree();
#ifdef _DEBUG
		m_i64NumObjectsCreated++;
#endif
		m_pTileTrees[uiX][uiY]->m_root = new Tile(0); // Dummy-Wurzel-Tile erzeugen
#ifdef _DEBUG
		m_i64NumObjectsCreated++;
#endif
		m_pTileTrees[uiX][uiY]->m_root->m_uiLevel = -1;


		m_pPageTrees[uiX][uiY]->m_root->m_pTiles[0] = m_pTileTrees[uiX][uiY]->m_root;
	}

	m_ui64StatNumBytesTransferred = 0;

}

uint DataLoader::GetNewGPUIndex()
{
	return m_uiNumGPUs++;
}

uint DataLoader::GetNewViewIndex()
{
	uint index = uint(m_views.size());

	m_views.push_back(View());

	return index;
}


void DataLoader::ComputeLoadingRadiuses(uint uiViewportHeight, float fFovy, float fAspectRatio, float fZFar, float fScreenSpaceError, float fPrefetchingFactor, uint viewIndex /*= 0*/)
{
	assert( viewIndex < m_views.size() );

	float fLevelZeroWorldSpaceError = m_fTileExtents[0] / uiTILE_SIZE;

	float tanfovy = tan(DEG2RAD(fFovy / 2.0f));
	float z = fLevelZeroWorldSpaceError * static_cast<float>(uiViewportHeight) / (tanfovy * fScreenSpaceError);
	float factor = (1.0f + (1.0f + fAspectRatio * fAspectRatio) * tanfovy * tanfovy) * fPrefetchingFactor  * fPrefetchingFactor;
	float fMaxLoadingRadiusSquared = fZFar * fZFar * factor;
	float fLoadingRadiusSquared = z * z * factor;
	
	// update the current view radiuses
	{
		std::vector< float > &loadingRadiusesSquared = m_views[viewIndex].fLoadingRadiusesSquared;
		loadingRadiusesSquared.resize(m_uiNumLevels);
		loadingRadiusesSquared[0] = fMaxLoadingRadiusSquared;
		fLoadingRadiusSquared /= 4.0f;
		for (uint uiLevel = 1; uiLevel < m_uiNumLevels; uiLevel++)
		{
			loadingRadiusesSquared[uiLevel] = min(fLoadingRadiusSquared, fMaxLoadingRadiusSquared);
			fLoadingRadiusSquared /= 4.0f;
		}
	}

	m_bUpdateTrees = true;
}


bool DataLoader::IsWithinLoadingRadius(const Box* pBox, uint uiLevel)
{
	for( uint i = 0 ; i < m_views.size() ; i++ ) {
		const View &view = m_views[i];
		if( uiLevel >= view.fLoadingRadiusesSquared.size() ) {
			continue;
		}

		tum3D::Vec3f vDiff = pBox->m_vCenter - view.position;
		tum3D::Vec3f vHalfExtent = pBox->m_vHalfExtent;
		float dx = max(0.0f, abs(vDiff.x()) - vHalfExtent.x());
		float dy = max(0.0f, abs(vDiff.y()) - vHalfExtent.y());
		float dz = max(0.0f, abs(vDiff.z()) - vHalfExtent.z());
		if( dx * dx + dy * dy + dz * dz < view.fLoadingRadiusesSquared[uiLevel] ) {
			return true;
		}
	}
	return false;
}


void DataLoader::Render(Terrain* pTerrain)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::Render");

	pTerrain->m_pVertexBufferBB->Bind( GL_ARRAY_BUFFER );
	pTerrain->m_tileRenderingBBEffect.SetupBinding( sizeof( VertexBB ), 0 );

	pTerrain->m_pIndexBufferBB->Bind( GL_ELEMENT_ARRAY_BUFFER );

	for (int iY = m_treesInfo.m_iTreeY0; iY <= m_treesInfo.m_iTreeY1; iY++)
	{
		for (int iX = m_treesInfo.m_iTreeX0; iX <= m_treesInfo.m_iTreeX1; iX++)
		{
			PageTree* pPageTree = m_pPageTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
			Render(pTerrain, pPageTree->m_root);
		}
	}
}


void DataLoader::Render(Terrain* pTerrain, Page* pPage)
{
	for (uint uiChild = 0; uiChild < 4; uiChild++)
	{
		if (pPage->m_children[uiChild] != NULL && pPage->m_children[uiChild]->m_pPageData != NULL)
		{
			uint uiLevel = pPage->m_children[uiChild]->m_uiLevel;
			tum3D::Vec3f vCenter = pPage->m_childrenBBoxes[uiChild].m_vCenter;
			tum3D::Vec3f vHalfExtent = pPage->m_childrenBBoxes[uiChild].m_vHalfExtent;

			tum3D::Mat4f mTranslationTransposed;
			tum3D::translationMat( vCenter - vHalfExtent, mTranslationTransposed );
			tum3D::Mat4f mTranslation;
			mTranslationTransposed.transpose(mTranslation);

			tum3D::Mat4f mWorldView;
			tum3D::scalingMat( vHalfExtent * 2.0f, mWorldView );
			mWorldView *= mTranslation * pTerrain->m_mView;

			pTerrain->m_tileRenderingBBEffect.mWorldView.Set(mWorldView);

			float f;
			if (m_uiNumLevels > 1)
			{
				f = static_cast<float>(uiLevel) / static_cast<float>(m_uiNumLevels - 1);
			}
			else
			{
				f = 1.0f;
			}
			float fColor[] = {1.0f - f, f, 0.0f, 1.0f};
			
			pTerrain->m_tileRenderingBBEffect.vColorBB.Set(fColor);

			uint numPasses = pTerrain->m_tileRenderingBBEffect.GetNumPasses();
			for (uint p = 0; p < numPasses; p++) {
				pTerrain->m_tileRenderingBBEffect.SetupPass( p );
				glDrawElements( GL_LINES, 24, GL_UNSIGNED_INT, (GLbyte*) 0 );
				checkGLError();
			}

			Render(pTerrain, pPage->m_children[uiChild]);
		}
	}
}


void DataLoader::Update(const ViewInfoVector &viewInfos, bool bExecuteAllIO)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::Update");

	for(uint i = 0 ; i < viewInfos.size() ; i++) {
		const ViewInfo &viewInfo = viewInfos[i];

		assert( viewInfo.viewIndex < m_views.size() );
		View &view = m_views[viewInfo.viewIndex];

		view.position = viewInfo.position;

		view.normalizedViewDirection = viewInfo.viewDirection;
		tum3D::normalize( view.normalizedViewDirection );
	}

	if(m_views.empty())
		return;

	// Berechne welche Page Trees im Loading Radius liegen

	// TODO:.. [12/23/2009 Andreas Kirsch]
	int iTreePositionX = static_cast<int>(floor(m_views[0].position.x() / m_fTileExtents[0]));
	int iTreePositionY = static_cast<int>(floor(m_views[0].position.y() / m_fTileExtents[0]));

	if (m_bUpdateTrees || iTreePositionX != m_iTreePositionX || iTreePositionY != m_iTreePositionY)
	{
		/*
		int iPageTreeX0 = max(iPageTreePositionX - static_cast<int>(m_uiLoadingRadiusLevelZeroInPageTrees), m_iTreeMinX);
		int iPageTreeY0 = max(iPageTreePositionY - static_cast<int>(m_uiLoadingRadiusLevelZeroInPageTrees), m_iTreeMinY);
		int iPageTreeX1 = min(iPageTreePositionX + static_cast<int>(m_uiLoadingRadiusLevelZeroInPageTrees), m_iTreeMaxX);
		int iPageTreeY1 = min(iPageTreePositionY + static_cast<int>(m_uiLoadingRadiusLevelZeroInPageTrees), m_iTreeMaxY);
		*/
		int iPageTreeX0 = m_iTreeMinX;
		int iPageTreeY0 = m_iTreeMinY;
		int iPageTreeX1 = m_iTreeMaxX;
		int iPageTreeY1 = m_iTreeMaxY;
		
		for (int iY = m_treesInfo.m_iTreeY0; iY <= m_treesInfo.m_iTreeY1; iY++)
		{
			for (int iX = m_treesInfo.m_iTreeX0; iX <= m_treesInfo.m_iTreeX1; iX++)			
			{
				if (iX < iPageTreeX0 || iX > iPageTreeX1 || iY < iPageTreeY0 || iY > iPageTreeY1)
				{
					PageTree* pPageTree = m_pPageTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
					if (pPageTree != NULL)
					{
						if (pPageTree->m_root->m_children[0] != NULL)
						{
							m_unloadQueue.push_back(pPageTree);
							pPageTree->m_unloadQueueLocator = m_unloadQueue.end();
							pPageTree->m_unloadQueueLocator--;
							pPageTree->m_bIsInUnloadQueue = true;
						}
					}
				}
			}
		}

		for (int iY = iPageTreeY0; iY <= iPageTreeY1; iY++)
		{
			for (int iX = iPageTreeX0; iX <= iPageTreeX1; iX++)
			{
				PageTree* pPageTree = m_pPageTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
				if (pPageTree != NULL)
				{
					if (pPageTree->m_bIsInUnloadQueue)
					{
						m_unloadQueue.erase(pPageTree->m_unloadQueueLocator);
						pPageTree->m_bIsInUnloadQueue = false;
					}
				}
			}
		}
				
		m_iTreePositionX = iTreePositionX;
		m_iTreePositionY = iTreePositionY;

		m_treesInfo.m_iTreeX0 = iPageTreeX0;
		m_treesInfo.m_iTreeY0 = iPageTreeY0;
		m_treesInfo.m_iTreeX1 = iPageTreeX1;
		m_treesInfo.m_iTreeY1 = iPageTreeY1;

		m_bUpdateTrees = false;
	}

	std::list<PageTree*>::iterator p = m_unloadQueue.begin();
	while (p != m_unloadQueue.end())
	{
		PageTree* pPageTree = *p;
		p++;
		if (UnloadPageSubtree(pPageTree->m_root->m_children[0]))
		{
			m_unloadQueue.erase(pPageTree->m_unloadQueueLocator);
			pPageTree->m_bIsInUnloadQueue = false;
		}
	}

	// Aktualisiere Page Trees

	if (bExecuteAllIO)
	{
		ExecutePendingIO();
		while (true)
		{
			for (int iY = m_treesInfo.m_iTreeY0; iY <= m_treesInfo.m_iTreeY1; iY++)
			{
				for (int iX = m_treesInfo.m_iTreeX0; iX <= m_treesInfo.m_iTreeX1; iX++)
				{
					PageTree* pPageTree = m_pPageTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
					if (pPageTree != NULL)
					{
						Update(pPageTree->m_root);
					}
				}
			}
			bool bDone = true;
			for (uint uiLevel = 0; uiLevel < m_uiNumLevels; uiLevel++)
			{
				if (m_requestQueue[uiLevel].size() != 0) { bDone = false; }
			}
			if (bDone) { break; }
			ExecuteRequestQueueIO();
		}
	}
	else
	{
		for (int iY = m_treesInfo.m_iTreeY0; iY <= m_treesInfo.m_iTreeY1; iY++)
		{
			for (int iX = m_treesInfo.m_iTreeX0; iX <= m_treesInfo.m_iTreeX1; iX++)
			{
				PageTree* pPageTree = m_pPageTrees[iX - m_iTreeMinX][iY - m_iTreeMinY];
				if (pPageTree != NULL)
				{
					Update(pPageTree->m_root);
				}
			}
		}

		SynchronizeAsyncIO();
	}

}

void DataLoader::Update(Page* pPage) // Page muss geladen sein
{
	for (uint uiChild = 0; uiChild < 4; uiChild++)
	{
		if (pPage->m_bAreChildrenAvailable[uiChild])
		{
			if (pPage->m_uiLevel + 1 <= m_uiMaxLevel && IsWithinLoadingRadius(&(pPage->m_childrenBBoxes[uiChild]), pPage->m_uiLevel + 1))
			// m_uiLevel ist vom Typ uint, um -1 korrekt zu behandeln daher + 1 <= statt <
			{
				if (pPage->m_children[uiChild] != NULL)
				{
					if (pPage->m_children[uiChild]->m_pPageData != NULL)
					{
						// Child Page geladen
						Update(pPage->m_children[uiChild]);
					}
					else
					{
						// Child Page angefordert

					}

				}
				else
				{
					// Child Page nicht angefordert

					// Cache überprüfen
					if (pPage->m_cachedChildren[uiChild] != NULL)
					{
						// Cache Hit
						m_pageCache.erase(pPage->m_cachedChildren[uiChild]->m_pageCacheLocator);
						pPage->m_cachedChildren[uiChild]->m_bIsInPageCache = false;

						pPage->m_children[uiChild] = pPage->m_cachedChildren[uiChild];
						pPage->m_cachedChildren[uiChild] = NULL;

						AddTiles(pPage->m_children[uiChild]);

						m_uiStatNumPages++;

						Update(pPage->m_children[uiChild]);
					}
					else
					{
						// Cache Miss, Page anfordern
						Page* pChildPage = new Page();
#ifdef _DEBUG
						m_i64NumObjectsCreated++;
#endif
						ZeroMemory(pChildPage, sizeof(Page));
						pChildPage->m_uiX = pPage->m_uiX * 2 + uiChild % 2;
						pChildPage->m_uiY = pPage->m_uiY * 2 + uiChild / 2;
						pChildPage->m_uiLevel = pPage->m_uiLevel + 1;
						pChildPage->m_uiChild = uiChild;
						pChildPage->m_pPageTree = pPage->m_pPageTree;
						pChildPage->m_parent = pPage;
						
						pPage->m_children[uiChild] = pChildPage;

						PageDataRequest* pChildPageDataRequest = new PageDataRequest();
#ifdef _DEBUG
						m_i64NumObjectsCreated++;
#endif
						ZeroMemory(pChildPageDataRequest, sizeof(PageDataRequest));
						pChildPageDataRequest->m_pPage = pChildPage;
						pChildPageDataRequest->m_fPriority = 1.0f;
						pChildPageDataRequest->m_uiFileID = pPage->m_pPageTree->m_uiFileID;
						pChildPageDataRequest->m_i64FilePointer = pPage->m_i64ChildrenFilePtrs[uiChild];
						pChildPageDataRequest->m_uiSize = pPage->m_uiChildrenSizes[uiChild];

						pChildPage->m_pPageDataRequest = pChildPageDataRequest;

						m_requestQueue[pChildPage->m_uiLevel].push_back(pChildPageDataRequest);
						m_uiStatRequestQueueSize++;
						pChildPage->m_requestQueueLocator = m_requestQueue[pChildPage->m_uiLevel].end();
						pChildPage->m_requestQueueLocator--;
						pChildPage->m_bIsInRequestQueue = true;
					}
				}
			}
			else // Child Page ist nicht innerhalb des Loading Radius
			{
				if (pPage->m_children[uiChild] != NULL)
				{
					// Child Page geladen oder angefordert
					UnloadPageSubtree(pPage->m_children[uiChild]);
				}
				else
				{
					// Child Page nicht angefordert
				}
			}

		}
	} // for uiChild
}


// UnloadPageSubtree gibt true zurück, genau dann wenn der Teilbaum vollständig entfernt worden ist
bool DataLoader::UnloadPageSubtree(Page* pPage)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::UnloadPageSubtree");

	if (pPage->m_pPageData != NULL)
	{
		// Page geladen
		bool bHasNoChildren = true;
		for (uint uiChild = 0; uiChild < 4; uiChild++)
		{
			if (pPage->m_children[uiChild] != NULL)
			{
				bHasNoChildren = bHasNoChildren && UnloadPageSubtree(pPage->m_children[uiChild]);
			}
		}
		if (bHasNoChildren)
		{
			if (!RemoveTiles(pPage))
			{
				// some of the tiles were still paged in (shouldn't actually happen)
				return false;
			}

			if (m_dwlSystemMemoryUsage <= m_dwlSystemMemoryLimit)
			{
				// Wenn Speicherlimit nicht überschritten, Page zum Cache hinzufügen
				pPage->m_parent->m_cachedChildren[pPage->m_uiChild] = pPage;
				pPage->m_parent->m_children[pPage->m_uiChild] = NULL;

				m_pageCache.push_back(pPage);
				pPage->m_pageCacheLocator = m_pageCache.end();
				pPage->m_pageCacheLocator--;
				pPage->m_bIsInPageCache = true;
			}
			else
			{
				// Sonst die Page aus dem Speicher entfernen (Cache ist leer, denn erst wenn der Cache leer ist, kann das Speicherlimit überschritten werden)
				BOOL bResult = VirtualFree(pPage->m_pPageData, 0, MEM_RELEASE);
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
				if (!bResult)
				{
					// VirtualFree ist fehlgeschlagen
					MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
				m_dwlSystemMemoryUsage -= pPage->m_parent->m_uiChildrenSizes[pPage->m_uiChild];

				pPage->m_parent->m_children[pPage->m_uiChild] = NULL;
				delete pPage;
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
			}

			m_uiStatNumPages--;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		// Page angefordert
		
		// Beachte: Es ist möglich, dass der Request bereits dem Loader Thread übergeben wurde
		// (D.h. in den Request Buffer verschoben wurde)
		// Dann ist kein Abbruch mehr möglich

		if (pPage->m_bIsInRequestQueue)
		{
			// Request abbrechen
			m_requestQueue[pPage->m_uiLevel].erase(pPage->m_requestQueueLocator);
			m_uiStatRequestQueueSize--;
			delete pPage->m_pPageDataRequest;
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif
			// Page löschen
			pPage->m_parent->m_children[pPage->m_uiChild] = NULL;
			delete pPage;
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif
			return true;
		}
		else
		{
			return false;
		}
	}

}

void DataLoader::CommitPageDataRequest(PageDataRequest* pPageDataRequest)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::CommitPageDataRequest");

	m_ui64StatNumBytesTransferred += pPageDataRequest->m_uiSize;


	Page* pPage = pPageDataRequest->m_pPage;

	// Speicher freigeben wenn nötig und möglich
	while (m_dwlSystemMemoryUsage + pPageDataRequest->m_uiSize > m_dwlSystemMemoryLimit && m_pageCache.size() > 0)
	{
		Page* pPage = m_pageCache.front();
		m_pageCache.pop_front();

		if (!RemoveTiles(pPage))
		{
			// some of the tiles were still paged in (shouldn't actually happen)
			break;
		}

		BOOL bResult = VirtualFree(pPage->m_pPageData, 0, MEM_RELEASE);
#ifdef _DEBUG
		m_i64NumObjectsDestroyed++;
#endif
		if (!bResult)
		{
			// VirtualFree ist fehlgeschlagen
			MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
			exit(-1);
		}

		m_dwlSystemMemoryUsage -= pPage->m_parent->m_uiChildrenSizes[pPage->m_uiChild];

		pPage->m_parent->m_cachedChildren[pPage->m_uiChild] = NULL;
		delete pPage;
#ifdef _DEBUG
		m_i64NumObjectsDestroyed++;
#endif
	}

	pPage->m_pPageData = pPageDataRequest->m_pPageData;
	
	byte* ptr = pPage->m_pPageData;
	ptr += pPageDataRequest->m_uiOffset;

	for (int uiChild = 0; uiChild < 4; uiChild++)
	{
		pPage->m_bAreChildrenAvailable[uiChild] = *reinterpret_cast<bool*>(ptr);
		ptr += sizeof(bool);
		pPage->m_fChildrenMinHeights[uiChild] = *reinterpret_cast<float*>(ptr);
		ptr += sizeof(float);
		pPage->m_fChildrenMaxHeights[uiChild] = *reinterpret_cast<float*>(ptr);
		ptr += sizeof(float);
		pPage->m_i64ChildrenFilePtrs[uiChild] = *reinterpret_cast<llong*>(ptr);
		ptr += sizeof(llong);
		pPage->m_uiChildrenSizes[uiChild] = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
	}

	pPage->m_uiOffset = static_cast<uint>(ptr - pPage->m_pPageData);

	pPage->m_pPageDataRequest = NULL;
	
	delete pPageDataRequest;
#ifdef _DEBUG
	m_i64NumObjectsDestroyed++;
#endif

	m_dwlSystemMemoryUsage += pPage->m_parent->m_uiChildrenSizes[pPage->m_uiChild];

	// Berechne Bounding Boxes der Child Pages
	for (uint uiChild = 0; uiChild < 4; uiChild++)
	{
		if (pPage->m_bAreChildrenAvailable[uiChild])
		{
			assert(pPage->m_fChildrenMinHeights[uiChild] <= pPage->m_fChildrenMaxHeights[uiChild]);
			// Prüfe ob Bounding Boxes der Child Pages innerhalb der Bounding Box der Parent Page liegen
			assert(pPage->m_fChildrenMinHeights[uiChild] >= pPage->m_parent->m_fChildrenMinHeights[pPage->m_uiChild]);
			assert(pPage->m_fChildrenMaxHeights[uiChild] <= pPage->m_parent->m_fChildrenMaxHeights[pPage->m_uiChild]);

			uint uiX = pPage->m_uiX * 2 + uiChild % 2;
			uint uiY = pPage->m_uiY * 2 + uiChild / 2;
			uint uiLevel = pPage->m_uiLevel + 1;
			Vec3f* pvCenter = &(pPage->m_childrenBBoxes[uiChild].m_vCenter);
			pvCenter->x() = (static_cast<float>(uiX) + 0.5f) * m_fTileExtents[uiLevel] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<uiLevel)) + static_cast<float>(pPage->m_pPageTree->m_iX) * m_fTileExtents[0];
			pvCenter->y() = (static_cast<float>(uiY) + 0.5f) * m_fTileExtents[uiLevel] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<uiLevel)) + static_cast<float>(pPage->m_pPageTree->m_iY) * m_fTileExtents[0];
			pvCenter->z() = (pPage->m_fChildrenMinHeights[uiChild] + pPage->m_fChildrenMaxHeights[uiChild]) / 2.0f;
			Vec3f* pvHalfExtent = &(pPage->m_childrenBBoxes[uiChild].m_vHalfExtent);
			pvHalfExtent->x() = m_fTileExtents[uiLevel] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<uiLevel)) / 2.0f;
			pvHalfExtent->y() = m_fTileExtents[uiLevel] * static_cast<float>(min(uiPAGE_EXTENT_IN_TILES, 1<<uiLevel)) / 2.0f;
			pvHalfExtent->z() = (pPage->m_fChildrenMaxHeights[uiChild] - pPage->m_fChildrenMinHeights[uiChild]) / 2.0f;
		}
	}

	AddTiles(pPage);

}


std::vector<PageDataRequest*> DataLoader::GetPageDataRequests(uint uiNumRequests)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::GetPageDataRequests");

	std::vector<PageDataRequest*> requests;

	uint uiLevel = 0;
	uint uiPreviousLevel = -1;

	while (requests.size() < uiNumRequests)
	{
		// Suche Level
		while (uiLevel < m_uiNumLevels && m_requestQueue[uiLevel].size() == 0)
		{
			uiLevel++;
		}
		// Abbruch, wenn kein Level gefunden
		if (uiLevel == m_uiNumLevels)
		{
			break;
		}
		if (uiLevel != uiPreviousLevel)
		{
			// Berechne Prioritäten
			std::list<PageDataRequest*>::iterator p;
			for (p = m_requestQueue[uiLevel].begin(); p != m_requestQueue[uiLevel].end(); p++)
			{
				(*p)->m_fPriority = ComputePriority(&(*p)->m_pPage->m_parent->m_childrenBBoxes[(*p)->m_pPage->m_uiChild]);
			}
		}
		// Suche Request mit maximaler Priorität
		std::list<PageDataRequest*>::iterator p, q;
		q = m_requestQueue[uiLevel].begin();
		p = q;
		p++;
		for ( ; p != m_requestQueue[uiLevel].end(); p++)
		{
			if ((*p)->m_fPriority > (*q)->m_fPriority)
			{
				q = p;
			}
		}
		PageDataRequest* pPageDataRequest = *q;
		m_requestQueue[uiLevel].erase(q);
		m_uiStatRequestQueueSize--;
		pPageDataRequest->m_pPage->m_bIsInRequestQueue = false;
		
		requests.push_back(pPageDataRequest);

	}

	return requests;

}


void DataLoader::SynchronizeAsyncIO()
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::SynchronizeAsyncIO");

	uint uiNumFreeSlots = 0;
	std::vector<PageDataRequest*> requestsToCommit;
	for (uint uiSlot = 0; uiSlot < uiNUM_LOADING_SLOTS; uiSlot++)
	{
		if (m_loadingSlots[uiSlot] == NULL)
		{
			uiNumFreeSlots++;
		}
		else if (HasOverlappedIoCompleted(&m_overlappeds[uiSlot]))
		{
			PageDataRequest* pPageDataRequest = m_loadingSlots[uiSlot];
			DWORD dwNumBytesRead;
			BOOL bResult = GetOverlappedResult(pPageDataRequest->m_hFile, &m_overlappeds[uiSlot], &dwNumBytesRead, FALSE);
			if (!bResult || dwNumBytesRead < pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset) // Bei einem Fehler
			{
				MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
				exit(-1);
			}
			requestsToCommit.push_back(pPageDataRequest);
			m_loadingSlots[uiSlot] = NULL;
			uiNumFreeSlots++;
		}
	}
	std::vector<PageDataRequest*> requestsToService = GetPageDataRequests(uiNumFreeSlots);
	uint uiRequest = 0;
	for (uint uiSlot = 0; uiRequest < requestsToService.size() && uiSlot < uiNUM_LOADING_SLOTS; uiSlot++)
	{
		if (m_loadingSlots[uiSlot] == NULL)
		{
			ZeroMemory(&m_overlappeds[uiSlot], sizeof(OVERLAPPED));

			PageDataRequest* pPageDataRequest = requestsToService[uiRequest];

			pPageDataRequest->m_hFile = m_pFilePool->GetFileHandle(pPageDataRequest->m_uiFileID);

			// Berechne Startposition und Endposition in Vielfachen der Page Size
			pPageDataRequest->m_uiOffset = static_cast<uint>(pPageDataRequest->m_i64FilePointer % static_cast<llong>(m_dwPageSize));
			uint uiSize = ((pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset + static_cast<uint>(m_dwPageSize) - 1) / static_cast<uint>(m_dwPageSize)) * static_cast<uint>(m_dwPageSize);
			llong i64Start = pPageDataRequest->m_i64FilePointer - static_cast<llong>(pPageDataRequest->m_uiOffset);

			// Speicher belegen
			pPageDataRequest->m_pPageData = (byte*)VirtualAlloc(NULL, uiSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#ifdef _DEBUG
			m_i64NumObjectsCreated++;
#endif
			if (pPageDataRequest->m_pPageData == NULL)
			{
				// VirtualAlloc ist fehlgeschlagen
				MessageBox(NULL, "VirtualAlloc failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
				exit(-1);
			}

			// Slot belegen
			m_loadingSlots[uiSlot] = pPageDataRequest;

			m_overlappeds[uiSlot].Offset = static_cast<DWORD>(i64Start & 0x00000000FFFFFFFFi64);
			m_overlappeds[uiSlot].OffsetHigh = static_cast<DWORD>(i64Start >> 32);
			m_overlappeds[uiSlot].hEvent = m_hCompletionEvents[uiSlot];

			DWORD dwNumBytesRead;
			BOOL bResult = ReadFile(pPageDataRequest->m_hFile, pPageDataRequest->m_pPageData, uiSize, &dwNumBytesRead, &m_overlappeds[uiSlot]);
			if (bResult)
			{
				// Read bereits fertig
				if (dwNumBytesRead < pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset) // Bei einem Fehler
				{
					MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
				requestsToCommit.push_back(m_loadingSlots[uiSlot]);
				m_loadingSlots[uiSlot] = NULL;
			}
			else
			{
				// Read noch nicht fertig
				DWORD error = GetLastError();
				if (error != ERROR_IO_PENDING) // Bei einem Fehler
				{
					MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
			}

			uiRequest++;
		}
	}

	for (uint ui = 0; ui < requestsToCommit.size(); ui++)
	{
		m_pFilePool->ReleaseFileHandle(requestsToCommit[ui]->m_uiFileID);
		CommitPageDataRequest(requestsToCommit[ui]);
		m_uiStatNumPages++;
	}

}

void DataLoader::ExecutePendingIO()
{
	// Auf Fertigstellung von Requests warten, die bereits in Bearbeitung sind
	for (uint uiSlot = 0; uiSlot < uiNUM_LOADING_SLOTS; uiSlot++)
	{
		if (m_loadingSlots[uiSlot] != NULL)
		{
			PageDataRequest* pPageDataRequest = m_loadingSlots[uiSlot];

			DWORD dwNumBytesRead;
			BOOL bResult = GetOverlappedResult(pPageDataRequest->m_hFile, &m_overlappeds[uiSlot], &dwNumBytesRead, TRUE); // Warten
			if (!bResult || dwNumBytesRead < pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset) // Bei einem Fehler
			{
				MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
				exit(-1);
			}

			m_loadingSlots[uiSlot] = NULL;
			m_pFilePool->ReleaseFileHandle(pPageDataRequest->m_uiFileID);
			CommitPageDataRequest(pPageDataRequest);
			m_uiStatNumPages++;
		}
	}

}


void DataLoader::ExecuteRequestQueueIO() // Darf erst nach ExecutePendingIO aufgerufen werden, da m_overlappeds[0] verwendet wird
{
	// Alle Requests in der Request Queue abarbeiten
	for (uint uiLevel = 0; uiLevel < m_uiNumLevels; uiLevel++)
	{
		for (std::list<PageDataRequest*>::iterator p = m_requestQueue[uiLevel].begin(); p != m_requestQueue[uiLevel].end(); p++)
		{
			PageDataRequest* pPageDataRequest = *p;
			pPageDataRequest->m_pPage->m_bIsInRequestQueue = false;

			pPageDataRequest->m_hFile = m_pFilePool->GetFileHandle(pPageDataRequest->m_uiFileID);

			// Berechne Startposition und Endposition in Vielfachen der Page Size
			pPageDataRequest->m_uiOffset = static_cast<uint>(pPageDataRequest->m_i64FilePointer % static_cast<llong>(m_dwPageSize));
			uint uiSize = ((pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset + static_cast<uint>(m_dwPageSize) - 1) / static_cast<uint>(m_dwPageSize)) * static_cast<uint>(m_dwPageSize);
			llong i64Start = pPageDataRequest->m_i64FilePointer - static_cast<llong>(pPageDataRequest->m_uiOffset);

			// Speicher belegen
			pPageDataRequest->m_pPageData = (byte*)VirtualAlloc(NULL, uiSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#ifdef _DEBUG
			m_i64NumObjectsCreated++;
#endif
			if (pPageDataRequest->m_pPageData == NULL)
			{
				// VirtualAlloc ist fehlgeschlagen
				MessageBox(NULL, "VirtualAlloc failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
				exit(-1);
			}

			assert(m_loadingSlots[0] == NULL);
			
			ZeroMemory(&m_overlappeds[0], sizeof(OVERLAPPED));

			m_overlappeds[0].Offset = static_cast<DWORD>(i64Start & 0x00000000FFFFFFFFi64);
			m_overlappeds[0].OffsetHigh = static_cast<DWORD>(i64Start >> 32);
			m_overlappeds[0].hEvent = m_hCompletionEvents[0];

			DWORD dwNumBytesRead;
			BOOL bResult = ReadFile(pPageDataRequest->m_hFile, pPageDataRequest->m_pPageData, uiSize, &dwNumBytesRead, &m_overlappeds[0]);
			if (bResult)
			{
				// Read bereits fertig
				if (dwNumBytesRead < pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset) // Bei einem Fehler
				{
					MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
			}
			else
			{
				// Read noch nicht fertig
				DWORD error = GetLastError();
				if (error != ERROR_IO_PENDING) // Bei einem Fehler
				{
					MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
				bResult = GetOverlappedResult(pPageDataRequest->m_hFile, &m_overlappeds[0], &dwNumBytesRead, TRUE); // Warten
				if (!bResult || dwNumBytesRead < pPageDataRequest->m_uiSize + pPageDataRequest->m_uiOffset) // Bei einem Fehler
				{
					MessageBox(NULL, "ReadFile failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
			}
			m_pFilePool->ReleaseFileHandle(pPageDataRequest->m_uiFileID);
			CommitPageDataRequest(pPageDataRequest);
			m_uiStatNumPages++;

		}
		m_requestQueue[uiLevel].clear();

	}
	m_uiStatRequestQueueSize = 0;
}


void DataLoader::AddTiles(Page* pPage)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::AddTiles");

	byte* ptr = pPage->m_pPageData + pPage->m_uiOffset;
	uint uiNumTiles = *reinterpret_cast<uchar*>(ptr);
	ptr += sizeof(uchar);
	assert(uiNumTiles <= uiPAGE_EXTENT_IN_TILES * uiPAGE_EXTENT_IN_TILES);

	for (uint ui = 0; ui < uiNumTiles; ui++)
	{
		Tile* pTile = new Tile(m_uiNumGPUs);
#ifdef _DEBUG
		m_i64NumObjectsCreated++;
#endif

		uint uiTile = *reinterpret_cast<uchar*>(ptr);
		ptr += sizeof(uchar);
		assert(uiTile < uiPAGE_EXTENT_IN_TILES * uiPAGE_EXTENT_IN_TILES);

		// Tile erzeugen
		// uiX bzw. uiY beziehen sich auf alle Child Tiles der Parent Page
		uint uiX = uiTile % uiPAGE_EXTENT_IN_TILES + (pPage->m_uiChild % 2) * uiPAGE_EXTENT_IN_TILES;
		uint uiY = uiTile / uiPAGE_EXTENT_IN_TILES + (pPage->m_uiChild / 2) * uiPAGE_EXTENT_IN_TILES;

		Tile* pParentTile = pPage->m_parent->m_pTiles[uiX / 2 + (uiY / 2) * uiPAGE_EXTENT_IN_TILES];
		
		pTile->m_uiX = pParentTile->m_uiX * 2 + uiX % 2;
		pTile->m_uiY = pParentTile->m_uiY * 2 + uiY % 2;
		pTile->m_uiLevel = pParentTile->m_uiLevel + 1;
		pTile->m_uiChild = uiX % 2 + (uiY % 2) * 2;
		
		// Tile einhängen
		pTile->m_parent = pParentTile;
		pParentTile->m_children[uiX % 2 + (uiY % 2) * 2] = pTile;
		pPage->m_pTiles[uiTile] = pTile;

		// Weitere Variablen des Tiles setzen
		pTile->m_fMinHeight = *reinterpret_cast<float*>(ptr);
		ptr += sizeof(float);
		pTile->m_fMaxHeight = *reinterpret_cast<float*>(ptr);
		ptr += sizeof(float);

		// m_uiStripHeadersSize == -1 bedeutet, dass die Geometriedaten unkomprimiert gespeichert sind
		// In diesem Fall sind die Geometriedaten komplett in den Strip Daten abgelegt
		pTile->m_uiStripHeadersSize = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
		assert(pTile->m_uiStripHeadersSize <= ((uiTILE_SIZE * uiTILE_SIZE * 2) / 16) * 8 || pTile->m_uiStripHeadersSize == -1);
		pTile->m_pStripHeaders = ptr;
		if (pTile->m_uiStripHeadersSize != -1)
		{
			ptr += pTile->m_uiStripHeadersSize;
		}

		pTile->m_uiStripDataFormat = *reinterpret_cast<uchar*>(ptr);
		ptr += sizeof(uchar);
		assert(pTile->m_uiStripDataFormat <= 16);

		pTile->m_uiStripDataSize = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
		assert(pTile->m_uiStripDataSize <= ((uiTILE_SIZE * uiTILE_SIZE * 2 + 2) * (pTile->m_uiStripDataFormat + 2) + 7) / 8
			|| pTile->m_uiStripHeadersSize == -1 && pTile->m_uiStripDataSize <= (uiTILE_SIZE * uiTILE_SIZE * 2 + uiTILE_SIZE * 4 * 2) * 3 * 4);
		pTile->m_pStripData = ptr;
		ptr += pTile->m_uiStripDataSize;

		pTile->m_uiNumBoundaryEdges = *reinterpret_cast<ushort*>(ptr);
		ptr += sizeof(ushort);
		assert(pTile->m_uiNumBoundaryEdges <= uiTILE_SIZE * 4);

		pTile->m_uiTextureDataSize = *reinterpret_cast<uint*>(ptr);
		ptr += sizeof(uint);
		pTile->m_pTextureData = ptr;
		
		Tile* pTextureTile = pTile;
		// GL uses centered sampling
		tum3D::Vec2f vTexCoordOffset(0.0f, 0.0f);
		tum3D::Vec2f vTexCoordSize(1.0f, 1.0f);

		if (pTile->m_uiTextureDataSize != -1)
		{
			// Die Kachel besitzt eine eigene Textur
			ptr += pTile->m_uiTextureDataSize;
		}
		else
		{
			// Die Kachel verwendet die Textur einer Vorfahren-Kachel
			// Berechne Texturkoordinaten
			do
			{
				vTexCoordOffset.x() = static_cast<float>(pTextureTile->m_uiChild % 2) * 0.5f + vTexCoordOffset.x() * 0.5f;
				vTexCoordOffset.y() = static_cast<float>(pTextureTile->m_uiChild / 2) * 0.5f + vTexCoordOffset.y() * 0.5f;
				vTexCoordSize.x() *= 0.5f;
				vTexCoordSize.y() *= 0.5f;
				pTextureTile = pTextureTile->m_parent;
			}
			while (pTextureTile->m_uiTextureDataSize == -1);
		}

		pTile->m_pTextureTile = pTextureTile;
		pTile->m_vTexCoordOffset = vTexCoordOffset;
		pTile->m_vTexCoordSize = vTexCoordSize;

		// Berechne Bounding Box

		assert(pTile->m_fMinHeight <= pTile->m_fMaxHeight);
		// Prüfe ob Bounding Box innerhalb der Bounding Box der Page liegt
		assert(pTile->m_uiStripHeadersSize == 0 || pTile->m_fMinHeight >= pPage->m_parent->m_fChildrenMinHeights[pPage->m_uiChild]);
		assert(pTile->m_uiStripHeadersSize == 0 || pTile->m_fMaxHeight <= pPage->m_parent->m_fChildrenMaxHeights[pPage->m_uiChild]);
		// Prüfe ob Bounding Box innerhalb der Bounding Box des Parent Tiles liegt
#ifdef _DEBUG
		if (pTile->m_uiLevel > 0)
		{
			assert(pTile->m_uiStripHeadersSize == 0 || pTile->m_fMinHeight >= pTile->m_parent->m_fMinHeight);
			assert(pTile->m_uiStripHeadersSize == 0 || pTile->m_fMaxHeight <= pTile->m_parent->m_fMaxHeight);
		}
#endif

		Vec3f* pvCenter = &(pTile->m_BBox.m_vCenter);
		pvCenter->x() = (static_cast<float>(pTile->m_uiX) + 0.5f) * m_fTileExtents[pTile->m_uiLevel] + static_cast<float>(pPage->m_pPageTree->m_iX) * m_fTileExtents[0];
		pvCenter->y() = (static_cast<float>(pTile->m_uiY) + 0.5f) * m_fTileExtents[pTile->m_uiLevel] + static_cast<float>(pPage->m_pPageTree->m_iY) * m_fTileExtents[0];
		pvCenter->z() = (pTile->m_fMinHeight + pTile->m_fMaxHeight) / 2.0f;
		Vec3f* pvHalfExtent = &(pTile->m_BBox.m_vHalfExtent);
		pvHalfExtent->x() = m_fTileExtents[pTile->m_uiLevel] / 2.0f;
		pvHalfExtent->y() = m_fTileExtents[pTile->m_uiLevel] / 2.0f;
		pvHalfExtent->z() = (pTile->m_fMaxHeight - pTile->m_fMinHeight) / 2.0f;

		if (pTile->m_uiStripHeadersSize != 0)
		{
			if (pTile->m_uiStripHeadersSize != -1)
			{
				uchar uc = pTile->m_pStripData[pTile->m_uiStripDataSize - 1];
				uint count = 0;
				uchar mask = 1 << 7;
				while ((uc & mask) != mask) // Suche End-Marker
				{
					mask >>= 1;
					count++;
				}
				pTile->m_uiNumTriangles = (pTile->m_uiStripDataSize * 8 - count - 2) / (pTile->m_uiStripDataFormat + 2) - 2;
				
				assert(pTile->m_uiNumTriangles <= uiTILE_SIZE * uiTILE_SIZE * 2);
			}
			else
			{
				pTile->m_uiNumTriangles = pTile->m_uiStripDataSize / (3 * 4) - pTile->m_uiNumBoundaryEdges * 2;
				assert(pTile->m_uiNumTriangles <= uiTILE_SIZE * uiTILE_SIZE * 2);
			}
		}
		else
		{
			pTile->m_uiNumTriangles = 0;
		}

		m_uiStatNumTiles++;

	}
}


bool DataLoader::RemoveTiles(Page* pPage, bool bForce)
{
	ScopedProfileSample sample(m_pProfiler, "DataLoader::RemoveTiles");

	bool bAllPagedOut = true;
	for (uint uiTile = 0; uiTile < uiPAGE_EXTENT_IN_TILES * uiPAGE_EXTENT_IN_TILES; uiTile++)
	{
		if (pPage->m_pTiles[uiTile] != NULL)
		{
			Tile* pTile = pPage->m_pTiles[uiTile];
			if (pTile->IsAnyGPUPagedIn())
			{
				bAllPagedOut = false;
				break;
			}
		}
	}

	if (!bAllPagedOut && !bForce)
	{
		// this shouldn't actually happen: some tile was still paged in
		return false;
	}

	for (uint uiTile = 0; uiTile < uiPAGE_EXTENT_IN_TILES * uiPAGE_EXTENT_IN_TILES; uiTile++)
	{
		if (pPage->m_pTiles[uiTile] != NULL)
		{
			Tile* pTile = pPage->m_pTiles[uiTile];

			pTile->m_parent->m_children[pTile->m_uiChild] = NULL;
			pPage->m_pTiles[uiTile] = NULL;

			delete pTile;
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif

			m_uiStatNumTiles--;
		}
	}

	return bAllPagedOut;
}


float DataLoader::ComputePriority(const Box* pBox)
{
	float priority = 0.0;
	for( uint i = 0 ; i < m_views.size() ; i++ ) {
		tum3D::Vec3f v = pBox->m_vCenter - m_views[i].position;
		tum3D::normalize( v );
		float dotProduct = tum3D::dotProd( v, m_views[i].normalizedViewDirection );
		if( dotProduct > 0 ) {
			priority += dotProduct;
		}
	}
	return priority;
}


bool DataLoader::GetHeight(float fX, float fY, float* pfHeight) const
{
	int iTileTreeX = static_cast<int>(floor(fX / m_fTileExtents[0]));
	int iTileTreeY = static_cast<int>(floor(fY / m_fTileExtents[0]));
	if (iTileTreeX < m_treesInfo.m_iTreeX0 || iTileTreeX > m_treesInfo.m_iTreeX1 || iTileTreeY < m_treesInfo.m_iTreeY0 || iTileTreeY > m_treesInfo.m_iTreeY1)
	{
		return false;
	}
	fX -= static_cast<float>(iTileTreeX) * m_fTileExtents[0];
	fY -= static_cast<float>(iTileTreeY) * m_fTileExtents[0];

	const TileTree* pTileTree = GetTileTree(iTileTreeX, iTileTreeY);
	if (pTileTree == 0)
	{
		return false;
	}

	Tile* pTile = pTileTree->m_root->m_children[0];
	if (pTile == 0)
	{
		return false;
	}

	uint uiLevel = 0;
	while (pTile->m_children[0] != 0)
	{
		uiLevel++;

		uint uiChildX = static_cast<uint>(fX / m_fTileExtents[uiLevel]);
		uint uiChildY = static_cast<uint>(fY / m_fTileExtents[uiLevel]);

		fX -= static_cast<float>(uiChildX) * m_fTileExtents[uiLevel];
		fY -= static_cast<float>(uiChildY) * m_fTileExtents[uiLevel];

		uint uiChild = uiChildX + uiChildY * 2;

		pTile = pTile->m_children[uiChild];
	}

	return pTile->GetHeight(fX, fY, pfHeight);
}


bool DataLoader::ComputeRayTerrainIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, tum3D::Vec3f* pvIntersectionPoint) const
{
	std::vector<TileSortItem> tilesToTest;

	for (int iY = m_treesInfo.m_iTreeY0; iY <= m_treesInfo.m_iTreeY1; iY++)
	{
		for (int iX = m_treesInfo.m_iTreeX0; iX <= m_treesInfo.m_iTreeX1; iX++)
		{
			const TileTree* pTileTree = GetTileTree(iX, iY);
			if (pTileTree != 0)
			{
				if (pTileTree->m_root->m_children[0] != 0)
				{
					ComputeTilesToTestForIntersection(vRayOrigin, vRayDirection, tilesToTest, pTileTree->m_root->m_children[0]);
				}
			}
		}
	}

	std::sort(tilesToTest.begin(), tilesToTest.end());

	for (uint uiTile = 0; uiTile < tilesToTest.size(); uiTile++)
	{
		Tile* pTile = tilesToTest[uiTile].m_pTile;
		if (pTile->ComputeRayTerrainIntersection(vRayOrigin, vRayDirection, pvIntersectionPoint))
		{
			return true;
		}
	}

	return false;
}


void DataLoader::ComputeTilesToTestForIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, std::vector<TileSortItem>& tilesToTest, Tile* pTile)
{
	float fDistance;
	if (ComputeRayBoxIntersection(vRayOrigin, vRayDirection, &pTile->m_BBox, &fDistance))
	{
		if (pTile->m_children[0] != 0)
		{
			for (uint uiChild = 0; uiChild < 4; uiChild++)
			{
				ComputeTilesToTestForIntersection(vRayOrigin, vRayDirection, tilesToTest, pTile->m_children[uiChild]);
			}
		}
		else
		{
			TileSortItem item;
			item.m_pTile = pTile;
			item.m_fDistance = fDistance;
			tilesToTest.push_back(item);
		}
	}
}

bool DataLoader::ComputeRayBoxIntersection(const tum3D::Vec3f& vRayOrigin, const tum3D::Vec3f& vRayDirection, const Box* pBox, float* pfDistance)
{
	tum3D::Vec3f vCenter = pBox->m_vCenter;
	tum3D::Vec3f vHalfExtent = pBox->m_vHalfExtent;
	
	tum3D::Vec3f vDiff = vCenter - vRayOrigin;

	// Falls Position innerhalb der Box: Schnittpunkt == Ray Origin
	if (abs(vDiff.x()) <= vHalfExtent.x() && abs(vDiff.y()) <= vHalfExtent.y() && abs(vDiff.z()) <= vHalfExtent.z())
	{
		*pfDistance = 0.0f;
		return true;
	}
	
	float fRayDirectionLength = vRayDirection.normf();

	// Prüfe Seitenflächen senkrecht zur x-Achse
	if (vRayDirection.x() != 0.0f)
	{
		float t = (vDiff.x() - sgn(vRayDirection.x()) * vHalfExtent.x()) / vRayDirection.x();
		if (t > 0)
		{
			if (abs(vDiff.y() - t * vRayDirection.y()) <= vHalfExtent.y() && abs(vDiff.z() - t * vRayDirection.z()) <= vHalfExtent.z())
			{
				*pfDistance = t * fRayDirectionLength;
				return true;
			}
		}
	}

	// Prüfe Seitenflächen senkrecht zur y-Achse
	if (vRayDirection.y() != 0.0f)
	{
		float t = (vDiff.y() - sgn(vRayDirection.y()) * vHalfExtent.y()) / vRayDirection.y();
		if (t > 0)
		{
			if (abs(vDiff.x() - t * vRayDirection.x()) <= vHalfExtent.x() && abs(vDiff.z() - t * vRayDirection.z()) <= vHalfExtent.z())
			{
				*pfDistance = t * fRayDirectionLength;
				return true;
			}
		}
	}

	// Prüfe Seitenflächen senkrecht zur z-Achse
	if (vRayDirection.z() != 0.0f)
	{
		float t = (vDiff.z() - sgn(vRayDirection.z()) * vHalfExtent.z()) / vRayDirection.z();
		if (t > 0)
		{
			if (abs(vDiff.x() - t * vRayDirection.x()) <= vHalfExtent.x() && abs(vDiff.y() - t * vRayDirection.y()) <= vHalfExtent.y())
			{
				*pfDistance = t * fRayDirectionLength;
				return true;
			}
		}
	}

	return false;
}


void DataLoader::CleanUp(Page* pPage) // Räumt die Kinder einer Page auf
{

	for (uint uiChild = 0; uiChild < 4; uiChild++)
	{

		if (pPage->m_children[uiChild] != NULL)
		{
			Page* pChildPage = pPage->m_children[uiChild];

			CleanUp(pChildPage);

			if (pChildPage->m_pPageData != NULL)
			{
				// Page geladen
				// force removing of tiles (even if still paged in, which shouldn't happen)
				bool bOK = RemoveTiles(pChildPage, true);
				assert(bOK);

				BOOL bResult = VirtualFree(pChildPage->m_pPageData, 0, MEM_RELEASE);
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
				if (!bResult)
				{
					// VirtualFree ist fehlgeschlagen
					MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
					exit(-1);
				}
			}
			else
			{
				// Page angefordert
				PageDataRequest* pPageDataRequest = pChildPage->m_pPageDataRequest;
				if (pPageDataRequest->m_pPageData != NULL)
				{
					BOOL bResult = VirtualFree(pPageDataRequest->m_pPageData, 0, MEM_RELEASE);
#ifdef _DEBUG
					m_i64NumObjectsDestroyed++;
#endif
					if (!bResult)
					{
						// VirtualFree ist fehlgeschlagen
						MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
						exit(-1);
					}
				}
				delete pPageDataRequest;
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
			}
			
			delete pChildPage;
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif

		}
		else if (pPage->m_cachedChildren[uiChild] != NULL)
		{
			// Page geladen
			Page* pChildPage = pPage->m_cachedChildren[uiChild];

			CleanUp(pChildPage);

			BOOL bResult = VirtualFree(pChildPage->m_pPageData, 0, MEM_RELEASE);
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif
			if (!bResult)
			{
				// VirtualFree ist fehlgeschlagen
				MessageBox(NULL, "VirtualFree failed\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
				exit(-1);
			}
			delete pChildPage;
#ifdef _DEBUG
			m_i64NumObjectsDestroyed++;
#endif

		}

	}

}


DataLoader::~DataLoader()
{
	for (uint uiSlot = 0; uiSlot < uiNUM_LOADING_SLOTS; uiSlot++)
	{
		if (m_loadingSlots[uiSlot] != NULL)
		{
			CancelIo(m_loadingSlots[uiSlot]->m_hFile);
		}
		CloseHandle(m_hCompletionEvents[uiSlot]);
	}

	// Speicher freigeben:
	// + PageTree-Objekte, Page-Objekte, PageDataRequest-Objekte, TileTree-Objekte, Tile-Objekte
	// + Virtueller Speicher Page::m_pPageData, PageDataRequest::m_pPageData freigeben
	for (uint uiY = 0; uiY < m_treesInfo.m_uiNumTrees[1]; uiY++)
	{
		for (uint uiX = 0; uiX < m_treesInfo.m_uiNumTrees[0]; uiX++)
		{
			PageTree* pPageTree = m_pPageTrees[uiX][uiY];
			if (pPageTree != NULL)
			{
				CleanUp(pPageTree->m_root);

				delete m_pTileTrees[uiX][uiY]->m_root;
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
				delete m_pTileTrees[uiX][uiY];
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
				delete pPageTree->m_root;
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
				delete pPageTree;
#ifdef _DEBUG
				m_i64NumObjectsDestroyed++;
#endif
			}
		}
	}

#ifdef _DEBUG
	assert(m_i64NumObjectsCreated + m_i64NumObjectsCreatedLoaderThread == m_i64NumObjectsDestroyed);
#endif

	delete m_pFilePool;


}
