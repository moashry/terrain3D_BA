#include "pipe.h"

#include <gl/glew.h>

#include "terrainLib/terrain.h"

#include "overviewmap.h"
#include "tumlogo.h"


namespace eqTerrain3D
{
	Pipe::Pipe( eq::Node* parent )
		: eq::Pipe( parent ), _glewContext( 0 ), _device( 0 ), _terrain( 0 ), _overviewMap( 0 ), _logoTexture( 0 ),
		  _profilerOutputFrequency( 100 ), _profiledFrames( 0 )
	{
		_profiler.addOutputHandler( &_profilerLogWriter );
	}

	void Pipe::glewSetContext( GLEWContext *context )
	{
		if( context == 0 && _device ) {
			exitGL();
		}
		_glewContext = context;
	}

	std::string Pipe::getProfileLogName() const
	{
		std::stringstream profileLog;
		profileLog << getNode()->getProfileLogName();
		profileLog << "_pipe_";
		if( !getName().empty() ) {
			profileLog << getName();
		} else {
			profileLog << getID();
		}

		return profileLog.str();
	}

	eq::WindowSystem Pipe::selectWindowSystem() const
	{
		const Config*          config   = getConfig();
		const InitData&        initData = config->getInitData();
		const eq::WindowSystem ws       = initData.getWindowSystem();

		if( ws == eq::WINDOW_SYSTEM_NONE )
			return eq::Pipe::selectWindowSystem();
		if( !supportsWindowSystem( ws ) ) {
			EQWARN << "Window system " << ws << " not supported, using default window system" << std::endl;
			return eq::Pipe::selectWindowSystem();
		}

		return ws;
	}

	bool Pipe::configInit( const uint32_t initID )
	{
		if( !eq::Pipe::configInit( initID ) )
			return false;

		// get framedata from config
		Config*         config      = getConfig();
		const InitData& initData    = config->getInitData();
		const uint32_t  frameDataID = initData.getFrameDataID();
		const bool mapped = config->mapObject( &_frameData, frameDataID );
		EQASSERT( mapped );
		if( !mapped ) {
			return false;
		}

		// init profiler
		std::string profileLogFilename = getProfileLogName() + ".log";
		_profilerLogWriter.open( profileLogFilename );
		if( !_profilerLogWriter.isGood() ) {
			EQWARN << "Pipe::configInit: failed to open profile log file \"" << profileLogFilename << "\"" << std::endl;
		}

		return true;
	}

	bool Pipe::configExit()
	{
		if( _profiler.isRunning() ) {
			_profiler.stop();
		}
		_profilerLogWriter.close();

		Config* config = getConfig();
		config->unmapObject( &_frameData );

		return eq::Pipe::configExit();
	}

	void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		_frameData.sync( frameID );

		// start profiler if necessary
		if( _frameData.getProfilerEnabled() && !_profiler.isRunning() ) {
			_profiler.start();
		}

		ScopedProfileSample( &_profiler, "Pipe::frameStart" );

		eq::Pipe::frameStart( frameID, frameNumber );

		// init GL if not done yet
		// can't do this earlier, because we have to wait until all windows are initialized
		// and their GL contexts properly shared
		if( !_device ) {
			initGL();
		}
	}

	void Pipe::frameFinish( const uint32_t frameID, const uint32_t frameNumber )
	{
		{ ScopedProfileSample sample( &_profiler, "Pipe::frameFinish" );
			eq::Pipe::frameFinish( frameID, frameNumber );
		}

		// update profiler
		if( _profiler.isRunning() ) {
			_profiler.endFrame();

			if( ++_profiledFrames >= _profilerOutputFrequency ) {
				_profiler.output();
				_profiledFrames = 0;
			}
		}

		// stop profiler if necessary
		if( !_frameData.getProfilerEnabled() && _profiler.isRunning() ) {
			_profiler.stop();
			_profiledFrames = 0;
		}
	}

	void Pipe::initGL()
	{
		assert( !_device );

		const Config* config = getConfig();
		const SystemConfiguration&  systemConfiguration  = config->getInitData().getSystemConfiguration();
		const TerrainConfiguration& terrainConfiguration = config->getInitData().getTerrainConfiguration();

		_device = new Renderer::Device();
		_terrain = new Terrain( getDataLoader(), systemConfiguration.GetInitNumVBs(), systemConfiguration.GetInitNumTextures(),
			terrainConfiguration.GetLevelZeroTileExtent(), terrainConfiguration.GetNumLevels(),
			terrainConfiguration.GetMinVerticalTolerance(), terrainConfiguration.GetTextureCompression() );

		_device->Create();
		_terrain->Create( _device );

		_terrain->SetProfiler( &_profiler );

		const std::string& overviewMapFilename = terrainConfiguration.GetOverviewMapFileName();
		if( !overviewMapFilename.empty() ) {
			float x0, y0, extentX, extentY;
			terrainConfiguration.GetOverviewMapExtents( &x0, &y0, &extentX, &extentY );
			_overviewMap = new OverviewMap( _device, overviewMapFilename.c_str(), x0, y0, extentX, extentY );
		}

		Renderer::Texture2D::Create( 256, 128, 1, GL_RGBA8, _logoTexture, ucTUMLogo );
	}

	void Pipe::exitGL()
	{
		assert( _device );

		delete _logoTexture;
		_logoTexture = 0;
		delete _overviewMap;
		_overviewMap = 0;
		_terrain->SafeRelease();
		_device->SafeRelease();
		delete _terrain;
		_terrain = 0;
		delete _device;
		_device = 0;
	}
}
