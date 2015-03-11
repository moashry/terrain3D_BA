#include "event.h"


namespace eqTerrain3D
{
	CameraParamsEvent::CameraParamsEvent()
	{
		memset( this, 0, sizeof( CameraParamsEvent ) );
	}

	CameraParamsEvent::CameraParamsEvent( const eq::UserEvent& eqEvent )
	{
		const char* data = eqEvent.data;

		_cameraIndex = uint( *reinterpret_cast< const ushort* >( data ) );
		data += sizeof( ushort );
		_viewportWidth = uint( *reinterpret_cast< const ushort* >( data ) );
		data += sizeof( ushort );
		_viewportHeight = uint( *reinterpret_cast< const ushort* >( data ) );
		data += sizeof( ushort );
		_fovy = *reinterpret_cast< const float* >( data );
		data += sizeof( float );
		_offsetx = int( *reinterpret_cast< const short* >( data ) );
		data += sizeof( short );
		_offsety = int( *reinterpret_cast< const short* >( data ) );
		data += sizeof( short );
		for( uint i = 0; i < 4; i++ ) {
			_headTransform.array[ 4*i + 0 ] = *reinterpret_cast< const float* >( data );
			data += sizeof( float );
			_headTransform.array[ 4*i + 1 ] = *reinterpret_cast< const float* >( data );
			data += sizeof( float );
			_headTransform.array[ 4*i + 2 ] = *reinterpret_cast< const float* >( data );
			data += sizeof( float );
			_headTransform.array[ 4*i + 3 ] = 0.0f;
		}
		_headTransform.array[ 15 ] = 1.0f;

		assert( data - eqEvent.data <= EQ_USER_EVENT_SIZE );
	}

	void CameraParamsEvent::fillEqEvent( eq::UserEvent& eqEvent )
	{
		char* data = eqEvent.data;

		// convert members to smaller types, and skip fourth column of head matrix,
		// so this fits into EQ_USER_EVENT_SIZE (64)
		*reinterpret_cast< ushort* >( data ) = ushort( _cameraIndex );
		data += sizeof( ushort );
		*reinterpret_cast< ushort* >( data ) = ushort( _viewportWidth );
		data += sizeof( ushort );
		*reinterpret_cast< ushort* >( data ) = ushort( _viewportHeight );
		data += sizeof( ushort );
		*reinterpret_cast< float* >( data ) = _fovy;
		data += sizeof( float );
		*reinterpret_cast< ushort* >( data ) = ushort( _offsetx );
		data += sizeof( ushort );
		*reinterpret_cast< ushort* >( data ) = ushort( _offsety );
		data += sizeof( ushort );
		for( uint i = 0; i < 4; i++ ) {
			*reinterpret_cast< float* >( data ) = _headTransform.array[ 4*i + 0 ];
			data += sizeof( float );
			*reinterpret_cast< float* >( data ) = _headTransform.array[ 4*i + 1 ];
			data += sizeof( float );
			*reinterpret_cast< float* >( data ) = _headTransform.array[ 4*i + 2 ];
			data += sizeof( float );
		}

		assert( data - eqEvent.data <= EQ_USER_EVENT_SIZE );
	}
}
