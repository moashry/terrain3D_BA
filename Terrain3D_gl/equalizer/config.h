#ifndef EQ_TERRAIN3D_CONFIG_H
#define EQ_TERRAIN3D_CONFIG_H

#include <vector>

#include <eq/base/refPtr.h>
#include <eq/client/client.h>
#include <eq/client/config.h>
#include <eq/client/configEvent.h>
#include <eq/client/server.h>

#include "terrainLib/terrainlib.h"

#include "localInitData.h"
#include "frameData.h"
#include "view.h"

#include "camera.h"
#include "recorder.h"

namespace eqTerrain3D
{
	class Config : public eq::Config
	{
	public:
		Config( eq::base::RefPtr< eq::Server > parent );

		bool init();
		bool exit();

		uint32_t startFrame();

		void setInitData( const LocalInitData& data ) { _initData = data; }
		const InitData& getInitData() const { return _initData; }

		// Map/unmap per-config data to the local node process 
		void mapData( const uint32_t initDataID );
		void unmapData();

		View* findView( const uint32_t id ) { return static_cast< View* >( eq::Config::findView( id ) ); }

		bool handleEvent( const eq::ConfigEvent* event );

		bool needsRedraw();

	private:
		struct Viewer {
			Camera *camera;
			Recorder *recorder;

			Viewer( Camera *camera, Recorder *recorder )
				: camera( camera ), recorder( recorder ) {}

			bool keyEvent( uint32_t key );
			bool handleEvent( const eq::ConfigEvent* event );
		};


		LocalInitData         _initData;
		FrameData             _frameData;

		std::vector< Viewer > _viewers;
		bool                  _camerasInited;

		int64_t               _lastFrameTime;

		int64_t               _messageTime;


		Viewer* _getViewer( uint32_t viewID );

		void    _deregisterData();

		bool    _frameDataKeyEvent( uint32_t key );

		void    _setMessage( const std::string& message );
		void    _setMessage( const std::string& message, bool enabled );
		static std::string _getEnabledDisabled( bool enabled );
	};
}

#endif // EQ_TERRAIN3D_CONFIG_H
