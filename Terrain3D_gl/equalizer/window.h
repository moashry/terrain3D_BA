#ifndef EQ_TERRAIN3D_WINDOW_H
#define EQ_TERRAIN3D_WINDOW_H

#include <eq/client/pipe.h>
#include <eq/client/window.h>

#include "config.h"
#include "node.h"
#include "pipe.h"

namespace eqTerrain3D
{
	class Window : public eq::Window
	{
		typedef eq::Window super;

	public:
		Window( eq::Pipe* parent )
			: eq::Window( parent ) {}

		// redeclare some get* functions to work with the specialized classes
		Config*       getConfig()       { return static_cast< Config* >      ( super::getConfig() ); }
		const Config* getConfig() const { return static_cast< const Config* >( super::getConfig() ); }

		Node*       getNode()       { return static_cast< Node* >      ( super::getNode() ); }
		const Node* getNode() const { return static_cast< const Node* >( super::getNode() ); }

		Pipe*       getPipe()       { return static_cast< Pipe* >      ( super::getPipe() ); }
		const Pipe* getPipe() const { return static_cast< const Pipe* >( super::getPipe() ); }

	private:
		bool configInitGL( const uint32_t initID );
		bool configExitGL();

		bool processEvent( const eq::Event& event );
	};
}

#endif // EQ_TERRAIN3D_WINDOW_H
