#include "channel.h"

#include <eq/client/client.h>
#include <eq/client/configEvent.h>
#include <eq/client/event.h>
#include <eq/client/frame.h>

#include "profiler/profiler.hpp"

#include "terrainLib/dataloader.h"
#include "terrainLib/terrain.h"

#include "camera.h"
#include "event.h"
#include "initData.h"
#include "overviewmap.h"


namespace eqTerrain3D
{
	bool Channel::configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID ) )
			return false;

		_dataLoaderViewIndex = getDataLoader()->GetNewViewIndex();

		return true;
	}

	bool Channel::configExit() {
		return eq::Channel::configExit();
	}

	void Channel::frameClear( const uint32_t frameID )
	{
		ScopedProfileSample sample( getPipe()->getProfiler(), "Channel::frameClear" );

		applyBuffer();
		applyViewport();

	#ifdef DEBUG
		const eq::Vector3ub color = getUniqueColor();
		EQ_GL_CALL( glClearColor( color.r()/255.0f, color.g()/255.0f, color.b()/255.0f, 1.0f ) );
	#else
		const float fClearColor[4] = { 57.0f / 255.0f, 113.0f / 255.0f, 165.0f / 255.0f, 1.0f };
		EQ_GL_CALL( glClearColor( fClearColor[0], fClearColor[1], fClearColor[2], fClearColor[3] ) );
	#endif

		EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

		glColorMask( true, true, true, true );
	}

	void Channel::_rasterizerStateSolid() const {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}

	void Channel::_rasterizerStateWireframe() const {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}

	void Channel::frameDraw( const uint32_t frameID )
	{
		ScopedProfileSample sample( getPipe()->getProfiler(), "Channel::frameDraw" );

		// need to call this every frame (instead of only on projection update)
		// because this sets the terrain's parameters, and the terrain might be
		// shared with other channels
		_updateProjection();

		Terrain *terrain = getTerrain();

		const FrameData& frameData = _getFrameData();

		const View *view = getView();
		EQASSERT( view );

		const Camera* camera = frameData.getCamera( view->getCameraIndex() );

		eq::Matrix4f viewMatrix;
		camera->GetViewMatrix( viewMatrix.array );

		const eq::Matrix4f &headTransform = getHeadTransform();
		viewMatrix = headTransform * viewMatrix;

		eq::Vector3f position;
		camera->GetPosition( position.array );

		eq::Vector4f viewDirection = -viewMatrix.get_row( 2 );

		// disable scissor test for terrain update (-> geometry decompressor!)
		glDisable( GL_SCISSOR_TEST );
		if( !frameData.getStaticMesh() ) {
			//printf( "Pos: %f %f %f Direction: %f %f %f\n", position.x(), position.y(), position.z(), viewDirection.x(), viewDirection.y(), viewDirection.z() );

			terrain->Update( viewMatrix.array, position.array, viewDirection.array, frameData.getOcclusionCulling() );

			_staticFrustumDirty = true;
		} else {
			if( _staticFrustumDirty ) {
				const eq::Frustumf& frustum = getFrustum();
				_updateStaticFrustum( frustum.compute_matrix() * viewMatrix );
				_staticFrustumDirty = false;
			}

			terrain->UpdateViewOnly( viewMatrix.array );
		}
		glEnable( GL_SCISSOR_TEST );

		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		glFrontFace( GL_CW );

		// set the target buffer to render into
		applyBuffer();
		applyViewport();

		if( frameData.getSatModify() ) {
			terrain->SetSaturation( frameData.getSaturation() );
		}

		if( frameData.getWireframe() ) {
			_rasterizerStateWireframe();
		} else {
			_rasterizerStateSolid();
		}

		if( frameData.getPageBoundingBoxes() ) {
			getDataLoader()->Render( terrain );
		}

		terrain->Render( frameData.getBoundingBoxes(), frameData.getSatModify() );

		Renderer::Effect::ResetState();

		if( frameData.getStaticMesh() ) {
			_drawViewFrustum( viewMatrix );
		}

		_rasterizerStateSolid();

		// reset color mask (for anaglyph stereo)
		glColorMask( true, true, true, true );

		glDisable( GL_CULL_FACE );

		// register ViewInfo at node, for dataloader update
		ViewInfo info;
		info.viewIndex = _dataLoaderViewIndex;
		info.position[ 0 ] = position.x();
		info.position[ 1 ] = position.y();
		info.position[ 2 ] = position.z();
		info.viewDirection[ 0 ] = viewDirection.x();
		info.viewDirection[ 1 ] = viewDirection.y();
		info.viewDirection[ 2 ] = viewDirection.z();
		getNode()->registerViewInfo( info );
	}

	void Channel::frameReadback( const uint32_t frameID )
	{
		ScopedProfileSample sample( getPipe()->getProfiler(), "Channel::frameReadback" );

		// Drop alpha channel from all frames during network transport
		const eq::FrameVector& frames = getOutputFrames();
		for( eq::FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i ) {
			( *i )->setAlphaUsage( false );
		}

		eq::Channel::frameReadback( frameID );
	}

	void Channel::frameViewFinish( const uint32_t frameID )
	{
		ScopedProfileSample sample( getPipe()->getProfiler(), "Channel::frameViewFinish" );

		applyBuffer();
		applyViewport();

		const FrameData& frameData = _getFrameData();

		if( frameData.getRenderStatistics() )
			drawStatistics();

		if( frameData.getRenderHelp() )
			_drawHelp();

		if( !frameData.getMessage().empty() )
			_drawMessage( frameData.getMessage() );

		if( frameData.getRenderOverviewMap() )
			_drawOverviewMap();

		if( frameData.getRenderLogo() )
			_drawLogo();
	}

	void Channel::_drawViewFrustum( const eq::Matrix4f& modelView )
	{
		// be lazy and use fixed function here...
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		applyFrustum();
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( modelView.array );

		glBegin( GL_LINES );
			// near plane
			glVertex3fv( _staticFrustum[ 0 ].array );
			glVertex3fv( _staticFrustum[ 1 ].array );
			glVertex3fv( _staticFrustum[ 1 ].array );
			glVertex3fv( _staticFrustum[ 3 ].array );
			glVertex3fv( _staticFrustum[ 3 ].array );
			glVertex3fv( _staticFrustum[ 2 ].array );
			glVertex3fv( _staticFrustum[ 2 ].array );
			glVertex3fv( _staticFrustum[ 0 ].array );
			// far plane
			glVertex3fv( _staticFrustum[ 4 ].array );
			glVertex3fv( _staticFrustum[ 5 ].array );
			glVertex3fv( _staticFrustum[ 5 ].array );
			glVertex3fv( _staticFrustum[ 7 ].array );
			glVertex3fv( _staticFrustum[ 7 ].array );
			glVertex3fv( _staticFrustum[ 6 ].array );
			glVertex3fv( _staticFrustum[ 6 ].array );
			glVertex3fv( _staticFrustum[ 4 ].array );
			// connections
			glVertex3fv( _staticFrustum[ 0 ].array );
			glVertex3fv( _staticFrustum[ 4 ].array );
			glVertex3fv( _staticFrustum[ 1 ].array );
			glVertex3fv( _staticFrustum[ 5 ].array );
			glVertex3fv( _staticFrustum[ 2 ].array );
			glVertex3fv( _staticFrustum[ 6 ].array );
			glVertex3fv( _staticFrustum[ 3 ].array );
			glVertex3fv( _staticFrustum[ 7 ].array );
		glEnd();
	}

	void Channel::_drawHelp()
	{
		const eq::Window::Font* font = getWindow()->getSmallFont();
		if( !font )
			return;

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		applyScreenFrustum();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glDisable( GL_DEPTH_TEST );

		eq::Frustumf frustum = getScreenFrustum();
		float x = float( frustum.left() ) + 10.0f;
		float y = float( frustum.top() ) - 20.0f;

		glColor3f( 0.0f, 1.0f, 0.0f );

		glRasterPos3f( x, y, 0.99f ); y -= 24.0f;
		font->draw( "Controls:" );

		glColor3f( 1.0f, 1.0f, 0.0f );

		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "Left Mouse Button: View Direction" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "Right Mouse Button: Picking" );

		y -= 8.0f;
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "Arrow Keys: Move Forward / Backward / Left / Right" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "Page Up / Down: Move Up / Down" );

		y -= 8.0f;
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "N: Set Heading to North" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "2: View From Above, Set Heading to North" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "X: Reset Camera Position" );

		y -= 8.0f;
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "W: Toggle Wireframe" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "P: Toggle Page Bounding Boxes" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "B: Toggle Tile Bounding Boxes" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "S: Toggle Static Mesh" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "Q: Toggle Statistics" );
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "O: Toggle Overview Map" );

		y -= 8.0f;
		glRasterPos3f( x, y, 0.99f ); y -= 16.0f;
		font->draw( "H / F1: Toggle Help" );

		glColor3f( 1.0f, 1.0f, 1.0f );

		glEnable( GL_DEPTH_TEST );
	}

	void Channel::_drawMessage( const std::string& message )
	{
		const eq::Window::Font* font = getWindow()->getMediumFont();
		if( !font )
			return;

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		applyScreenFrustum();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glDisable( GL_DEPTH_TEST );

		eq::Frustumf frustum = getScreenFrustum();
		float x = float( frustum.left() ) + 10.0f;
		float y = float( frustum.top() - 0.5f * frustum.get_height() ) - 10.0f;

		size_t pos0 = 0;
		for( size_t pos1 = message.find( '\n' ); pos1 != std::string::npos; pos1 = message.find( '\n', pos0 ) ) {
			glRasterPos3f( x, y, 0.99f ); y -= 22.0f;
			font->draw( message.substr( pos0, pos1 - pos0 ) );
			pos0 = pos1 + 1;
		}
		glRasterPos3f( x, y, 0.99f );
		font->draw( message.substr( pos0 ) );

		glEnable( GL_DEPTH_TEST );
	}

	void Channel::_drawOverviewMap()
	{
		OverviewMap* overviewMap = getPipe()->getOverviewMap();
		if( !overviewMap )
			return;

		const View* view = getView();
		EQASSERT( view );
		const Camera* camera = _getFrameData().getCamera( view->getCameraIndex() );
		const eq::PixelViewport& pvp = getPixelViewport();

		float channelAspect = float( pvp.w ) / float( pvp.h );
		float mapAspect = overviewMap->GetAspectRatio();
		float height = 0.25f;
		float width = height * mapAspect / channelAspect;
		eq::Vector3f position;
		camera->GetPosition( position.array );
		eq::Matrix4f viewMatrix;
		camera->GetViewMatrix( viewMatrix.array );
		// don't include head matrix here!
		eq::Vector4f viewDirection = -viewMatrix.get_row( 2 );
		float heading = RAD2DEG( atan2( viewDirection.y(), viewDirection.x() ) );

		OverviewMap::RenderState state;
		state.x = 0.0f;
		state.y = 1.0f - height;
		state.width = width;
		state.height = height;
		state.posMarkerX = position.x();
		state.posMarkerY = position.y();
		state.posMarkerHeading = heading;

		overviewMap->Render( state );
	}

	void Channel::_drawLogo()
	{
		Renderer::FramebufferTexture2D* texture = getPipe()->getLogoTexture();

		const eq::PixelViewport& pvp = getPixelViewport();

		// be lazy and use fixed function here...
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glOrtho( 0.0, pvp.w, 0.0, pvp.h, -1.0, 1.0 );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glDisable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glActiveTexture( GL_TEXTURE0 );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture->GetTextureID() );

		float posX = float( pvp.w - texture->GetWidth() - 50 );
		float posY = 0.0f;
		glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2f( 0.0, 1.0 ); glVertex2f( posX, posY );
			glTexCoord2f( 1.0, 1.0 ); glVertex2f( posX + texture->GetWidth(), posY );
			glTexCoord2f( 0.0, 0.0 ); glVertex2f( posX, posY + texture->GetHeight() );
			glTexCoord2f( 1.0, 0.0 ); glVertex2f( posX + texture->GetWidth(), posY + texture->GetHeight() );
		glEnd();

		glDisable( GL_TEXTURE_2D );

		glDisable( GL_BLEND );
		glEnable( GL_DEPTH_TEST );
	}

	const FrameData& Channel::_getFrameData() const
	{
		return getPipe()->getFrameData();
	}

	void Channel::_updateProjection()
	{
		const SystemConfiguration &systemConfiguration = getConfig()->getInitData().getSystemConfiguration();

		setNearFar( systemConfiguration.GetZNear(), systemConfiguration.GetZFar() );

		const eq::Frustumf& frustum = getFrustum();
		eq::Matrix4f projectionMatrix = frustum.compute_matrix();

		const eq::PixelViewport& pvp = getPixelViewport();

		// terrain and dataloader expect centered (non-offcenter) frustum,
		// so compute params of "padded" centered frustum
		float leftright = std::max( fabsf( frustum.left() ), fabsf( frustum.right() ) );
		float bottomtop = std::max( fabsf( frustum.bottom() ), fabsf( frustum.top() ) );

		float fovy = 2.0f * RAD2DEG( atanf( bottomtop / frustum.near_plane() ) );
		float aspectRatio = leftright / bottomtop;

		// compute viewport for padded frustum
		eq::PixelViewport adaptedPVP;
		adaptedPVP.w = uint( float( pvp.w ) * 2.0f * leftright / ( frustum.right() - frustum.left()   ) );
		adaptedPVP.h = uint( float( pvp.h ) * 2.0f * bottomtop / ( frustum.top()   - frustum.bottom() ) );
		adaptedPVP.x = pvp.x - uint( float( adaptedPVP.w ) * 0.5f * ( 1.0f - frustum.left()   / -leftright ) );
		adaptedPVP.y = pvp.y - uint( float( adaptedPVP.h ) * 0.5f * ( 1.0f - frustum.bottom() / -bottomtop ) );

		float screenSpaceError = systemConfiguration.GetScreenSpaceError();
		float prefetchingFactor = systemConfiguration.GetPrefetchingFactor();
		getTerrain()->SetParameters( projectionMatrix.array, adaptedPVP.h, fovy, aspectRatio, frustum.near_plane(), screenSpaceError );
		getDataLoader()->ComputeLoadingRadiuses( adaptedPVP.h, fovy, aspectRatio, frustum.far_plane(), screenSpaceError, prefetchingFactor, _dataLoaderViewIndex );

		// update the camera only if this is the active channel
		const FrameData& frameData = _getFrameData();
		uint32_t activeChannelID = frameData.getActiveChannelID();
		const View* view = getView();
		if( activeChannelID == getID() && view ) {
			// mouse event coords are relative to the window, and use upper left origin
			const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();
			int offsetX = -adaptedPVP.x;
			int offsetY = ( adaptedPVP.y + adaptedPVP.h ) - windowPVP.h;

			// send event to config
			eq::ConfigEvent configEvent;
			configEvent.data.type = CUSTOMEVENT_CAMERAPARAMS;

			CameraParamsEvent camEvent;
			camEvent.setCameraIndex( view->getCameraIndex() );
			camEvent.setViewportWidth( adaptedPVP.w );
			camEvent.setViewportHeight( adaptedPVP.h );
			camEvent.setFovy( fovy );
			camEvent.setOffsetX( offsetX );
			camEvent.setOffsetY( offsetY );
			camEvent.setHeadTransform( getHeadTransform() );

			camEvent.fillEqEvent( configEvent.data.user );

			getConfig()->sendEvent( configEvent );
		}
	}

	void Channel::_updateStaticFrustum( const eq::Matrix4f& modelViewProj )
	{
		// view frustum in normalized device space = unit cube
		_staticFrustum[ 0 ].set( -1.0f, -1.0f, -1.0f );
		_staticFrustum[ 1 ].set(  1.0f, -1.0f, -1.0f );
		_staticFrustum[ 2 ].set( -1.0f,  1.0f, -1.0f );
		_staticFrustum[ 3 ].set(  1.0f,  1.0f, -1.0f );
		_staticFrustum[ 4 ].set( -1.0f, -1.0f,  1.0f );
		_staticFrustum[ 5 ].set(  1.0f, -1.0f,  1.0f );
		_staticFrustum[ 6 ].set( -1.0f,  1.0f,  1.0f );
		_staticFrustum[ 7 ].set(  1.0f,  1.0f,  1.0f );

		// transform into world space
		eq::Matrix4f mvpInv;
		modelViewProj.inverse( mvpInv );
		for( uint i = 0; i < 8; i++ ) {
			_staticFrustum[ i ] = mvpInv * eq::Vector4f( _staticFrustum[ i ], 1.0f );
		}
	}
}
