#include "frameData.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eqTerrain3D
{
	FrameData::FrameData()
	#define INIT_STATE( name ) _b##name( false )
		: INIT_STATE( Wireframe )
		, INIT_STATE( BoundingBoxes )
		, INIT_STATE( PageBoundingBoxes )
		, INIT_STATE( StaticMesh )
		, INIT_STATE( OcclusionCulling )
		, INIT_STATE( RenderHelp )
		, INIT_STATE( RenderStatistics )
		, INIT_STATE( RenderOverviewMap )
		, INIT_STATE( RenderLogo )
		, INIT_STATE( SatModify )
		, INIT_STATE( ProfilerEnabled )
		, _fSaturation( 1.0f )
		, _activeViewID( EQ_ID_INVALID )
		, _activeChannelID( EQ_ID_INVALID )
	#undef INIT_STATE
	{
		EQINFO << "New FrameData " << std::endl;
	}

	FrameData::~FrameData()
	{
		for( uint i = 0; i < _cameras.size(); i++ ) {
			delete _cameras[ i ];
		}
	}

	void FrameData::reset()
	{
		#define RESET_STATE( name )	do { _b##name = false; } while( false )
		RESET_STATE( Wireframe );
		RESET_STATE( BoundingBoxes );
		RESET_STATE( PageBoundingBoxes );
		RESET_STATE( StaticMesh );
		RESET_STATE( OcclusionCulling );
		RESET_STATE( RenderHelp );
		RESET_STATE( RenderStatistics );
		RESET_STATE( RenderOverviewMap );
		RESET_STATE( RenderLogo );
		RESET_STATE( SatModify );
		RESET_STATE( ProfilerEnabled );
		#undef RESET_STATE
		_fSaturation = 1.0f;
		_activeViewID = EQ_ID_INVALID;
		_activeChannelID = EQ_ID_INVALID;
		_message.clear();
	}


	void FrameData::setSaturation( const float &fSaturation )
	{
		setDirty( DIRTY_SATURATION );
		_fSaturation = fSaturation;
	}

	void FrameData::setNumCameras( uint count )
	{
		setDirty( DIRTY_CAMERAS );

		uint oldCount = uint( _cameras.size() );
		// delete excess cameras
		for( uint i = count; i < oldCount; i++ ) {
			delete _cameras[ i ];
		}
		_cameras.resize( count );
		// create new cameras if necessary
		for( uint i = oldCount; i < count; i++ ) {
			_cameras[ i ] = new Camera();
		}
	}

	void FrameData::setActiveViewID( uint32_t id )
	{
		setDirty( DIRTY_ACTIVE_VIEW );
		_activeViewID = id;
	}

	void FrameData::setActiveChannelID( uint32_t id )
	{
		setDirty( DIRTY_ACTIVE_CHANNEL );
		_activeChannelID = id;
	}

	void FrameData::setMessage( const std::string& message )
	{
		if( _message == message )
			return;

		setDirty( DIRTY_MESSAGE );
		_message = message;
	}


	void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
	{
		eq::Object::serialize( os, dirtyBits );
		
		if( dirtyBits & DIRTY_STATE ) {
			ushort state = 0;
			ushort bitMask = 1;
			#define SERIALIZE_STATE( name )	do { if( _b##name ) state |= bitMask; bitMask <<= 1; } while( false )
			SERIALIZE_STATE( Wireframe );
			SERIALIZE_STATE( BoundingBoxes );
			SERIALIZE_STATE( PageBoundingBoxes );
			SERIALIZE_STATE( StaticMesh );
			SERIALIZE_STATE( OcclusionCulling );
			SERIALIZE_STATE( RenderHelp );
			SERIALIZE_STATE( RenderStatistics );
			SERIALIZE_STATE( RenderOverviewMap );
			SERIALIZE_STATE( RenderLogo );
			SERIALIZE_STATE( SatModify );
			SERIALIZE_STATE( ProfilerEnabled );
			#undef SERIALIZE_STATE
			os << state;
		}
		if( dirtyBits & DIRTY_SATURATION ) {
			os << _fSaturation;
		}
		if( dirtyBits & DIRTY_CAMERAS ) {
			uint numCameras = getNumCameras();
			os << numCameras;
			for( uint i = 0; i < numCameras; i++ ) {
				_cameras[ i ]->getInstanceData( os );
			}
		}
		if( dirtyBits & DIRTY_ACTIVE_VIEW ) {
			os << _activeViewID;
		}
		if( dirtyBits & DIRTY_ACTIVE_CHANNEL ) {
			os << _activeChannelID;
		}
		if( dirtyBits & DIRTY_MESSAGE ) {
			os << _message;
		}
	}

	void FrameData::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
	{
		eq::Object::deserialize( is, dirtyBits );

		if( dirtyBits & DIRTY_STATE ) {
			ushort state;
			is >> state;
			ushort bitMask = 1;
			#define SERIALIZE_STATE( name )	do { _b##name = (state & bitMask) != 0; bitMask <<= 1; } while( false )
			SERIALIZE_STATE( Wireframe );
			SERIALIZE_STATE( BoundingBoxes );
			SERIALIZE_STATE( PageBoundingBoxes );
			SERIALIZE_STATE( StaticMesh );
			SERIALIZE_STATE( OcclusionCulling );
			SERIALIZE_STATE( RenderHelp );
			SERIALIZE_STATE( RenderStatistics );
			SERIALIZE_STATE( RenderOverviewMap );
			SERIALIZE_STATE( RenderLogo );
			SERIALIZE_STATE( SatModify );
			SERIALIZE_STATE( ProfilerEnabled );
			#undef SERIALIZE_STATE
		}
		if( dirtyBits & DIRTY_SATURATION ) {
			is >> _fSaturation;
		}
		if( dirtyBits & DIRTY_CAMERAS ) {
			uint oldNumCameras = uint( _cameras.size() );
			uint numCameras;
			is >> numCameras;
			// delete excess cameras
			for( uint i = numCameras; i < oldNumCameras; i++ ) {
				delete _cameras[ i ];
			}
			_cameras.resize( numCameras );
			// create cameras if necessary
			for( uint i = oldNumCameras; i < numCameras; i++ ) {
				_cameras[ i ] = new Camera();
			}
			// read data
			for( uint i = 0; i < numCameras; i++ ) {
				_cameras[ i ]->applyInstanceData( is );
			}
		}
		if( dirtyBits & DIRTY_ACTIVE_VIEW ) {
			is >> _activeViewID;
		}
		if( dirtyBits & DIRTY_ACTIVE_CHANNEL ) {
			is >> _activeChannelID;
		}
		if( dirtyBits & DIRTY_MESSAGE ) {
			is >> _message;
		}
	}
}
