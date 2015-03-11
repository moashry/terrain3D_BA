/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "resourcepool.h"


ResourcePool::ResourcePool(uint uiVBBlockLevelZeroSize, uint uiNumVBBlockLevels, uint uiInitNumVBs, TEXTURE_COMPRESSION	eTextureCompression, uint uiTextureResolution, uint uiNumMipMapLevels, uint uiInitNumTextures)
: m_pDevice(0)
{
	m_uiStatNumVBsInUse = 0;
	m_uiStatNumVBBlocksInUse = 0;
	m_uiStatVBBlockMemoryUsage = 0;
	m_uiStatNumTexturesInUse = 0;
	m_uiStatTextureMemoryUsage = 0;
	
	m_uiVBBlockLevelZeroSize = uiVBBlockLevelZeroSize;
	m_uiNumVBBlockLevels = uiNumVBBlockLevels;
	m_uiInitNumVBs = uiInitNumVBs;
	m_uiVBNumVBBlocks = 1 << (m_uiNumVBBlockLevels - 1);
	m_uiVBSize = m_uiVBBlockLevelZeroSize << (m_uiNumVBBlockLevels - 1);

	m_eTextureCompression = eTextureCompression;
	assert(m_eTextureCompression == TEX_COMP_S3TC || m_eTextureCompression == TEX_COMP_S3TC_SHARED);
	m_uiTextureResolution = uiTextureResolution;
	m_uiNumMipMapLevels = uiNumMipMapLevels;
	m_uiInitNumTextures = uiInitNumTextures;

	m_uiStatEstimatedTextureSize = (m_uiTextureResolution * m_uiTextureResolution * 2) / 3; // 0,5 Byte pro Texel, Mipmap-Pyramide

	m_freeVBBlocks.resize(m_uiNumVBBlockLevels);
}


bool ResourcePool::Create(Renderer::Device *pDevice)
{
	m_pDevice = pDevice;

	// Erzeuge Ressourcen
	for (uint ui = 0; ui < m_uiInitNumVBs; ui++)
	{
		VBDesc* pVBDesc = new VBDesc();
		m_VBs.push_back(pVBDesc);
		V_RETURN( CreateVB(pVBDesc) );
		m_freeVBBlocks[m_uiNumVBBlockLevels - 1].push_back(&pVBDesc->m_VBBlocks[0]);
		pVBDesc->m_VBBlocks[0].m_uiLevel = m_uiNumVBBlockLevels - 1;
		pVBDesc->m_VBBlocks[0].m_locator = m_freeVBBlocks[m_uiNumVBBlockLevels - 1].end();
		pVBDesc->m_VBBlocks[0].m_locator--;
	}

	for (uint ui = 0; ui < m_uiInitNumTextures; ui++)
	{
		Renderer::FramebufferTexture2D* pTexture;
		V_RETURN( CreateTexture( &pTexture ) );
		m_textures.push_back( pTexture );
		m_freeTextures.push_back( pTexture );
	}

	return true;
}


bool ResourcePool::CreateVB(VBDesc* pVBDesc)
{
	memset(pVBDesc, 0, sizeof(VBDesc));

	// Vertex Buffer erzeugen
	Renderer::Buffer::Create( pVBDesc->m_pVertexBuffer, m_uiVBSize, GL_DYNAMIC_DRAW );

	// Block Descriptors erzeugen
	pVBDesc->m_VBBlocks.resize(m_uiVBNumVBBlocks);
	for (uint ui = 0; ui < m_uiVBNumVBBlocks; ui++)
	{
		memset(&pVBDesc->m_VBBlocks[ui], 0, sizeof(VBBlockDesc));
		pVBDesc->m_VBBlocks[ui].m_pVBDesc = pVBDesc;
		pVBDesc->m_VBBlocks[ui].m_uiBlockAddress = ui;
	}

	// Dummy-Daten in die Ressource schreiben, damit die Ressource in den Videospeicher eingelagert wird
	uchar uc = 0;
	pVBDesc->m_pVertexBuffer->SubData( 0, 1, &uc );

	return true;
}


bool ResourcePool::CreateTexture( Renderer::FramebufferTexture2D **pTexture )
{
	*pTexture = 0;
	Renderer::Texture2D::Create( m_uiTextureResolution, m_uiTextureResolution, m_uiNumMipMapLevels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, *pTexture );

	return true;
}


bool ResourcePool::GetVB(uint uiSize, VBHandle** ppVBHandle)
{
	uint uiRequestedLevel = 0;
	while ((m_uiVBBlockLevelZeroSize << uiRequestedLevel) < uiSize)
	{
		uiRequestedLevel++;
	}
	assert(uiRequestedLevel < m_uiNumVBBlockLevels);

	uint uiAvailableLevel;
	while (true)
	{
		uiAvailableLevel = uiRequestedLevel;
		while (uiAvailableLevel < m_uiNumVBBlockLevels && m_freeVBBlocks[uiAvailableLevel].size() == 0)
		{
			uiAvailableLevel++;
		}
		// Falls kein Block ausreichender Größe frei, Cache leeren
		if (uiAvailableLevel == m_uiNumVBBlockLevels && m_VBCache.size() > 0)
		{
			// Ersten Vertex Buffer aus Cache entfernen (LRU)
			VBHandle* pVBHandle = m_VBCache.front();
			m_VBCache.pop_front();

			// Referenzen freigeben
			for (std::list<VBHandle**>::iterator p = pVBHandle->m_references.begin(); p != pVBHandle->m_references.end(); p++)
			{
				**p = 0;
			}

			// Vertex Buffer freigeben
			InternalReleaseVB(pVBHandle);
		}
		else
		{
			break;
		}
	}

	if (uiAvailableLevel == m_uiNumVBBlockLevels)
	{
		VBDesc* pVBDesc = new VBDesc();
		m_VBs.push_back(pVBDesc);
		V_RETURN( CreateVB(pVBDesc) );
		m_freeVBBlocks[m_uiNumVBBlockLevels - 1].push_back(&pVBDesc->m_VBBlocks[0]);
		pVBDesc->m_VBBlocks[0].m_locator = --m_freeVBBlocks[m_uiNumVBBlockLevels - 1].end();
		
		uiAvailableLevel = m_uiNumVBBlockLevels - 1;
	}

	if (uiAvailableLevel == m_uiNumVBBlockLevels - 1)
	{
		m_uiStatNumVBsInUse++;
	}

	VBBlockDesc* pVBBlockDesc = m_freeVBBlocks[uiAvailableLevel].back();
	m_freeVBBlocks[uiAvailableLevel].pop_back();
	while (uiAvailableLevel != uiRequestedLevel)
	{
		uiAvailableLevel--;
		// Split
		uint uiBuddyBlockAddress = pVBBlockDesc->m_uiBlockAddress ^ (1 << uiAvailableLevel);
		VBBlockDesc* pBuddyVBBlockDesc = &pVBBlockDesc->m_pVBDesc->m_VBBlocks[uiBuddyBlockAddress];
		m_freeVBBlocks[uiAvailableLevel].push_back(pBuddyVBBlockDesc);
		pBuddyVBBlockDesc->m_uiLevel = uiAvailableLevel;
		pBuddyVBBlockDesc->m_locator = --m_freeVBBlocks[uiAvailableLevel].end();

		assert(!pBuddyVBBlockDesc->m_bIsInUse);
	}
	
	pVBBlockDesc->m_uiLevel = uiRequestedLevel;
	pVBBlockDesc->m_bIsInUse = true;

	VBHandle* pVBHandle = new VBHandle(this);
	pVBHandle->m_pVBDesc = pVBBlockDesc->m_pVBDesc;
	pVBHandle->m_uiBlockAddress = pVBBlockDesc->m_uiBlockAddress;
	pVBHandle->m_uiOffset = pVBBlockDesc->m_uiBlockAddress * m_uiVBBlockLevelZeroSize;
	pVBHandle->m_uiSize = m_uiVBBlockLevelZeroSize << uiRequestedLevel;
	
	// Referenz anlegen
	pVBHandle->m_references.push_back(ppVBHandle);
	*ppVBHandle = pVBHandle;

	// Vertex Buffer zum Cache hinzufügen (denn m_uiUsageCounter == 0)
	m_VBCache.push_back(pVBHandle);
	pVBHandle->m_VBCacheLocator = --m_VBCache.end();

	m_uiStatNumVBBlocksInUse++;
	m_uiStatVBBlockMemoryUsage += m_uiVBBlockLevelZeroSize << uiRequestedLevel;

	return true;
}


void ResourcePool::GetVB(VBHandle* pVBHandle, VBHandle** ppVBHandle)
{
	// Referenz anlegen
	pVBHandle->m_references.push_back(ppVBHandle);
	*ppVBHandle = pVBHandle;
}


void ResourcePool::ReleaseVB(VBHandle** ppVBHandle)
{
	VBHandle* pVBHandle = *ppVBHandle;
	// Referenz freigeben
	pVBHandle->m_references.remove(ppVBHandle);
	*ppVBHandle = 0;

	// Wenn es die letzte Referenz war, Vertex Buffer freigeben
	if (pVBHandle->m_references.size() == 0)
	{
		if (pVBHandle->m_uiUsageCounter == 0)
		{
			// Vertex Buffer aus Cache entfernen
			m_VBCache.erase(pVBHandle->m_VBCacheLocator);
		}
		// Vertex Buffer freigeben
		InternalReleaseVB(pVBHandle);
	}
}


uint ResourcePool::IncrementVBUsageCounter(VBHandle* pVBHandle)
{
	// Wenn m_uiUsageCounter == 0, Vertex Buffer aus Cache entfernen
	if (pVBHandle->m_uiUsageCounter == 0)
	{
		// Vertex Buffer aus Cache entfernen
		m_VBCache.erase(pVBHandle->m_VBCacheLocator);
	}
	pVBHandle->m_uiUsageCounter++;

	return pVBHandle->m_uiUsageCounter;
}


uint ResourcePool::DecrementVBUsageCounter(VBHandle* pVBHandle)
{
	assert(pVBHandle->m_uiUsageCounter > 0);

	pVBHandle->m_uiUsageCounter--;
	// Wenn m_uiUsageCounter == 0, Vertex Buffer zum Cache hinzufügen
	if (pVBHandle->m_uiUsageCounter == 0)
	{
		// Vertex Buffer zum Cache hinzufügen
		m_VBCache.push_back(pVBHandle);
		pVBHandle->m_VBCacheLocator = --m_VBCache.end();
	}

	return pVBHandle->m_uiUsageCounter;
}


void ResourcePool::InternalReleaseVB(VBHandle* pVBHandle)
{
	VBDesc* pVBDesc = pVBHandle->m_pVBDesc;
	VBBlockDesc* pVBBlockDesc = &pVBDesc->m_VBBlocks[pVBHandle->m_uiBlockAddress];
	uint uiLevel = pVBBlockDesc->m_uiLevel;

	assert(m_uiStatNumVBBlocksInUse > 0);
	assert(m_uiStatVBBlockMemoryUsage >= (m_uiVBBlockLevelZeroSize << uiLevel));
	m_uiStatNumVBBlocksInUse--;
	m_uiStatVBBlockMemoryUsage -= m_uiVBBlockLevelZeroSize << uiLevel;

	pVBBlockDesc->m_bIsInUse = false;

	while (uiLevel < m_uiNumVBBlockLevels - 1)
	{
		// Combine
		uint uiBuddyBlockAddress = pVBBlockDesc->m_uiBlockAddress ^ (1 << uiLevel);
		VBBlockDesc* pBuddyVBBlockDesc = &pVBDesc->m_VBBlocks[uiBuddyBlockAddress];
		if (pBuddyVBBlockDesc->m_uiLevel != uiLevel || pBuddyVBBlockDesc->m_bIsInUse)
		{
			break;
		}
		m_freeVBBlocks[uiLevel].erase(pBuddyVBBlockDesc->m_locator);
		
		assert(std::min(uiBuddyBlockAddress, pVBBlockDesc->m_uiBlockAddress) == (pVBBlockDesc->m_uiBlockAddress & ~(1 << uiLevel)));

		pVBBlockDesc = &pVBDesc->m_VBBlocks[pVBBlockDesc->m_uiBlockAddress & ~(1 << uiLevel)];
		uiLevel++;
	}

	if (uiLevel == m_uiNumVBBlockLevels - 1)
	{
		m_uiStatNumVBsInUse--;
	}

	m_freeVBBlocks[uiLevel].push_back(pVBBlockDesc);
	pVBBlockDesc->m_uiLevel = uiLevel;
	pVBBlockDesc->m_locator = --m_freeVBBlocks[uiLevel].end();

	delete pVBHandle;
}


bool ResourcePool::GetTexture(TextureHandle** ppTextureHandle)
{
	// Falls keine Textur frei, Cache leeren
	if (m_freeTextures.size() == 0 && m_textureCache.size() > 0)
	{
		// Erste Textur aus Cache entfernen (LRU)
		TextureHandle* pTextureHandle = m_textureCache.front();
		m_textureCache.pop_front();

		// Referenzen freigeben
		for (std::list<TextureHandle**>::iterator p = pTextureHandle->m_references.begin(); p != pTextureHandle->m_references.end(); p++)
		{
			**p = 0;
		}

		// Textur freigeben
		InternalReleaseTexture(pTextureHandle);
	}

	Renderer::FramebufferTexture2D *pTexture;
	if( m_freeTextures.size() == 0 )
	{
		V_RETURN( CreateTexture( &pTexture ) );
		m_textures.push_back( pTexture );
	}
	else
	{
		pTexture = m_freeTextures.back();
		m_freeTextures.pop_back();
	}

	TextureHandle* pTextureHandle = new TextureHandle(this);
	pTextureHandle->m_pTexture = pTexture;

	// Referenz anlegen
	pTextureHandle->m_references.push_back(ppTextureHandle);
	*ppTextureHandle = pTextureHandle;

	// Textur zum Cache hinzufügen (denn m_uiUsageCounter == 0)
	m_textureCache.push_back(pTextureHandle);
	pTextureHandle->m_textureCacheLocator = --m_textureCache.end();

	m_uiStatNumTexturesInUse++;
	m_uiStatTextureMemoryUsage += m_uiStatEstimatedTextureSize;

	return true;
}


void ResourcePool::GetTexture(TextureHandle* pTextureHandle, TextureHandle** ppTextureHandle)
{
	// Referenz anlegen
	pTextureHandle->m_references.push_back(ppTextureHandle);
	*ppTextureHandle = pTextureHandle;
}


void ResourcePool::ReleaseTexture(TextureHandle** ppTextureHandle)
{
	TextureHandle* pTextureHandle = *ppTextureHandle;
	// Referenz freigeben
	pTextureHandle->m_references.remove(ppTextureHandle);
	*ppTextureHandle = 0;

	// Wenn es die letzte Referenz war, Textur freigeben
	if (pTextureHandle->m_references.size() == 0)
	{
		if (pTextureHandle->m_uiUsageCounter == 0)
		{
			// Textur aus Cache entfernen
			m_textureCache.erase(pTextureHandle->m_textureCacheLocator);
		}
		// Textur freigeben
		InternalReleaseTexture(pTextureHandle);
	}
}


uint ResourcePool::IncrementTextureUsageCounter(TextureHandle* pTextureHandle)
{
	// Wenn m_uiUsageCounter == 0, Textur aus Cache entfernen
	if (pTextureHandle->m_uiUsageCounter == 0)
	{
		// Textur aus Cache entfernen
		m_textureCache.erase(pTextureHandle->m_textureCacheLocator);
	}
	pTextureHandle->m_uiUsageCounter++;

	return pTextureHandle->m_uiUsageCounter;
}


uint ResourcePool::DecrementTextureUsageCounter(TextureHandle* pTextureHandle)
{
	assert(pTextureHandle->m_uiUsageCounter > 0);

	pTextureHandle->m_uiUsageCounter--;
	// Wenn m_uiUsageCounter == 0, Textur zum Cache hinzufügen
	if (pTextureHandle->m_uiUsageCounter == 0)
	{
		// Textur zum Cache hinzufügen
		m_textureCache.push_back(pTextureHandle);
		pTextureHandle->m_textureCacheLocator = --m_textureCache.end();
	}

	return pTextureHandle->m_uiUsageCounter;
}


void ResourcePool::InternalReleaseTexture(TextureHandle* pTextureHandle)
{
	m_freeTextures.push_back(pTextureHandle->m_pTexture);

	delete pTextureHandle;

	assert(m_uiStatNumTexturesInUse > 0);
	assert(m_uiStatTextureMemoryUsage >= m_uiStatEstimatedTextureSize);
	m_uiStatNumTexturesInUse--;
	m_uiStatTextureMemoryUsage -= m_uiStatEstimatedTextureSize;

}


void ResourcePool::FlushCache()
{
	for (std::list<VBHandle*>::iterator p = m_VBCache.begin(); p != m_VBCache.end(); p++)
	{
		VBHandle* pVBHandle = *p;

		// Referenzen freigeben
		for (std::list<VBHandle**>::iterator q = pVBHandle->m_references.begin(); q != pVBHandle->m_references.end(); q++)
		{
			**q = 0;
		}

		// Vertex Buffer freigeben
		InternalReleaseVB(pVBHandle);
	}
	m_VBCache.clear();

	for (std::list<TextureHandle*>::iterator p = m_textureCache.begin(); p != m_textureCache.end(); p++)
	{
		TextureHandle* pTextureHandle = *p;

		// Referenzen freigeben
		for (std::list<TextureHandle**>::iterator q = pTextureHandle->m_references.begin(); q != pTextureHandle->m_references.end(); q++)
		{
			**q = 0;
		}

		// Textur freigeben
		InternalReleaseTexture(pTextureHandle);

	}
	m_textureCache.clear();

}


void ResourcePool::SafeRelease()
{
	FlushCache();

	for (uint ui = 0; ui < m_VBs.size(); ui++)
	{
		SAFE_DELETE(m_VBs[ui]->m_pVertexBuffer);
		delete m_VBs[ui];
	}
	for (uint ui = 0; ui < m_textures.size(); ui++)
	{
		SAFE_DELETE( m_textures[ui] );
	}

	m_VBs.clear();
#ifdef _DEBUG
	for (uint ui = 0; ui < m_uiNumVBBlockLevels - 1; ui++)
	{
		assert(m_freeVBBlocks[ui].size() == 0);
	}
#endif
	m_freeVBBlocks[m_uiNumVBBlockLevels - 1].clear();
	m_textures.clear();
	m_freeTextures.clear();

	m_pDevice = 0;
}


ResourcePool::~ResourcePool()
{
}
