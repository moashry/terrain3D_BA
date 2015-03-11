#ifndef EQ_TERRAIN3D_NODE_H
#define EQ_TERRAIN3D_NODE_H

#include <eq/client/config.h>
#include <eq/client/node.h>

#include "profiler/profiler.hpp"
#include "profiler/profilerlogwriter.hpp"

#include "config.h"
#include "frameData.h"

class DataLoader;


namespace eqTerrain3D
{
	class Node : public eq::Node
	{
	public:
		Node( eq::Config* parent );

		// redeclare some get* functions to work with the specialized classes
		Config*       getConfig()       { return static_cast< Config* >      ( eq::Node::getConfig() ); }
		const Config* getConfig() const { return static_cast< const Config* >( eq::Node::getConfig() ); }

		DataLoader* getDataLoader() { return _dataLoader; }

		void registerViewInfo( const ViewInfo& info );

		std::string getProfileLogName() const; // without extension

		// not threadsafe - use only from node thread!
		Profiler* getProfiler() { return &_profiler; }

	private:
		bool configInit( const uint32_t initID );
		bool configExit();
		void frameStart( const uint32_t frameID, const uint32_t frameNumber );
		void frameDrawFinish( const uint32_t frameID, const uint32_t frameNumber );
		void frameFinish( const uint32_t frameID, const uint32_t frameNumber );

		FrameData         _frameData;

		DataLoader*       _dataLoader;
		ViewInfoVector    _viewInfos;

		Profiler          _profiler;
		ProfilerLogWriter _profilerLogWriter;
		uint              _profilerOutputFrequency;
		uint              _profiledFrames;
	};
}

#endif // EQ_TERRAIN3D_NODE_H
