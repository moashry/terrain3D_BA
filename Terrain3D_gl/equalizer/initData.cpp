#include "initData.h"


namespace eqTerrain3D
{
	InitData::InitData()
			: _frameDataID( EQ_ID_INVALID )
			, _windowSystem( eq::WINDOW_SYSTEM_NONE )
	{
	}

	InitData::~InitData()
	{
		setFrameDataID( EQ_ID_INVALID );
	}

	void InitData::getInstanceData( eq::net::DataOStream& os )
	{
		os << _frameDataID << _windowSystem;

		_systemConfiguration.getInstanceData( os );

		os << _terrainConfiguration.GetTextureCompression();
		os << _terrainConfiguration.GetLevelZeroTileExtent();
		os << _terrainConfiguration.GetNumLevels();
		const std::vector<FileDesc>& fileDescs = _terrainConfiguration.GetFileDescs();
		os << uint( fileDescs.size() );
		for( uint i = 0; i < fileDescs.size(); i++ ) {
			os << fileDescs[ i ].m_uiFileID << fileDescs[ i ].m_ptcFileName;
		}
		os << _terrainConfiguration.GetTreeDescs();
		os << _terrainConfiguration.GetOverviewMapFileName();
		float x0, y0, extentX, extentY;
		_terrainConfiguration.GetOverviewMapExtents( &x0, &y0, &extentX, &extentY );
		os << x0 << y0 << extentX << extentY;
		os << _terrainConfiguration.GetMinVerticalTolerance();
		os << _terrainConfiguration.GetDemoFlightFileName();

		os << _terrainConfiguration.GetMaxSpeed();
		os << _terrainConfiguration.GetMinHeight();

		uint numStartLocations = _terrainConfiguration.GetNumStartLocations();
		os << numStartLocations;
		for( uint i = 0; i < numStartLocations; i++ ) {
			os << _terrainConfiguration.GetStartLocation( i );
		}
	}

	void InitData::applyInstanceData( eq::net::DataIStream& is )
	{
		is >> _frameDataID >> _windowSystem;

		_systemConfiguration.applyInstanceData( is );

		TEXTURE_COMPRESSION texComp; is >> texComp;
		_terrainConfiguration.SetTextureCompression( texComp );
		float levelZeroTileExtent; is >> levelZeroTileExtent;
		_terrainConfiguration.SetLevelZeroTileExtent( levelZeroTileExtent );
		uint numLevels; is >> numLevels;
		_terrainConfiguration.SetNumLevels( numLevels );
		std::vector< FileDesc > fileDescs;
		uint numFileDescs; is >> numFileDescs;
		for( uint i = 0; i < numFileDescs; i++ ) {
			FileDesc desc; is >> desc.m_uiFileID >> desc.m_ptcFileName;
			fileDescs.push_back( desc );
		}
		_terrainConfiguration.SetFileDescs( fileDescs );
		std::vector< TreeDesc > treeDescs; is >> treeDescs;
		_terrainConfiguration.SetTreeDescs( treeDescs );
		std::string overviewMap; is >> overviewMap;
		_terrainConfiguration.SetOverviewMapFileName( overviewMap );
		float x0, y0, extentX, extentY; is >> x0 >> y0 >> extentX >> extentY;
		_terrainConfiguration.SetOverviewMapExtents( x0, y0, extentX, extentY );
		float minTolerance; is >> minTolerance;
		_terrainConfiguration.SetMinVerticalTolerance( minTolerance );
		std::string demoFlight; is >> demoFlight;
		_terrainConfiguration.SetDemoFlightFileName( demoFlight );
		float maxSpeed; is >> maxSpeed;
		_terrainConfiguration.SetMaxSpeed( maxSpeed );
		float minHeight; is >> minHeight;
		_terrainConfiguration.SetMinHeight( minHeight );

		uint numStartLocations; is >> numStartLocations;
		std::vector< StartLocation > startLocations;
		for( uint i = 0; i < numStartLocations; i++ ) {
			StartLocation location; is >> location;
			startLocations.push_back( location );
		}
		_terrainConfiguration.SetStartLocations( startLocations );

		EQASSERT( _frameDataID != EQ_ID_INVALID );
		EQINFO << "New InitData instance" << std::endl;
	}
}
