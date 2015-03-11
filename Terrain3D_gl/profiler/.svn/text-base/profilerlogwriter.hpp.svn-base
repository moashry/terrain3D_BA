/*************************************************************************************

Simple Runtime Profiler

Author: Marc Treib

(c) Marc Treib

mailto:treib@in.tum.de

Last Changed: 2010-04-27

*************************************************************************************/

#ifndef _profilerlogwriter_hpp_
#define _profilerlogwriter_hpp_


#include <fstream>
#include <string>

#include "profiler.hpp"

typedef unsigned int uint;


class ProfilerLogWriter : public IProfilerOutputHandler {
public:
	ProfilerLogWriter( );
	ProfilerLogWriter( const std::string& fileName );
	~ProfilerLogWriter( );

	bool open( const std::string& fileName );
	void close( );
	bool isOpen( ) const;
	bool isGood( ) const;

	void started( float timestampMS );
	void stopped( float timestampMS, float profiledTimeMS, uint profiledFrames );

	void beginOutput( float timestampMS, float profiledTimeMS, uint profiledFrames );
	void putSample( const std::string& name, uint parentCount, bool hasUncountedChildren, float totalTimeMS, float maxCallTimeMS, float maxFrameTimeMS, float ownTimeMS, uint totalCount, uint maxFrameCount );
	void endOutput( );

	void cleared( float timestampMS );

private:
	void initFileState( );

	std::ofstream m_file;
};


#endif // _profilerlogwriter_hpp_
