#ifndef EQ_TERRAIN3D_EVENT_H
#define EQ_TERRAIN3D_EVENT_H


#include <eq/client/event.h>
#include <eq/client/types.h>

#include "global.h"


namespace eqTerrain3D
{
	enum ECustomEventType
	{
		CUSTOMEVENT_UINT32_ACTIVECHANNEL = eq::Event::USER,
		CUSTOMEVENT_CAMERAPARAMS
	};


	template< typename Type, uint count = 1 >
	class SingleTypeEvent
	{
	public:
		SingleTypeEvent();
		SingleTypeEvent( const eq::UserEvent& eqEvent );

		Type		getValue( uint i = 0 ) const { return m_value[ i ]; }
		const Type*	getValues() const { return m_value; }
		void		setValue( const Type& value, uint i = 0 ) { m_value[ i ] = value; }
		void		setValues( const Type values[ count ] ) { for( uint i = 0; i < count; i++ ) setValue( values[ i ], i ); }

		void		fillEqEvent( eq::UserEvent& eqEvent );

	private:
		Type		m_value[ count ];
	};

	typedef SingleTypeEvent< uint32_t > Uint32Event;


	class CameraParamsEvent
	{
	public:
		CameraParamsEvent();
		CameraParamsEvent( const eq::UserEvent& eqEvent );

		uint                getCameraIndex()    const { return _cameraIndex; }
		uint                getViewportWidth()  const { return _viewportWidth; }
		uint                getViewportHeight() const { return _viewportHeight; }
		float               getFovy()           const { return _fovy; }
		int                 getOffsetX()        const { return _offsetx; }
		int                 getOffsetY()        const { return _offsety; }
		const eq::Matrix4f& getHeadTransform()  const { return _headTransform; }

		void                setCameraIndex( uint index )                          { _cameraIndex = index; }
		void                setViewportWidth( uint width )                        { _viewportWidth = width; }
		void                setViewportHeight( uint height )                      { _viewportHeight = height; }
		void                setFovy( float fovy )                                 { _fovy = fovy; }
		void                setOffsetX( int offsetx )                             { _offsetx = offsetx; }
		void                setOffsetY( int offsety )                             { _offsety = offsety; }
		void                setHeadTransform( const eq::Matrix4f& headTransform ) { _headTransform = headTransform; }

		void                fillEqEvent( eq::UserEvent& eqEvent );

	private:
		uint         _cameraIndex;
		uint         _viewportWidth;
		uint         _viewportHeight;
		float        _fovy;
		int          _offsetx;
		int          _offsety;
		eq::Matrix4f _headTransform;
	};


	// implementation of SingleTypeEvent
	namespace
	{
		template< typename Type >
		Type getDataItem( const eq::UserEvent& eqEvent, uint offset ) {
			return *reinterpret_cast< const Type* >( eqEvent.data + offset );
		}

		template< typename Type >
		void setDataItem( eq::UserEvent& eqEvent, uint offset, Type value ) {
			*reinterpret_cast< Type* >( eqEvent.data + offset ) = value;
		}
	}

	template< typename Type, uint count >
	SingleTypeEvent< Type, count >::SingleTypeEvent()
	{
		assert( count * sizeof( Type ) <= EQ_USER_EVENT_SIZE );
		for( uint i = 0; i < count; i++ ) {
			m_value[ i ] = ( Type )0;
		}
	}

	template< typename Type, uint count >
	SingleTypeEvent< Type, count >::SingleTypeEvent( const eq::UserEvent& eqEvent )
	{
		assert( count * sizeof( Type ) <= EQ_USER_EVENT_SIZE );
		for( uint i = 0; i < count; i++ ) {
			m_value[ i ] = getDataItem< Type >( eqEvent, i * sizeof( Type ) );
		}
	}

	template< typename Type, uint count >
	void SingleTypeEvent< Type, count >::fillEqEvent( eq::UserEvent& eqEvent )
	{
		for( uint i = 0; i < count; i++ ) {
			setDataItem< Type >( eqEvent, i * sizeof( Type ), m_value[ i ] );
		}
	}

}


#endif EQ_TERRAIN3D_EVENT_H
