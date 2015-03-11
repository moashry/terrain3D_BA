/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__GEOMETRYDECOMPRESSOR_H__
#define __TUM3D__GEOMETRYDECOMPRESSOR_H__

#include <vector>

#include "Vec.h"

#include "renderer/device.h"

#include "decompression.h"
#include "decompressionStreamout.h"

class GeometryDecompressor
{
public:
	GeometryDecompressor();

	bool Create(Renderer::Device *pDevice);
	void SafeRelease();

	void AddJob(uint uiStripDataFormat,
		uint uiStripHeadersSize, const byte* pStripHeaders,
		uint uiStripDataSize, const byte* pStripData,
		uint uiNumTriangles,
		Renderer::Buffer *pVB, uint uiVBOffset, uint uiX, uint uiY, uint uiLevel
		);

	void FlushJobs();

private:
	static const uint			uiNUM_COLUMNS = 4096;
	static const uint			uiNUM_STRIPS_PER_COLUMN = 8;
	static const uint			uiMAX_NUM_JOBS = 256;

	uint						m_uiCurrentColumnOffset;
	uint						m_uiCurrentStripDataBufferOffset;

	Renderer::Device*			m_pDevice;

	decompression				m_decompressionEffect;
	decompressionStreamout		m_decompressionStreamoutEffect;

	Renderer::TextureBuffer*	m_pStripHeadersBuffer;
	Renderer::TextureBuffer*	m_pStripDataBuffer;

	Renderer::Buffer*			m_pVertexBuffer;
#if !defined( INDEXED_DRAWING )
	Renderer::Buffer*			m_pIndexBuffer;
#endif

	Renderer::FramebufferTexture2D*	m_pTriangleDataTexture[2];

	struct Job
	{
		uint				m_uiColumnOffset;
		uint				m_uiNumColumns;
		uint				m_uiStripDataBufferOffset;
		uint				m_uiStripDataFormat;
		uint				m_uiNumTriangles;
		Renderer::Buffer*	m_pVB;
		uint				m_uiVBOffset;

		uint				m_uiX, m_uiY, m_uiLevel;
	};
	std::vector<Job>		m_jobs;

	struct Vertex
	{
		tum3D::Vec2f		m_vPos;
		uint				m_uiStripDataBufferOffset;
		uint				m_uiStripDataFormat;
	};
	std::vector<Vertex>		m_vertices;

};

#endif