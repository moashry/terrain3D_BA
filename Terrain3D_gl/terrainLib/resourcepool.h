/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__RESOURCEPOOL_H__
#define __TUM3D__RESOURCEPOOL_H__

#include <vector>
#include <list>

#include "global.h"
#include "renderer/device.h"

class ResourcePool;
struct VBBlockDesc;

struct VBDesc
{
	Renderer::Buffer*			m_pVertexBuffer;
	std::vector<VBBlockDesc>	m_VBBlocks;
};

struct VBBlockDesc
{
	VBDesc*						m_pVBDesc;
	uint						m_uiBlockAddress;
	uint						m_uiLevel;
	std::list<VBBlockDesc*>::iterator	m_locator;
	bool						m_bIsInUse;
};

struct VBHandle
{
	ResourcePool*				m_pResourcePool;

	VBDesc*						m_pVBDesc;
	uint						m_uiBlockAddress;

	uint						m_uiOffset;
	uint						m_uiSize;

	std::list<VBHandle**>		m_references;
	uint						m_uiUsageCounter;
	std::list<VBHandle*>::iterator	m_VBCacheLocator;

	VBHandle(ResourcePool* pResourcePool) : m_pResourcePool(pResourcePool), m_pVBDesc(NULL), m_uiBlockAddress(0), m_uiOffset(0), m_uiSize(0), m_uiUsageCounter(0) { }
};

struct TextureHandle
{
	ResourcePool*				m_pResourcePool;

	Renderer::FramebufferTexture2D*	m_pTexture;

	std::list<TextureHandle**>	m_references;
	uint						m_uiUsageCounter;
	std::list<TextureHandle*>::iterator	m_textureCacheLocator;

	TextureHandle(ResourcePool* pResourcePool) : m_pResourcePool(pResourcePool), m_pTexture(NULL), m_uiUsageCounter(0) { }
};


class ResourcePool
{
public:
	ResourcePool(
		uint uiVBBlockLevelZeroSize,
		uint uiNumVBBlockLevels,
		uint uiInitNumVBs,
		TEXTURE_COMPRESSION	eTextureCompression,
		uint uiTextureResolution,
		uint uiNumMipMapLevels,
		uint uiInitNumTextures
	);
	bool Create(Renderer::Device *pDevice);
	void SafeRelease(); // Erst aufrufen, wenn alle Referenzen freigegeben sind
	~ResourcePool();

	bool GetVB(uint uiSize, VBHandle** ppVBHandle); // Referenz auf freien Vertex Buffer anlegen
	void GetVB(VBHandle* pVBHandle, VBHandle** ppVBHandle); // Zus�tzliche Referenz auf Vertex Buffer anlegen
	void ReleaseVB(VBHandle** ppVBHandle); // Referenz freigeben
	uint IncrementVBUsageCounter(VBHandle* pVBHandle); // Usage Counter inkrementieren
	uint DecrementVBUsageCounter(VBHandle* pVBHandle); // Usage Counter dekrementieren

	bool GetTexture(TextureHandle** ppTextureHandle); // Referenz auf freie Textur anlegen
	void GetTexture(TextureHandle* pTextureHandle, TextureHandle** ppTextureHandle); // Zus�tzliche Referenz auf Textur anlegen
	void ReleaseTexture(TextureHandle** ppTextureHandle); // Referenz freigeben
	uint IncrementTextureUsageCounter(TextureHandle* pTextureHandle); // Usage Counter inkrementieren
	uint DecrementTextureUsageCounter(TextureHandle* pTextureHandle); // Usage Counter dekrementieren

	void FlushCache();

	uint GetStatNumVBsInUse();
	uint GetStatNumVBBlocksInUse();
	uint GetStatVBBlockMemoryUsage();
	uint GetStatNumTexturesInUse();
	uint GetStatTextureMemoryUsage();


private:
	bool CreateVB(VBDesc* pVBDesc);
	bool CreateTexture( Renderer::FramebufferTexture2D **pTexture );

	void InternalReleaseVB(VBHandle* pVBHandle);
	void InternalReleaseTexture(TextureHandle* pTextureHandle);

	Renderer::Device *m_pDevice;

	uint			m_uiVBBlockLevelZeroSize;
	uint			m_uiNumVBBlockLevels;
	uint			m_uiInitNumVBs;
	uint			m_uiVBNumVBBlocks;
	uint			m_uiVBSize;
	TEXTURE_COMPRESSION	m_eTextureCompression;
	uint			m_uiTextureResolution;
	uint			m_uiNumMipMapLevels;
	uint			m_uiInitNumTextures;

	std::vector<VBDesc*>					m_VBs;
	std::vector<std::list<VBBlockDesc*>>	m_freeVBBlocks;
	std::vector<Renderer::FramebufferTexture2D*>	m_textures;
	std::vector<Renderer::FramebufferTexture2D*>	m_freeTextures;

	std::list<VBHandle*>					m_VBCache;
	std::list<TextureHandle*>				m_textureCache;

	uint			m_uiStatNumVBsInUse;
	uint			m_uiStatNumVBBlocksInUse;
	uint			m_uiStatVBBlockMemoryUsage;
	uint			m_uiStatNumTexturesInUse;
	uint			m_uiStatTextureMemoryUsage;

	uint			m_uiStatEstimatedTextureSize;

};


inline uint ResourcePool::GetStatNumVBsInUse()
{
	return m_uiStatNumVBsInUse;
}

inline uint ResourcePool::GetStatNumVBBlocksInUse()
{
	return m_uiStatNumVBBlocksInUse;
}

inline uint ResourcePool::GetStatVBBlockMemoryUsage()
{
	return m_uiStatVBBlockMemoryUsage;
}

inline uint ResourcePool::GetStatNumTexturesInUse()
{
	return m_uiStatNumTexturesInUse;
}

inline uint ResourcePool::GetStatTextureMemoryUsage()
{
	return m_uiStatTextureMemoryUsage;
}

#endif