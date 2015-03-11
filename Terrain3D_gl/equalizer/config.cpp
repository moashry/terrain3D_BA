#include "config.h"

#include <eq/client/event.h>

#include "global.h"
#include "event.h"
#include "node.h"


namespace eqTerrain3D
{
	Config::Config( eq::base::RefPtr< eq::Server > parent )
		: eq::Config( parent ), _camerasInited( false ), _lastFrameTime( 0 ), _messageTime( 0 )
	{
	}

	bool Config::init()
	{
		const TerrainConfiguration& terrainConfiguration = _initData.getTerrainConfiguration();

		_frameData.setNumCameras( terrainConfiguration.GetNumStartLocations() );

		for( uint i = 0; i < terrainConfiguration.GetNumStartLocations(); i++ ) {
			const StartLocation& startLocation = terrainConfiguration.GetStartLocation( i );
			Camera* camera = _frameData.getCamera( i );
			camera->Init( terrainConfiguration.GetMaxSpeed(), terrainConfiguration.GetMinHeight(), startLocation.vStartPosition, startLocation.fStartHeading, startLocation.fStartPitch );
			Recorder* recorder = new Recorder( camera );
			_viewers.push_back( Viewer( camera, recorder ) );
		}
		_camerasInited = false;

		registerObject( &_frameData );
		_frameData.setAutoObsolete( getLatency() );

		_initData.setFrameDataID( _frameData.getID() );
		registerObject( &_initData );

		if( !eq::Config::init( _initData.getID() ) ) {
			for( uint i = 0; i < _viewers.size(); i++ ) {
				// don't delete camera (owned by framedata)
				delete _viewers[ i ].recorder;
			}
			_viewers.clear();
			_deregisterData();
			return false;
		}

		_lastFrameTime = getTime();

		return true;
	}

	bool Config::exit()
	{
		const bool ret = eq::Config::exit();

		for( uint i = 0; i < _viewers.size(); i++ ) {
			// don't delete camera (owned by framedata)
			delete _viewers[ i ].recorder;
		}
		_viewers.clear();

		_deregisterData();

		return ret;
	}

	void Config::_deregisterData()
	{
		deregisterObject( &_initData );
		deregisterObject( &_frameData );

		_initData.setFrameDataID( EQ_ID_INVALID );
	}


	void Config::mapData( const uint32_t initDataID )
	{
		if( _initData.getID() == EQ_ID_INVALID ) {
			EQCHECK( mapObject( &_initData, initDataID ) );
			// data was retrieved, unmap immediately
			unmapObject( &_initData );
		} else {
			// appNode, _initData is registered already
			EQASSERT( _initData.getID() == initDataID );
		}
	}

	void Config::unmapData()
	{
	}

	uint32_t Config::startFrame()
	{
		int64_t currentFrameTime = getTime();
		float fElapsedTime = ( currentFrameTime - _lastFrameTime ) * 0.001f;
		_lastFrameTime = currentFrameTime;

		// reset message after two seconds
		if( _messageTime != 0 && _lastFrameTime - _messageTime > 2000 ) {
			_messageTime = 0;
			_frameData.setMessage( "" );
		}

		if( !_camerasInited ) {
			eq::NodeVector nodes = getNodes();
			assert( nodes.size() == 1 );
			if( !nodes.empty() ) {
				Node* node = static_cast< Node* >( nodes[ 0 ] );
				DataLoader* dataLoader = node->getDataLoader();

				for( int viewIndex = 0 ; viewIndex < _viewers.size() ; viewIndex++ ) {
					Viewer &viewer = _viewers[ viewIndex ];
					viewer.camera->SetDataLoader( dataLoader );
				}
			}
			_camerasInited = true;
		}

		for( int viewIndex = 0 ; viewIndex < _viewers.size() ; viewIndex++ ) {
			Viewer &viewer = _viewers[ viewIndex ];

			// for now, always mark camera dirty
			_frameData.markCameraDirty( viewIndex );

			RECORDER_STATE eRecState = viewer.recorder->GetState();
			if( eRecState != REC_STATE_PLAYING_BACK ) {
				viewer.camera->Update( fElapsedTime );
			}

			if( eRecState == REC_STATE_RECORDING || eRecState == REC_STATE_PLAYING_BACK ) {
				viewer.recorder->Update( fElapsedTime );
			}
		}
		//eq::Observer *observer = getObservers()[0];
		//observer->setHeadMatrix( viewMatrix );

		const uint32_t version = _frameData.commit();

		return eq::Config::startFrame( version );
	}


	bool Config::needsRedraw()
	{
		return true;
	}

	namespace {
		CAM_KEY eqKeyToCamKey( uint32_t key ) {
			switch( key ) {
		#define KEYMAP( eqKey, camKey ) case eq::KC_##eqKey: return CAM_KEY_##camKey
		#define KEYMAP_SIMPLE( key ) KEYMAP( key, key )
				KEYMAP_SIMPLE( UP );
				KEYMAP_SIMPLE( DOWN );
				KEYMAP_SIMPLE( LEFT );
				KEYMAP_SIMPLE( RIGHT );
				KEYMAP( PAGE_UP, PAGEUP );
				KEYMAP( PAGE_DOWN, PAGEDOWN );
				KEYMAP( SHIFT_L, SHIFT );
				KEYMAP( SHIFT_R, SHIFT );
				KEYMAP_SIMPLE( HOME );
				KEYMAP_SIMPLE( END );
		#undef KEYMAP_SIMPLE
		#undef KEYMAP
			default:
				return CAM_KEY_UNKNOWN;
			}
		}

		MOUSE_BUTTON eqMouseButtonToCamMouseButton( uint32_t button ) {
			switch( button ) {
				case eq::PTR_BUTTON1:
					return MOUSE_BUTTON_LEFT;

				case eq::PTR_BUTTON2:
					return MOUSE_BUTTON_MIDDLE;

				case eq::PTR_BUTTON3:
					return MOUSE_BUTTON_RIGHT;

				default:
					return MOUSE_BUTTON_NONE;
			}
		}
	}

	bool Config::_frameDataKeyEvent( uint32_t key ) {
		switch( toupper( key ) ) {
			case 'W':
			{
				_frameData.toggleWireframe();
				_setMessage( "Wireframe", _frameData.getWireframe() );
				return true;
			}
			case 'P':
			{
				_frameData.togglePageBoundingBoxes();
				_setMessage( "Page Bounding Boxes", _frameData.getPageBoundingBoxes() );
				return true;
			}
			case 'B':
			{
				_frameData.toggleBoundingBoxes();
				_setMessage( "Tile Bounding Boxes", _frameData.getBoundingBoxes() );
				return true;
			}
			case 'S':
			{
				_frameData.toggleStaticMesh();
				_setMessage( "Static Mesh", _frameData.getStaticMesh() );
				return true;
			}
			case eq::KC_F1:
			case 'H':
			{
				_frameData.toggleRenderHelp();
				return true;
			}
			case 'Q':
			{
				_frameData.toggleRenderStatistics();
				return true;
			}
			case 'O':
			{
				_frameData.toggleRenderOverviewMap();
				return true;
			}
			case 'L':
			{
				_frameData.toggleRenderLogo();
				return true;
			}
			case 'U':
			{
				_frameData.toggleProfilerEnabled();
				_setMessage( "Profiler", _frameData.getProfilerEnabled() );
				return true;
			}
		}

		return false;
	}

	bool Config::handleEvent( const eq::ConfigEvent* event )
	{
		bool redraw = false;

		uint32_t viewID = event->data.context.view.id;

		switch( event->data.type )
		{
			case eq::Event::KEY_PRESS:
			{
				const eq::KeyEvent& eventData = event->data.keyPress;
				if( _frameDataKeyEvent( eventData.key ) ) {
					redraw = true;
					return true;
				}
				// apply key events only to active view
				viewID = _frameData.getActiveViewID();
				break;
			}
			case eq::Event::KEY_RELEASE:
			{
				// apply key events only to active view
				viewID = _frameData.getActiveViewID();
				break;
			}

			case eq::Event::POINTER_BUTTON_PRESS:
			{
				// update active view on mouse event
				_frameData.setActiveViewID( viewID );

				const eq::PointerEvent& eventData = event->data.pointerButtonPress;
				if( eventData.button == eq::PTR_BUTTON2 ) {
					// middle mouse button switches to next viewer (camera)
					View* view = findView( viewID );
					if( view ) {
						view->setCameraIndex( ( view->getCameraIndex() + 1 ) % _viewers.size() );
					}
					redraw = true;
					return true;
				}
				break;
			}
			case eq::Event::POINTER_MOTION:
			{
				// update active view on mouse event
				_frameData.setActiveViewID( viewID );
				break;
			}

			case eq::Event::WINDOW_EXPOSE:
			case eq::Event::WINDOW_RESIZE:
			case eq::Event::WINDOW_CLOSE:
			case eq::Event::VIEW_RESIZE:
			{
				redraw = true;
				break;
			}

			case CUSTOMEVENT_UINT32_ACTIVECHANNEL:
			{
				Uint32Event myEvent( event->data.user );
				_frameData.setActiveChannelID( myEvent.getValue() );
				return true;
			}
			case CUSTOMEVENT_CAMERAPARAMS:
			{
				CameraParamsEvent myEvent( event->data.user );
				Camera* camera = _frameData.getCamera( myEvent.getCameraIndex() );
				camera->SetParameters( myEvent.getViewportWidth(), myEvent.getViewportHeight(), myEvent.getFovy(), myEvent.getOffsetX(), myEvent.getOffsetY(), myEvent.getHeadTransform().array );
				return true;
			}
		}

		// forward to viewer
		Viewer* viewer = _getViewer( viewID );
		if( viewer ) {
			redraw |= viewer->handleEvent( event );	
		}

		redraw |= eq::Config::handleEvent( event );
		return redraw;
	}

	Config::Viewer* Config::_getViewer( uint32_t viewID ) {
		const View *view = findView( viewID );
		if( !view ) {
			return 0;
		}

		return &_viewers[ view->getCameraIndex() ];
	}

	void Config::_setMessage( const std::string& message ) {
		_messageTime = getTime();
		_frameData.setMessage( message );
	}

	void Config::_setMessage( const std::string& message, bool enabled ) {
		_setMessage( message + " " + _getEnabledDisabled( enabled ) );
	}

	std::string Config::_getEnabledDisabled( bool enabled )
	{
		if( enabled )
			return "enabled";
		else
			return "disabled";
	}

	bool Config::Viewer::keyEvent( uint32_t key ) {
		switch( toupper( key ) ) {
			case 'X':
			{
				camera->Reset();
				return true;
			}
			case 'N':
			{
				float fHeading = 270.0f;
				float fPitch, fRoll;
				camera->GetAngles(0, &fPitch, &fRoll);
				camera->SetAngles(fHeading, fPitch, fRoll);
				return true;
			}
			case '2':
			{
				float fHeading = 270.0f;
				float fPitch = 90.0f;
				float fRoll = 0.0f;
				camera->SetAngles(fHeading, fPitch, fRoll);
				return true;
			}
		}
		return false;
	}

	bool Config::Viewer::handleEvent( const eq::ConfigEvent* event ) {
		switch( event->data.type )
		{
			case eq::Event::KEY_PRESS:
			{
				const eq::KeyEvent& eventData = event->data.keyPress;
				camera->KeyboardEvent( eqKeyToCamKey( eventData.key ), true );
				recorder->KeyboardEvent( ( uchar )eventData.key );
				keyEvent( eventData.key );

				return true;
			}
			case eq::Event::KEY_RELEASE:
			{
				const eq::KeyEvent& eventData = event->data.keyRelease;
				camera->KeyboardEvent( eqKeyToCamKey( eventData.key ), false );

				return true;
			}
			case eq::Event::POINTER_BUTTON_PRESS:
			{
				const eq::PointerEvent& eventData = event->data.pointerButtonPress;
				camera->MouseButtonEvent( eqMouseButtonToCamMouseButton( eventData.button ), true, eventData.x, eventData.y );

				return true;
			}
			case eq::Event::POINTER_BUTTON_RELEASE:
			{	
				const eq::PointerEvent& eventData = event->data.pointerButtonRelease;
				camera->MouseButtonEvent( eqMouseButtonToCamMouseButton( eventData.button ), false, eventData.x, eventData.y );

				return true;
			}
			case eq::Event::POINTER_MOTION:
			{
				const eq::PointerEvent &eventData = event->data.pointerMotion;
				camera->MouseMoveEvent( eventData.x, eventData.y );

				return true;
			}
		}
		return false;
	}
}
