/******************************************************************************

overviewmap.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#ifndef __TUM3D__OVERVIEWMAP_H__
#define __TUM3D__OVERVIEWMAP_H__


#include "terrainLib/renderer/device.h"


class OverviewMap {
public:
	struct RenderState {
		// x, y, width, height in [0,1]
		float x;
		float y;
		float width;
		float height;
		// pos marker
		float posMarkerX;
		float posMarkerY;
		float posMarkerHeading;
	};

	OverviewMap( Renderer::Device* device, const char *filename, float x0, float y0, float extentX, float extentY );
	~OverviewMap();

	float GetAspectRatio() const { return m_aspectRatio; }

	// render the map into the rectangle x,y - x+maxWidth,y+maxHeight (coord system: 0,0 is upper left corner)
	void Render( const RenderState& state );

	//void GetPosition( const RenderState& state, float x, float y, float& mapX, float& mapY, bool& inOverviewMap ) const;

private:
	Renderer::Device* m_device;
#ifdef GLEW_MX
	GLEWContext* glewGetContext() { return m_device->glewGetContext(); }
#endif

	Renderer::FramebufferTexture2D* m_overviewMapTexture;
	float m_aspectRatio;

	Renderer::FramebufferTexture2D* m_posMarkerTexture;

	// position/extent of the map in world space
	float m_x0, m_y0, m_extentX, m_extentY;
};


#endif