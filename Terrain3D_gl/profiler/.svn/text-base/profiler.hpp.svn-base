/*************************************************************************************

Simple Runtime Profiler

Author: Marc Treib

(c) Marc Treib

mailto:treib@in.tum.de

Last Changed: 2010-04-27

*************************************************************************************/

#ifndef _profiler_hpp_
#define _profiler_hpp_


#include <set>
#include <string>

typedef unsigned int uint;
typedef unsigned __int64 uint64;


class IProfilerOutputHandler {
public:
	virtual void started( float timestampMS ) = 0;
	virtual void stopped( float timestampMS, float profiledTimeMS, uint profiledFrames ) = 0;

	virtual void beginOutput( float timestampMS, float profiledTimeMS, uint profiledFrames ) = 0;
	virtual void putSample( const std::string& name, uint parentCount, bool hasUncountedChildren, float totalTimeMS, float maxCallTimeMS, float maxFrameTimeMS, float ownTimeMS, uint totalCount, uint maxFrameCount ) = 0;
	virtual void endOutput( ) = 0;

	virtual void cleared( float timestampMS ) = 0;
};

//TODO accumulated stats per sample name
class Profiler {
public:
	static const uint MAX_PROFILER_SAMPLES = 100;

	Profiler( );

	bool addOutputHandler( IProfilerOutputHandler* handler );
	bool removeOutputHandler( IProfilerOutputHandler* handler );

	void setMaxSampleDepth( int maxDepth ) { m_maxSampleDepth = maxDepth; }

	void start( );
	void stop( );
	bool isRunning( ) const { return m_running; }

	void startSample( const std::string& sampleName );
	void endSample( );

	void endFrame( );

	void output( ) const;

	void clear( );

protected:
	void output( int parentIndex ) const;

	void beginOutput( float timestampMS, float profiledTimeMS ) const;
	void putSample( const std::string& name, uint parentCount, bool hasUncountedChildren, float totalTimeMS, float maxCallTimeMS, float maxFrameTimeMS, float ownTimeMS, uint totalCount, uint maxFrameCount ) const;
	void endOutput( ) const;

	struct sample {
		sample( ) : valid( false ) { }

		bool        valid;					// sample valid?
		std::string name;					// name of the sample
		int         parentIndex;			// index of the parent sample
		uint        parentCount;			// number of parents this sample has (useful for indenting)

		bool        hasUncountedChildren;	// true if there are child samples that have not been counted separately (because max sample depth was reaches)

		uint64      totalTime;				// total time recorded across all profiles of this sample
		uint64      maxCallTime;			// maximum time a single instance of the sample has taken
		uint64      childTime;				// total time taken by children of this sample
		uint64      thisFrameTime;			// time recorded across all profiles of this sample during the current frame
		uint64      maxFrameTime;			// maximum time recorded in a single frame

		uint        totalCount;				// total number of times this sample has been profiled
		uint        thisFrameCount;			// number of times this sample has been profiled during the current frame
		uint        maxFrameCount;			// maximum number fo times this sample has been profiled in a single frame

		uint        openCount;				// how often this sample is currently opened (recursively)
		uint64      startTime;				// starting time on the clock
	} m_samples[ MAX_PROFILER_SAMPLES ];

	typedef std::set< IProfilerOutputHandler* > OutputHandlerSet;

	OutputHandlerSet m_outputHandlers;

	int              m_maxSampleDepth;

	int              m_openSampleIndex;
	int              m_openSampleUncountedChildrenCount;
	int              m_openSampleCount;
	int              m_droppedSampleCount;

	uint64           m_globalStartTime;

	uint64           m_totalProfiledTime;
	uint             m_totalProfiledFrames;

	bool             m_running;
	uint64           m_startTime;
	mutable uint64   m_endTime;

	uint64           m_perfFreq;
};

class ScopedProfileSample {
public:
	ScopedProfileSample( Profiler* pProfiler, const std::string& sampleName ) : m_pProfiler( pProfiler ) { if( m_pProfiler ) m_pProfiler->startSample( sampleName ); }
	~ScopedProfileSample( ) { if( m_pProfiler ) m_pProfiler->endSample( ); }

private:
	Profiler* m_pProfiler;
};


#endif // _profiler_hpp_
