#include <eq/eq.h>

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"
#include "view.h"

#include <stdlib.h>

#include <fstream>

using namespace eq::base;
using namespace std;

class NodeFactory : public eq::NodeFactory
{
public:
	eq::Config*  createConfig( eq::ServerPtr parent ) { return new eqTerrain3D::Config( parent ); }
	eq::Node*    createNode( eq::Config* parent )     { return new eqTerrain3D::Node( parent ); }
	eq::Pipe*    createPipe( eq::Node* parent )       { return new eqTerrain3D::Pipe( parent ); }
	eq::Window*  createWindow( eq::Pipe* parent )     { return new eqTerrain3D::Window( parent ); }
	eq::Channel* createChannel( eq::Window* parent )  { return new eqTerrain3D::Channel( parent ); }
	eq::View*	 createView()                         { return new eqTerrain3D::View(); }
};

int run( RefPtr< eq::Client > &client, eqTerrain3D::LocalInitData &initData ) {
	// 1. connect to server
	eq::ServerPtr server = new eq::Server;
	if( !client->connectServer( server ))
	{
		EQERROR << "Can't open server" << endl;
		return EXIT_FAILURE;
	}

	// 2. choose config
	eq::ConfigParams configParams;
	eqTerrain3D::Config* config = static_cast<eqTerrain3D::Config*>( server->chooseConfig( configParams ) );

	if( !config )
	{
		EQERROR << "No matching config on server" << endl;
		client->disconnectServer( server );
		return EXIT_FAILURE;
	}

	// 3. init config
	eq::base::Clock clock;

	config->setInitData( initData );
	if( !config->init( ))
	{
		EQERROR << "Config initialization failed: " 
			<< config->getErrorMessage() << endl;
		server->releaseConfig( config );
		client->disconnectServer( server );
		return EXIT_FAILURE;
	}

	EQLOG( eq::LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
		<< endl;

	// 4. run main loop
	uint32_t maxFrames = initData.getMaxFrames();

	clock.reset();
	while( config->isRunning( ) && maxFrames-- )
	{
		config->startFrame();
		config->finishFrame();

		// this whole block is not really needed - I think its meant to handle events better and sleep if nothing happens (finishFrame also handles events) [10/10/2009 Andreas Kirsch]
		// {
		while( !config->needsRedraw( )) // wait for an event requiring redraw
		{
			if( client->hasCommands( )) // execute non-critical pending commands
			{
				client->processCommand();
				config->handleEvents(); // non-blocking
			}
			else  // no pending commands, block on user event
			{
				const eq::ConfigEvent* event = config->nextEvent();
				if( !config->handleEvent( event ))
					EQVERB << "Unhandled " << event << endl;
			}
		}
		config->handleEvents(); // process all pending events
		// }
	}
	const uint32_t frame = config->finishAllFrames();
	const float    time  = clock.getTimef();
	EQLOG( eq::LOG_STATS ) << "Rendering took " << time << " ms (" << frame
		<< " frames @ " << ( frame / time * 1000.f) << " FPS)"
		<< endl;

	// 5. exit config
	clock.reset();
	config->exit();
	EQLOG( eq::LOG_STATS ) << "Exit took " << clock.getTimef() << " ms" << endl;

	// 6. cleanup and exit
	server->releaseConfig( config );
	if( !client->disconnectServer( server ))
		EQERROR << "Client::disconnectServer failed" << endl;

	server = 0;
	return EXIT_SUCCESS;
}

int main( const int argc, char** argv )
{
	// enable run-time memory check for debug builds
	#if defined( _WIN32 ) && ( defined( DEBUG ) || defined( _DEBUG ) )
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	// 1. parse arguments
	eqTerrain3D::LocalInitData initData;
	if( !initData.parseArguments( argc, argv ) ) {
		EQERROR << "Parsing command line arguments failed" << endl;
		return EXIT_FAILURE;
	}

	// 2. Redirect log output to file, if requested
	std::ofstream* logFile = 0;
	const std::string& logFilename = initData.getLogFilename();
	if( !logFilename.empty( ))
	{
		logFile = new ofstream( logFilename.c_str( ));
		eq::base::Log::setOutput( *logFile );
	}

	// 3. Equalizer initialization
	NodeFactory nodeFactory;
	if( !eq::init( argc, argv, &nodeFactory ))
	{
		EQERROR << "Equalizer init failed" << endl;
		return EXIT_FAILURE;
	}

	// 4. initialization of local client node
	RefPtr< eq::Client > client = new eq::Client;
	if( !client->initLocal( argc, argv ))
	{
		EQERROR << "Can't init client" << endl;
		eq::exit();
		return EXIT_FAILURE;
	}

	// 5. run main-loop
	int ret = run( client, initData );

	// 6. cleanup and exit
	client->exitLocal();

	client = 0;

	eq::exit();

	if( logFile )
		logFile->close();
	delete logFile;

	return ret;
}