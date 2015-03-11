/******************************************************************************

overviewmap.cpp

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/
#include "overviewmap.h"

#include "posmarker.h"


OverviewMap::OverviewMap( Renderer::Device *device, const char *filename, float x0, float y0, float extentX, float extentY ) 
: m_device( device ), m_x0( x0 ), m_y0( y0 ), m_extentX( extentX ), m_extentY( extentY ),
  m_overviewMapTexture( 0 ), m_posMarkerTexture( 0 )
{
	if( filename ) {
		uint width, height;
		if( Renderer::Texture2D::CreateFromFile( filename, m_overviewMapTexture, width, height ) ) {
			m_overviewMapTexture->GenerateMipMap();
			m_aspectRatio = float( width ) / float( height );
		} else {
			m_aspectRatio = 0.0f;
		}
	}

	uint width, height;
	if( Renderer::Texture2D::CreateFromMemory( ucPOS_MARKER_PNG, uiPOS_MARKER_PNG_SIZE, m_posMarkerTexture, width, height ) ) {
		m_posMarkerTexture->GenerateMipMap();
	}
}

OverviewMap::~OverviewMap() {
	delete m_posMarkerTexture;
	delete m_overviewMapTexture;
}

void OverviewMap::Render( const RenderState &state ) {
	if( !m_overviewMapTexture ) {
		return;
	}

	// ortho projection, origin upper left
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, 1, 1, 0, -1, 1 );
	// no modelview
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );

	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );

	// draw map
	glBindTexture( GL_TEXTURE_2D, m_overviewMapTexture->GetTextureID() );
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0, 0.0 ); glVertex2f( state.x, state.y );
		glTexCoord2f( 0.0, 1.0 ); glVertex2f( state.x, state.y + state.height );
		glTexCoord2f( 1.0, 1.0 ); glVertex2f( state.x + state.width, state.y + state.height );
		glTexCoord2f( 1.0, 0.0 ); glVertex2f( state.x + state.width, state.y );
	glEnd();

	// enable blending for pos marker
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBlendEquation( GL_FUNC_ADD );

	float windowAspect = state.width/state.height / m_aspectRatio;

	// compute pos marker position/size
	float posX = state.x + state.width  * ( state.posMarkerX - m_x0 ) / m_extentX;
	float posY = state.y + state.height * ( state.posMarkerY - m_y0 ) / m_extentY;
	float halfSize = 0.1f * state.height;

	// apply transform
	glTranslatef( posX, posY, 0.0f );
	glScalef( windowAspect, 1.0f, 1.0f );
	glRotatef( state.posMarkerHeading, 0.0f, 0.0f, 1.0f );

	// draw pos marker
	glBindTexture( GL_TEXTURE_2D, m_posMarkerTexture->GetTextureID() );
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0, 0.0 ); glVertex2f( -halfSize, -halfSize );
		glTexCoord2f( 0.0, 1.0 ); glVertex2f( -halfSize,  halfSize );
		glTexCoord2f( 1.0, 1.0 ); glVertex2f(  halfSize,  halfSize );
		glTexCoord2f( 1.0, 0.0 ); glVertex2f(  halfSize, -halfSize );
	glEnd();

	// reset state
	glDisable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
}

//void OverviewMap::GetPosition( const RenderState &state, float x, float y, float &mapX, float &mapY, bool &inOverviewMap ) const {
//	float width, height;
//	GetWidthAndHeightFromState( state, width, height );
//	
//	const float relativeX = (x - state.x) / width;
//	const float relativeY = (y - state.y) / height;
//
//	mapX = m_x0 + relativeX * m_extentX;
//	mapY = m_y0 + relativeY * m_extentY;
//
//	if( x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f ) {
//		inOverviewMap = false;
//	} else {
//		inOverviewMap = true;
//	}
//}