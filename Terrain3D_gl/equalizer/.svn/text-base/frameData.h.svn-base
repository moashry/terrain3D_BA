#ifndef EQ_TERRAIN3D_FRAMEDATA_H
#define EQ_TERRAIN3D_FRAMEDATA_H

#include <eq/client/object.h>
#include <eq/client/types.h>

#include "global.h"
#include "camera.h"

namespace eqTerrain3D
{
	class FrameData : public eq::Object
	{
	public:
		FrameData();
		~FrameData();

		void reset();

#define STATE( name ) \
public: \
	void set##name( const bool b##name ) { setDirty( DIRTY_STATE ); _b##name = b##name; } \
	bool get##name() const { return _b##name; } \
	void toggle##name() { setDirty( DIRTY_STATE ); _b##name = !_b##name; } \
private:\
	bool _b##name;

		STATE( Wireframe );
		STATE( BoundingBoxes );
		STATE( PageBoundingBoxes );
		STATE( StaticMesh );
		STATE( OcclusionCulling );
		STATE( RenderHelp );
		STATE( RenderStatistics );
		STATE( RenderOverviewMap );
		STATE( RenderLogo );
		STATE( SatModify );
		STATE( ProfilerEnabled );

#undef STATE

	public:
		float         getSaturation() const { return _fSaturation; }
		void          setSaturation( const float& fSaturation );

		uint          getNumCameras() const { return uint( _cameras.size() ); }
		void          setNumCameras( uint count );

		Camera*       getCamera( uint index )       { setDirty( DIRTY_CAMERAS ); return _cameras[ index ]; }
		const Camera* getCamera( uint index ) const { return _cameras[ index ]; }
		void          markCameraDirty( uint index ) { setDirty( DIRTY_CAMERAS ); }

		uint32_t      getActiveViewID() const { return _activeViewID; }
		void          setActiveViewID( uint32_t id );

		uint32_t      getActiveChannelID() const { return _activeChannelID; }
		void          setActiveChannelID( uint32_t id );

		const std::string& getMessage() const { return _message; }
		void               setMessage( const std::string& message );

	private:
		float _fSaturation;

		std::vector< Camera* > _cameras; //TODO store which are dirty

		uint32_t _activeViewID;
		uint32_t _activeChannelID;

		std::string _message;


		void serialize( eq::net::DataOStream& os, const uint64_t dirtyBits );
		void deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits );

		ChangeType getChangeType() const { return DELTA; }

		// The changed parts of the data since the last pack()
		enum DirtyBits
		{
			DIRTY_STATE          = eq::Object::DIRTY_CUSTOM << 0,
			DIRTY_SATURATION     = eq::Object::DIRTY_CUSTOM << 1,
			DIRTY_CAMERAS        = eq::Object::DIRTY_CUSTOM << 2,
			DIRTY_ACTIVE_VIEW    = eq::Object::DIRTY_CUSTOM << 3,
			DIRTY_ACTIVE_CHANNEL = eq::Object::DIRTY_CUSTOM << 4,
			DIRTY_MESSAGE        = eq::Object::DIRTY_CUSTOM << 5
		};
	};
}

#endif // EQ_TERRAIN3D_FRAMEDATA_H
