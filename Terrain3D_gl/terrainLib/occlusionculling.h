/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

// Horizon Occlusion Culling
// F�r die Repr�sentation des Horizonts wird ein Bin�rbaum verwendet

#ifndef __TUM3D__OCCLUSIONCULLING_H__
#define __TUM3D__OCCLUSIONCULLING_H__

#include <vector>

#include "global.h"
#include "box.h"

struct HorizonNode
{
	float			m_fLeft, m_fRight;
	float			m_fMinHeight, m_fMaxHeight;
	HorizonNode*	m_leftChild;
	HorizonNode*	m_rightChild;
};


class OcclusionCulling
{
public:
	OcclusionCulling();
	~OcclusionCulling();
	void SetParameters(float fZNear);
	void Update(const RawVec3f pvPosition, const RawVec3f pvViewDirection);
	bool IsVisible(const Box* pBox);

	uint GetStatNumHorizonNodes();
	uint GetStatMaxDepth();

private:
	HorizonNode* GetHorizonNode();
	void ReleaseHorizonNodes(HorizonNode* pNode);

	bool TestOccludee(HorizonNode* pNode, float fXLeft, float fXRight, float fZ);
	void AddOccluder(HorizonNode* pNode, float fXLeft, float fXRight, float fZ);

	uint GetStatMaxDepth(HorizonNode* pNode);

	HorizonNode*	m_horizonRoot;
	bool			m_bUseOcclusionCulling;
	tum3D::Vec3f	m_vPosition;
	tum3D::Vec3f	m_vNormalizedHorizontalViewDirection;

	float			m_fZNear;

	uint			m_uiStatNumHorizonNodes;

	std::vector<HorizonNode*>	m_horizonNodeCache;

};

inline HorizonNode* OcclusionCulling::GetHorizonNode()
{
	HorizonNode* pNode;
	if (m_horizonNodeCache.size() > 0)
	{
		pNode = m_horizonNodeCache.back();
		m_horizonNodeCache.pop_back();
	}
	else
	{
		pNode = new HorizonNode();
	}
	m_uiStatNumHorizonNodes++;
	return pNode;
}

inline uint OcclusionCulling::GetStatNumHorizonNodes()
{
	return m_uiStatNumHorizonNodes;
}

inline uint OcclusionCulling::GetStatMaxDepth()
{
	if (m_horizonRoot != 0)
	{
		return GetStatMaxDepth(m_horizonRoot);
	}
	else
	{
		return 0;
	}
}

#endif