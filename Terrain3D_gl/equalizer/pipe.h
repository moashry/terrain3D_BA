#ifndef EQ_TERRAIN3D_PIPE_H
#define EQ_TERRAIN3D_PIPE_H

#include <eq/client/node.h>
#include <eq/client/pipe.h>

#include "profiler/profiler.hpp"
#include "profiler/profilerlogwriter.hpp"

#include "config.h"
#include "frameData.h"
#include "node.h"

class DataLoader;
class Terrain;
class OverviewMap;

namespace Renderer
{
	class Device;
}

namespace eqTerrain3D
{
	class Pipe : public eq::Pipe
	{
		typedef eq::Pipe super;

	public:
		Pipe( eq::Node* parent );

		void glewSetContext( GLEWContext *context );

		const FrameData& getFrameData() const { return _frameData; }

		// redeclare some get* functions to work with the specialized classes
		Config*       getConfig()       { return static_cast< Config* >      ( super::getConfig() ); }
		const Config* getConfig() const { return static_cast< const Config* >( super::getConfig() ); }

		Node*       getNode()       { return static_cast< Node* >      ( super::getNode() ); }
		const Node* getNode() const { return static_cast< const Node* >( super::getNode() ); }

		Terrain*    getTerrain()    { return _terrain; }
		DataLoader* getDataLoader() { return getNode()->getDataLoader(); }

		OverviewMap* getOverviewMap() { return _overviewMap; }

		Renderer::FramebufferTexture2D* getLogoTexture() { return _logoTexture; }

		std::string getProfileLogName() const; // without extension

		// not threadsafe - use only from pipe thread!
		Profiler* getProfiler() { return &_profiler; }

	private:
		eq::WindowSystem selectWindowSystem() const;
		bool configInit( const uint32_t initID );
		bool configExit();
		void frameStart( const uint32_t frameID, const uint32_t frameNumber );
		void frameFinish( const uint32_t frameID, const uint32_t frameNumber );

		void initGL();
		void exitGL();

		FrameData         _frameData;

		GLEWContext*      _glewContext;
		GLEWContext*      glewGetContext() { return _glewContext; }

		Renderer::Device* _device;
		Terrain*          _terrain;
		OverviewMap*      _overviewMap;
		Renderer::FramebufferTexture2D* _logoTexture;

		Profiler          _profiler;
		ProfilerLogWriter _profilerLogWriter;
		uint              _profilerOutputFrequency;
		uint              _profiledFrames;
	};
}

#endif // EQ_TERRAIN3D_PIPE_H
