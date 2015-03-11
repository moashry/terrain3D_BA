#include "window.h"

#include <eq/client/configEvent.h>
#include <eq/client/task.h>

#include "config.h"
#include "channel.h"
#include "event.h"
#include "frameData.h"
#include "pipe.h"


namespace eqTerrain3D
{
	bool Window::configInitGL( const uint32_t initID )
	{
		// make sure required GL features are supported
		if( !GLEW_VERSION_3_2 ) {
			setErrorMessage( "Terrain3D requires OpenGL 3.2" );
			return false;
		}
		if( !GLEW_EXT_texture_compression_s3tc ) {
			setErrorMessage( "Terrain3D requires GL_EXT_texture_compression_s3tc" );
			return false;
		}
		if( !GLEW_EXT_texture_filter_anisotropic ) {
			setErrorMessage( "Terrain3D requires GL_EXT_texture_filter_anisotropic" );
			return false;
		}

		// don't call eq::Window::configInitGL, it just sets some unnecessary GL state
		// instead, set required state ourselves
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LESS );
		glEnable( GL_SCISSOR_TEST ); // needed by Equalizer to constrain channel viewport
		glDisable( GL_DITHER );


		getPipe()->glewSetContext( glewGetContext() );

		return true;
	}

	bool Window::configExitGL()
	{
		getPipe()->glewSetContext( 0 );

		return eq::Window::configExitGL();
	}

	bool Window::processEvent( const eq::Event& event )
	{
		switch( event.type ) {
			case eq::Event::POINTER_MOTION:
			case eq::Event::POINTER_BUTTON_PRESS:
			case eq::Event::POINTER_BUTTON_RELEASE:
			{
				// find the clicked channel
				const eq::ChannelVector& channels = getChannels();
				eq::Channel* activeChannel = 0;
				for( uint i = 0; i < channels.size(); i++ ) {
					eq::Channel* channel = channels[ i ];

					// skip channels that don't draw or assemble (eq clear-only helper channel)
					if( !( channel->getTasks() & ( eq::TASK_DRAW | eq::TASK_ASSEMBLE ) ) ) {
						continue;
					}

					// event mouse coords have origin upper left, pvp has origin lower left
					if( channel->getNativePixelViewPort().isPointInside( event.pointer.x, getPixelViewport().h - event.pointer.y ) ) {
						activeChannel = channel;
						break;
					}
				}

				// if we found a channel, send event to config
				if( activeChannel ) {
					eq::ConfigEvent configEvent;
					configEvent.data.type = CUSTOMEVENT_UINT32_ACTIVECHANNEL;

					Uint32Event myEvent;
					myEvent.setValue( activeChannel->getID() );
					myEvent.fillEqEvent( configEvent.data.user );

					getConfig()->sendEvent( configEvent );
				}

				break;
			}
		}

		return eq::Window::processEvent( event );
	}
}
