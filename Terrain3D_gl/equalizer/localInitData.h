#ifndef EQ_TERRAIN3D_LOCALINITDATA_H
#define EQ_TERRAIN3D_LOCALINITDATA_H

#include <string>

#include "initData.h"


namespace eqTerrain3D
{
	class LocalInitData : public InitData
	{
	public:
		LocalInitData();

		bool parseArguments( const int argc, char** argv );

		const std::string& getLogFilename() const { return _logFilename; }

		bool               isResident()     const { return _isResident; }
		uint32_t           getMaxFrames()   const { return _maxFrames; }


		const LocalInitData& operator = ( const LocalInitData& from );

	private:
		//TODO: log, and maxframes/resident
		std::string _logFilename;

		uint32_t    _maxFrames;
		bool        _isResident;
	};
}


#endif // EQ_TERRAIN3D_LOCALINITDATA_H
