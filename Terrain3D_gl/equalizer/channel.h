#ifndef EQ_TERRAIN3D_CHANNEL_H
#define EQ_TERRAIN3D_CHANNEL_H

#include <string>

#include <eq/client/types.h>
#include <eq/client/channel.h>
#include <eq/client/window.h>

#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"
#include "view.h"

class Camera;
class DataLoader;
class Terrain;


namespace eqTerrain3D
{
	class FrameData;
	class InitData;

	class Channel : public eq::Channel
	{
		typedef eq::Channel super;

	public:
		Channel( eq::Window* parent )
			: eq::Channel( parent ), _dataLoaderViewIndex( ~0u ), _staticFrustumDirty( true ) {}

		// redeclare some get* functions to work with the specialized classes
		Config*       getConfig()       { return static_cast< Config* >(       super::getConfig() ); }
		const Config* getConfig() const { return static_cast< const Config* >( super::getConfig() ); }

		Node*       getNode()       { return static_cast< Node* >(       super::getNode() ); }
		const Node* getNode() const { return static_cast< const Node* >( super::getNode() ); }

		Pipe*       getPipe()       { return static_cast< Pipe* >(       super::getPipe() ); }
		const Pipe* getPipe() const { return static_cast< const Pipe* >( super::getPipe() ); }

		Window*       getWindow()       { return static_cast< Window* >(       super::getWindow() ); }
		const Window* getWindow() const { return static_cast< const Window* >( super::getWindow() ); }

		// no non-const getView!
		const View* getView() const { return static_cast< const View* >( super::getView() ); }

	private:
		bool configInit( const uint32_t initID );
		bool configExit();

		void frameClear( const uint32_t frameID );
		void frameDraw( const uint32_t frameID );
		void frameReadback( const uint32_t frameID );
		void frameViewFinish( const uint32_t frameID );

		void _drawViewFrustum( const eq::Matrix4f& modelView );

		void _drawHelp();
		void _drawMessage( const std::string& message );
		void _drawOverviewMap();
		void _drawLogo();

		Terrain*    getTerrain()    { return getPipe()->getTerrain(); }
		DataLoader* getDataLoader() { return getPipe()->getDataLoader(); }

		const FrameData& _getFrameData() const;

		void _rasterizerStateSolid() const;
		void _rasterizerStateWireframe() const;

		// frustum, LOD, etc
		void _updateProjection();

		void _updateStaticFrustum( const eq::Matrix4f& modelViewProj );

		uint _dataLoaderViewIndex;

		eq::Vector3f _staticFrustum[ 8 ];
		bool         _staticFrustumDirty;
	};
}

#endif // EQ_TERRAIN3D_CHANNEL_H
