#include "localInitData.h"


namespace eqTerrain3D
{
	LocalInitData::LocalInitData()
		: _maxFrames( 0xffffffffu )
		, _isResident( false )
	{
	}

	const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
	{
		setSystemConfiguration( from.getSystemConfiguration() );
		setTerrainConfiguration( from.getTerrainConfiguration() );
		setWindowSystem( from.getWindowSystem( ));

		return *this;
	}

	bool LocalInitData::parseArguments( const int argc, char** argv )
	{
		std::string configFile = "Terrain3D.cfg";
		std::string terrainFile;

		// parse arguments - look for *.cfg and *.terrain
		for( int i = 1; i < argc; i++ ) {
			std::string arg( argv[ i ] );
			if( arg == "--" ) {
				// this indicates the end of application-specific args, the rest is meant for equalizer
				break;
			}
			if( arg.substr( arg.length() - 4 ) == ".cfg" ) {
				configFile = arg;
			}
		}

		// read system config
		if( !_systemConfiguration.ReadFile( configFile ) ) {
			return false;
		}

		// if terrain file wasn't specified on cmdline, use value from system config
		if( terrainFile.empty() ) {
			terrainFile = _systemConfiguration.GetTerrainFileName();
		}

		// read terrain config
		if( !_terrainConfiguration.ReadFile( terrainFile ) ) {
			return false;
		}

		return true;
	}
}
