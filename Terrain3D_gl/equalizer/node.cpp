#include "node.h"

#include <sstream>

#include <eq/client/global.h>

#include "terrainLib/terrainlib.h"


namespace eqTerrain3D
{
	Node::Node( eq::Config* parent )
		: eq::Node( parent ), _dataLoader( 0 ),
		  _profilerOutputFrequency( 100 ), _profiledFrames( 0 )
	{
		_profiler.addOutputHandler( &_profilerLogWriter );
	}

	void Node::registerViewInfo( const ViewInfo& info )
	{
		_viewInfos.push_back( info );
	}

	std::string Node::getProfileLogName() const
	{
		std::stringstream profileLog;
		profileLog << "profile_node_";
		if( !getName().empty() ) {
			profileLog << getName();
		} else {
			profileLog << getID();
		}

		return profileLog.str();
	}

	bool Node::configInit( const uint32_t initID )
	{
		if( !eq::Node::configInit( initID ) )
			return false;

		switch( getIAttribute( IATTR_THREAD_MODEL ) ) {
			case eq::UNDEFINED:
				// default to draw sync
				setIAttribute( IATTR_THREAD_MODEL, eq::DRAW_SYNC );
				break;

			case eq::ASYNC:
				// async execution not supported
				// allow anyway when specified explicitly, but warn
				EQWARN << "Node::configInit: ASYNC execution is not supported, strange things may happen..." << std::endl;
				break;

			// everything else is ok
		}

		// tell config to map its data (if non-appnode)
		Config* config = getConfig();
		config->mapData( initID );

		// get framedata from config
		const InitData& initData    = config->getInitData();
		const uint32_t  frameDataID = initData.getFrameDataID();

		const bool mapped = config->mapObject( &_frameData, frameDataID );
		EQASSERT( mapped );
		if( !mapped ) {
			return false;
		}

		// init dataloader
		const SystemConfiguration&  systemConfiguration  = config->getInitData().getSystemConfiguration();
		const TerrainConfiguration& terrainConfiguration = config->getInitData().getTerrainConfiguration();
		
		_dataLoader = CreateDataLoader( terrainConfiguration, systemConfiguration.GetSystemMemoryUsageLimit(), systemConfiguration.GetScreenSpaceError() );

		// init profiler
		std::string profileLogFilename = getProfileLogName() + ".log";
		_profilerLogWriter.open( profileLogFilename );
		if( !_profilerLogWriter.isGood() ) {
			EQWARN << "Node::configInit: failed to open profile log file \"" << profileLogFilename << "\"" << std::endl;
		}

		_dataLoader->SetProfiler( &_profiler );

		return true;
	}

	bool Node::configExit()
	{
		if( _profiler.isRunning() ) {
			_profiler.stop();
		}
		_profilerLogWriter.close();

		delete _dataLoader;
		_dataLoader = 0;

		Config* config = getConfig();
		config->unmapObject( &_frameData );

		config->unmapData();

		return eq::Node::configExit();
	}

	void Node::frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		_frameData.sync( frameID );

		// start profiler if necessary
		if( _frameData.getProfilerEnabled() && !_profiler.isRunning() ) {
			_profiler.start();
		}

		ScopedProfileSample sample( &_profiler, "Node::frameStart" );

		eq::Node::frameStart( frameID, frameNumber );
	}

	void Node::frameDrawFinish( const uint32_t frameID, const uint32_t frameNumber )
	{
		ScopedProfileSample sample( &_profiler, "Node::frameDrawFinish" );

		// wait for pipes to finish
		eq::Node::frameDrawFinish( frameID, frameNumber );

		// update data loader
		_dataLoader->Update( _viewInfos );
		_viewInfos.clear();
	}

	void Node::frameFinish( const uint32_t frameID, const uint32_t frameNumber )
	{
		{ ScopedProfileSample sample( &_profiler, "Node::frameFinish" );
			eq::Node::frameFinish( frameID, frameNumber );
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
}
