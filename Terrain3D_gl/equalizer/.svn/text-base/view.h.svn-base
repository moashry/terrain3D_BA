#ifndef EQ_TERRAIN3D_VIEW_H
#define EQ_TERRAIN3D_VIEW_H

#include <eq/client/view.h>

#include "global.h"

namespace eqTerrain3D
{
	class View : public eq::View
	{
	public:
		View() : _cameraIndex( 0 ) {}

		uint getCameraIndex() const {
			return _cameraIndex;
		}

		void setCameraIndex( uint cameraIndex ) {
			setDirty( DIRTY_CAM_INDEX );
			_cameraIndex = cameraIndex;
		}

	private:
		uint _cameraIndex;

		virtual void serialize( eq::net::DataOStream& os, const uint64_t dirtyBits ) {
			eq::View::serialize( os, dirtyBits );

			if( dirtyBits & DIRTY_CAM_INDEX ) {
				os << _cameraIndex;
			}
		}

		virtual void deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits ) {
			eq::View::deserialize( is, dirtyBits );

			if( dirtyBits & DIRTY_CAM_INDEX ) {
				is >> _cameraIndex;
			}
		}

		enum DirtyBits
		{
			DIRTY_CAM_INDEX = View::DIRTY_CUSTOM << 0
		};
	};
}

#endif // EQ_TERRAIN3D_VIEW_H
