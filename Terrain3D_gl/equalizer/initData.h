#ifndef EQ_TERRAIN3D_INITDATA_H
#define EQ_TERRAIN3D_INITDATA_H

#include <eq/client/windowSystem.h>
#include <eq/net/object.h>

#include "terrainLib/terrainConfiguration.h"

#include "configuration.h"


namespace eqTerrain3D
{
	class InitData : public eq::net::Object
	{
	public:
		InitData();
		virtual ~InitData();

		void setFrameDataID( const uint32_t id )   { _frameDataID = id; }

		uint32_t           getFrameDataID() const   { return _frameDataID; }
		eq::WindowSystem   getWindowSystem() const  { return _windowSystem; }

		const SystemConfiguration& getSystemConfiguration() const { return _systemConfiguration; }
		const TerrainConfiguration& getTerrainConfiguration() const { return _terrainConfiguration; }

	protected:
		void getInstanceData( eq::net::DataOStream& os );
		void applyInstanceData( eq::net::DataIStream& is );

		void setWindowSystem( const eq::WindowSystem windowSystem ) { _windowSystem = windowSystem; }

		void setSystemConfiguration( const SystemConfiguration &systemConfiguration	) { _systemConfiguration = systemConfiguration; }
		void setTerrainConfiguration( const TerrainConfiguration &terrainConfiguration	) { _terrainConfiguration = terrainConfiguration; }

		SystemConfiguration  _systemConfiguration;
		TerrainConfiguration _terrainConfiguration;

	private:
		uint32_t         _frameDataID;
		eq::WindowSystem _windowSystem;
	};
}


#endif // EQ_TERRAIN3D_INITDATA_H