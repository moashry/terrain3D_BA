/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "occlusionculling.h"

#include <assert.h>
#include <float.h>

OcclusionCulling::OcclusionCulling()
{
	m_horizonRoot = 0;

	m_uiStatNumHorizonNodes = 0;
}


void OcclusionCulling::SetParameters(float fZNear)
{
	m_fZNear = fZNear;
}


void OcclusionCulling::Update(const RawVec3f pvPosition, const RawVec3f pvViewDirection)
{
	tum3D::Vec3f vHorizontalViewDirection = pvViewDirection;
	if (vHorizontalViewDirection.x() == 0.0f && vHorizontalViewDirection.y() == 0.0f)
	{
		// Kein Occlusion Culling verwenden, wenn der Betrachter senkrecht nach unten blickt
		m_bUseOcclusionCulling = false;
		return;
	}
	m_bUseOcclusionCulling = true;
	vHorizontalViewDirection.z() = 0.0f;
	m_vNormalizedHorizontalViewDirection = vHorizontalViewDirection;
	tum3D::normalize( m_vNormalizedHorizontalViewDirection );

	m_vPosition = pvPosition;

	if (m_horizonRoot != 0)
	{
		ReleaseHorizonNodes(m_horizonRoot);
	}

	m_uiStatNumHorizonNodes = 0;

	m_horizonRoot = GetHorizonNode();
	memset(m_horizonRoot, 0, sizeof(HorizonNode));
	m_horizonRoot->m_fLeft = -FLT_MAX;
	m_horizonRoot->m_fRight = FLT_MAX;
	m_horizonRoot->m_fMinHeight = -FLT_MAX;
	m_horizonRoot->m_fMaxHeight = -FLT_MAX;
}


bool OcclusionCulling::IsVisible(const Box* pBox)
{
	if (!m_bUseOcclusionCulling)
	{
		return true;
	}

	// Occlusion Culling nur verwenden, wenn die Box vollständig in dem von der
	// Betrachterposition abgewandten Halbraum einer Ebene liegt, die parallel zur
	// Projektionsebene verläuft und von der Betrachterposition den Abstand zNear hat
	tum3D::Vec3f vDiff = pBox->m_vCenter - m_vPosition;
	tum3D::Vec3f vHalfExtent = pBox->m_vHalfExtent;
	float zcenter = m_vNormalizedHorizontalViewDirection.x() * vDiff.x() + m_vNormalizedHorizontalViewDirection.y() * vDiff.y();
	float reff = vHalfExtent.x() * abs(m_vNormalizedHorizontalViewDirection.x()) + vHalfExtent.y() * abs(m_vNormalizedHorizontalViewDirection.y());
	if (zcenter - reff < m_fZNear)
	{
		return true;
	}

	// Bestimme die x- und y-World-Koordinaten des bei Projektion auf die Ebene linken und rechten Vertex
	// der Ober- bzw. Unterseite der Bounding-Box
	tum3D::Vec2f vLeft, vRight;
	if (vDiff.x() < -vHalfExtent.x())
	{
		if (vDiff.y() < -vHalfExtent.y())
		{
			vLeft.x() = vDiff.x() - vHalfExtent.x();
			vLeft.y() = vDiff.y() + vHalfExtent.y();
			vRight.x() = vDiff.x() + vHalfExtent.x();
			vRight.y() = vDiff.y() - vHalfExtent.y();
		}
		else if (vDiff.y() > vHalfExtent.y())
		{
			vLeft.x() = vDiff.x() + vHalfExtent.x();
			vLeft.y() = vDiff.y() + vHalfExtent.y();
			vRight.x() = vDiff.x() - vHalfExtent.x();
			vRight.y() = vDiff.y() - vHalfExtent.y();
		}
		else // -vHalfExtent.y <= vDiff.y <= vHalfExtent.y
		{
			vLeft.x() = vDiff.x() + vHalfExtent.x();
			vLeft.y() = vDiff.y() + vHalfExtent.y();
			vRight.x() = vDiff.x() + vHalfExtent.x();
			vRight.y() = vDiff.y() - vHalfExtent.y();
		}
	}
	else if (vDiff.x() > vHalfExtent.x())
	{
		if (vDiff.y() < -vHalfExtent.y())
		{
			vLeft.x() = vDiff.x() - vHalfExtent.x();
			vLeft.y() = vDiff.y() - vHalfExtent.y();
			vRight.x() = vDiff.x() + vHalfExtent.x();
			vRight.y() = vDiff.y() + vHalfExtent.y();
		}
		else if (vDiff.y() > vHalfExtent.y())
		{
			vLeft.x() = vDiff.x() + vHalfExtent.x();
			vLeft.y() = vDiff.y() - vHalfExtent.y();
			vRight.x() = vDiff.x() - vHalfExtent.x();
			vRight.y() = vDiff.y() + vHalfExtent.y();
		}
		else // -vHalfExtent.y <= vDiff.y <= vHalfExtent.y
		{
			vLeft.x() = vDiff.x() - vHalfExtent.x();
			vLeft.y() = vDiff.y() - vHalfExtent.y();
			vRight.x() = vDiff.x() - vHalfExtent.x();
			vRight.y() = vDiff.y() + vHalfExtent.y();
		}
	}
	else // -vHalfExtent.x <= vDiff.x <= vHalfExtent.x
	{
		if (vDiff.y() < -vHalfExtent.y())
		{
			vLeft.x() = vDiff.x() - vHalfExtent.x();
			vLeft.y() = vDiff.y() + vHalfExtent.y();
			vRight.x() = vDiff.x() + vHalfExtent.x();
			vRight.y() = vDiff.y() + vHalfExtent.y();
		}
		else
		{
			assert(vDiff.y() > vHalfExtent.y());

			vLeft.x() = vDiff.x() + vHalfExtent.x();
			vLeft.y() = vDiff.y() - vHalfExtent.y();
			vRight.x() = vDiff.x() - vHalfExtent.x();
			vRight.y() = vDiff.y() - vHalfExtent.y();
		}
	}

	// Berechne Occludee, gegeben durch fXLeft, fXRight, fZ
	float tLeft = vLeft.x() * m_vNormalizedHorizontalViewDirection.x() + vLeft.y() * m_vNormalizedHorizontalViewDirection.y();
	assert(tLeft > 0.0f);
	float fXLeft = (vLeft.y() * m_vNormalizedHorizontalViewDirection.x() - vLeft.x() * m_vNormalizedHorizontalViewDirection.y()) / tLeft;

	float tRight = vRight.x() * m_vNormalizedHorizontalViewDirection.x() + vRight.y() * m_vNormalizedHorizontalViewDirection.y();
	assert(tRight > 0.0f);
	float fXRight = (vRight.y() * m_vNormalizedHorizontalViewDirection.x() - vRight.x() * m_vNormalizedHorizontalViewDirection.y()) / tRight;

	float z = vDiff.z() + vHalfExtent.z();
	float t;
	if (z >= 0)
	{
		// Minimiere t
		t = (vDiff.x() - sgn(m_vNormalizedHorizontalViewDirection.x()) * vHalfExtent.x()) * m_vNormalizedHorizontalViewDirection.x()
			+ (vDiff.y() - sgn(m_vNormalizedHorizontalViewDirection.y()) * vHalfExtent.y()) * m_vNormalizedHorizontalViewDirection.y();
	}
	else
	{
		// Maximiere t
		t = (vDiff.x() + sgn(m_vNormalizedHorizontalViewDirection.x()) * vHalfExtent.x()) * m_vNormalizedHorizontalViewDirection.x()
			+ (vDiff.y() + sgn(m_vNormalizedHorizontalViewDirection.y()) * vHalfExtent.y()) * m_vNormalizedHorizontalViewDirection.y();
	}
	assert(t > 0.0f);	
	float fZ = z / t;

	bool bIsVisible = TestOccludee(m_horizonRoot, fXLeft, fXRight, fZ);

	if (bIsVisible)
	{
		// Berechne Occluder, gegeben durch fXLeft, fXRight, fZ
		z = vDiff.z() - vHalfExtent.z();

		float fZLeft, fZRight;
		fZLeft = z / tLeft;
		fZRight = z / tRight;
		fZ = std::min(fZLeft, fZRight);
		
		AddOccluder(m_horizonRoot, fXLeft, fXRight, fZ);
		return true;
	}
	else
	{
		return false;
	}

}


bool OcclusionCulling::TestOccludee(HorizonNode* pNode, float fXLeft, float fXRight, float fZ)
{
	assert(pNode->m_fLeft <= fXLeft && fXLeft < fXRight && fXRight <= pNode->m_fRight);
	assert(pNode->m_fMinHeight <= pNode->m_fMaxHeight);

	if (fZ <= pNode->m_fMinHeight)
	{
		return false;
	}
	else if (pNode->m_fMaxHeight < fZ)
	{
		return true;
	}
	else // m_fMinHeight < fZ <= m_fMaxHeight
	{
		assert(pNode->m_leftChild != 0); // Ist der Knoten ein Blatt, dann ist m_fMinHeight == m_fMaxHeight
			
		float fCut = pNode->m_leftChild->m_fRight;

		if (fXLeft < fCut)
		{
			if (fXRight <= fCut)
			{
				return TestOccludee(pNode->m_leftChild, fXLeft, fXRight, fZ);
			}
			else // fXLeft < fCut < fXRight
			{
				return TestOccludee(pNode->m_leftChild, fXLeft, fCut, fZ)
					|| TestOccludee(pNode->m_rightChild, fCut, fXRight, fZ);
			}
		}
		else // fCut <= fXLeft
		{
			return TestOccludee(pNode->m_rightChild, fXLeft, fXRight, fZ);
		}
	}
}


void OcclusionCulling::AddOccluder(HorizonNode* pNode, float fXLeft, float fXRight, float fZ)
{
	assert(pNode->m_fLeft <= fXLeft && fXLeft < fXRight && fXRight <= pNode->m_fRight);
	assert(pNode->m_fMinHeight <= pNode->m_fMaxHeight);

	if (fZ > pNode->m_fMinHeight)
	{
		if (pNode->m_fLeft == fXLeft && pNode->m_fRight == fXRight && pNode->m_fMaxHeight <= fZ)
		{
			if (pNode->m_leftChild != 0)
			{
				ReleaseHorizonNodes(pNode->m_leftChild);
				pNode->m_leftChild = 0;
				ReleaseHorizonNodes(pNode->m_rightChild);
				pNode->m_rightChild = 0;
			}
			pNode->m_fMinHeight = fZ;
			pNode->m_fMaxHeight = fZ;
		}
		else
		{
			if (pNode->m_leftChild != 0)
			{
				// Kinder vorhanden
				float fCut = pNode->m_leftChild->m_fRight;

				if (fXLeft < fCut)
				{
					if (fXRight <= fCut)
					{
						AddOccluder(pNode->m_leftChild, fXLeft, fXRight, fZ);
					}
					else // fXLeft < fCut < fXRight
					{
						AddOccluder(pNode->m_leftChild, fXLeft, fCut, fZ);
						AddOccluder(pNode->m_rightChild, fCut, fXRight, fZ);
					}
				}
				else // fCut <= fXLeft
				{
					AddOccluder(pNode->m_rightChild, fXLeft, fXRight, fZ);

				}

				pNode->m_fMinHeight = std::min(pNode->m_leftChild->m_fMinHeight, pNode->m_rightChild->m_fMinHeight);
				pNode->m_fMaxHeight = std::max(pNode->m_leftChild->m_fMaxHeight, pNode->m_rightChild->m_fMaxHeight);
			}
			else
			{
				// Keine Kinder vorhanden
				if (fXRight < pNode->m_fRight)
				{
					HorizonNode* pChildNode = GetHorizonNode();
					memset(pChildNode, 0, sizeof(HorizonNode));
					pChildNode->m_fLeft = fXRight;
					pChildNode->m_fRight = pNode->m_fRight;
					pChildNode->m_fMinHeight = pNode->m_fMinHeight;
					pChildNode->m_fMaxHeight = pNode->m_fMaxHeight;
					pNode->m_rightChild = pChildNode;

					pChildNode = GetHorizonNode();
					memset(pChildNode, 0, sizeof(HorizonNode));
					pChildNode->m_fLeft = pNode->m_fLeft;
					pChildNode->m_fRight = fXRight;
					pChildNode->m_fMinHeight = pNode->m_fMinHeight;
					pChildNode->m_fMaxHeight = pNode->m_fMaxHeight;
					pNode->m_leftChild = pChildNode;

					AddOccluder(pNode->m_leftChild, fXLeft, fXRight, fZ);

				}
				else // fXRight == pNode->m_fRight
				{
					assert(pNode->m_fLeft < fXLeft); // Der Fall pNode->m_fLeft == fXLeft && pNode->m_fRight == fXRight wurde schon behandelt

					HorizonNode* pChildNode = GetHorizonNode();
					memset(pChildNode, 0, sizeof(HorizonNode));
					pChildNode->m_fLeft = pNode->m_fLeft;
					pChildNode->m_fRight = fXLeft;
					pChildNode->m_fMinHeight = pNode->m_fMinHeight;
					pChildNode->m_fMaxHeight = pNode->m_fMaxHeight;
					pNode->m_leftChild = pChildNode;

					pChildNode = GetHorizonNode();
					memset(pChildNode, 0, sizeof(HorizonNode));
					pChildNode->m_fLeft = fXLeft;
					pChildNode->m_fRight = pNode->m_fRight;
					pChildNode->m_fMinHeight = fZ;
					pChildNode->m_fMaxHeight = fZ;
					pNode->m_rightChild = pChildNode;

				}
				
				pNode->m_fMaxHeight = fZ;
			}
		}
	}
}


uint OcclusionCulling::GetStatMaxDepth(HorizonNode* pNode)
{
	if (pNode->m_leftChild != 0)
	{
		uint uiMaxDepthLeft = GetStatMaxDepth(pNode->m_leftChild);
		uint uiMaxDepthRight = GetStatMaxDepth(pNode->m_rightChild);
		return 1 + std::max(uiMaxDepthLeft, uiMaxDepthRight);
	}
	else
	{
		return 0;
	}
}


void OcclusionCulling::ReleaseHorizonNodes(HorizonNode* pNode)
{
	if (pNode->m_leftChild != 0)
	{
		ReleaseHorizonNodes(pNode->m_leftChild);
		ReleaseHorizonNodes(pNode->m_rightChild);
	}
	m_horizonNodeCache.push_back(pNode);
	m_uiStatNumHorizonNodes--;
}


OcclusionCulling::~OcclusionCulling()
{
	if (m_horizonRoot != 0)
	{
		ReleaseHorizonNodes(m_horizonRoot);
	}
	for (uint ui = 0; ui < m_horizonNodeCache.size(); ui++)
	{
		delete m_horizonNodeCache[ui];
	}
}
