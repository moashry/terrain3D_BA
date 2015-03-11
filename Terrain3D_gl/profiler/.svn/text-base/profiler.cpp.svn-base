/*************************************************************************************

Simple Runtime Profiler

Author: Marc Treib

(c) Marc Treib

mailto:treib@in.tum.de

Last Changed: 2010-04-27

*************************************************************************************/

#include "profiler.hpp"

#include <cassert>
#include <limits>

#include <windows.h>


namespace {
	uint64 getTime( ) {
		LARGE_INTEGER time;
		QueryPerformanceCounter( &time );
		return time.QuadPart;
	}
}

Profiler::Profiler( )
	: m_maxSampleDepth( 8 ),
	  m_openSampleIndex( -1 ), m_openSampleUncountedChildrenCount( 0 ), m_openSampleCount( 0 ), m_droppedSampleCount( 0 ),
	  m_totalProfiledTime( 0 ), m_totalProfiledFrames( 0 ),
	  m_running( false ), m_startTime( 0 ), m_endTime( 0 )
{
	LARGE_INTEGER perfFreq;
	QueryPerformanceFrequency(&perfFreq);
	m_perfFreq = perfFreq.QuadPart;

	m_globalStartTime = getTime( );
}

bool Profiler::addOutputHandler( IProfilerOutputHandler* handler ) {
	std::pair< std::set< IProfilerOutputHandler* >::iterator, bool > inserted = m_outputHandlers.insert( handler );
	return inserted.second;
}

bool Profiler::removeOutputHandler( IProfilerOutputHandler* handler ) {
	return m_outputHandlers.erase( handler ) != 0;
}

void Profiler::start( ) {
	assert( !m_running );

	// update total profiled time before resetting start time
	m_totalProfiledTime += m_endTime - m_startTime;

	// start
	m_startTime = getTime( );
	m_running = true;

	// notify output handlers
	float timestampMS = 1000.0f * float( m_startTime - m_globalStartTime ) / float( m_perfFreq );
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->started( timestampMS );
	}
}

void Profiler::stop( ) {
	assert( m_running );

	// stop
	m_endTime = getTime( );
	m_running = false;

	// notify output handlers
	float timestampMS = 1000.0f * float( m_endTime - m_globalStartTime ) / float( m_perfFreq );
	uint64 profiledTime = m_endTime - m_startTime;
	float profiledTimeMS = 1000.0f * float( m_totalProfiledTime + profiledTime ) / float( m_perfFreq );
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->stopped( timestampMS, profiledTimeMS, m_totalProfiledFrames );
	}
}

void Profiler::startSample( const std::string& sampleName ) {
	// ignore if we're not running
	if( !m_running )
		return;

	// check for simple (direct) recursion
	if( m_openSampleIndex >= 0 && m_samples[ m_openSampleIndex ].name == sampleName ) {
		m_samples[ m_openSampleIndex ].openCount++;
		m_samples[ m_openSampleIndex ].totalCount++;
		m_samples[ m_openSampleIndex ].thisFrameCount++;
		if( m_samples[ m_openSampleIndex ].thisFrameCount > m_samples[ m_openSampleIndex ].maxFrameCount )
			m_samples[ m_openSampleIndex ].maxFrameCount = m_samples[ m_openSampleIndex ].thisFrameCount;
		return;
	}

	// check if maximum sample depth was reached
	if( m_openSampleCount >= m_maxSampleDepth ) {
		m_openSampleUncountedChildrenCount++;
		m_samples[ m_openSampleIndex ].hasUncountedChildren = true;
		return;
	}

	// find the sample
	int i = 0;
	int storeIndex = -1;
	for( i = 0; i < MAX_PROFILER_SAMPLES; i++ ) {
		if( !m_samples[ i ].valid ) {
			if( storeIndex < 0 ) {
				storeIndex = i;
			}
		} else {
			if( m_samples[ i ].parentIndex == m_openSampleIndex && m_samples[ i ].name == sampleName ) {
				// this is the sample we want
				assert( m_samples[ i ].openCount == 0 && "Tried to profile a sample which was already being profiled" );

				m_samples[ i ].totalCount++;
				m_samples[ i ].thisFrameCount++;
				if( m_samples[ i ].thisFrameCount > m_samples[ i ].maxFrameCount )
					m_samples[ i ].maxFrameCount = m_samples[ i ].thisFrameCount;

				m_samples[ i ].openCount++;
				m_samples[ i ].startTime = getTime( );

				m_openSampleIndex = i;
				m_openSampleCount++;

				return;
			}
		}
	}

	// we haven't found it, so it must be a new sample
	assert( storeIndex >= 0 && "Profiler has run out of sample slots!" );
	// handle case when we run out of slots
	if( storeIndex < 0 ) {
		m_droppedSampleCount++;
		if( m_openSampleIndex >= 0 ) {
			m_samples[ m_openSampleIndex ].hasUncountedChildren = true;
		}
		return;
	}

	// init the new sample and start profiling it
	m_samples[ storeIndex ].valid = true;
	m_samples[ storeIndex ].name = sampleName;
	m_samples[ storeIndex ].parentIndex = m_openSampleIndex;
	m_samples[ storeIndex ].parentCount = m_openSampleCount;

	m_samples[ storeIndex ].hasUncountedChildren = false;

	m_samples[ storeIndex ].totalTime = 0;
	m_samples[ storeIndex ].maxCallTime = 0;
	m_samples[ storeIndex ].childTime = 0;
	m_samples[ storeIndex ].thisFrameTime = 0;
	m_samples[ storeIndex ].maxFrameTime = 0;

	m_samples[ storeIndex ].totalCount = 1;
	m_samples[ storeIndex ].thisFrameCount = 1;
	m_samples[ storeIndex ].maxFrameCount = 1;

	m_samples[ storeIndex ].openCount = 1;
	m_samples[ storeIndex ].startTime = getTime( );

	m_openSampleIndex = storeIndex;
	m_openSampleCount++;
}

void Profiler::endSample( ) {
	// ignore if we're not running
	if( !m_running )
		return;

	assert( m_openSampleIndex >= 0 );
	assert( m_openSampleCount > 0 );
	assert( m_samples[ m_openSampleIndex ].openCount > 0 );

	// handle case when we have run out of slots
	if( m_droppedSampleCount > 0 ) {
		m_droppedSampleCount--;
		return;
	}

	// if the sample has uncounted children, nothing to do here
	if( m_openSampleUncountedChildrenCount > 0 ) {
		m_openSampleUncountedChildrenCount--;
		return;
	}

	// if sample is opened multiple times, also nothing to do
	if( m_samples[ m_openSampleIndex ].openCount > 1 ) {
		m_samples[ m_openSampleIndex ].openCount--;
		return;
	}

	uint64 endTime = getTime( );
	// ok, we're done timing
	m_samples[ m_openSampleIndex ].openCount = 0;
	// calculate the time taken this profile
	uint64 timeTaken = endTime - m_samples[ m_openSampleIndex ].startTime;

	// update this sample
	m_samples[ m_openSampleIndex ].totalTime += timeTaken;
	if( timeTaken > m_samples[ m_openSampleIndex ].maxCallTime )
		m_samples[ m_openSampleIndex ].maxCallTime = timeTaken;
	m_samples[ m_openSampleIndex ].thisFrameTime += timeTaken;
	if( m_samples[ m_openSampleIndex ].thisFrameTime > m_samples[ m_openSampleIndex ].maxFrameTime )
		m_samples[ m_openSampleIndex ].maxFrameTime = m_samples[ m_openSampleIndex ].thisFrameTime;

	// update parent sample
	if( m_samples[ m_openSampleIndex ].parentIndex >= 0 )
		m_samples[ m_samples[ m_openSampleIndex ].parentIndex ].childTime += timeTaken;

	// now, the parent sample is the open sample
	m_openSampleIndex = m_samples[ m_openSampleIndex ].parentIndex;
	m_openSampleCount--;
}

void Profiler::endFrame( ) {
	// ignore if we're not running
	if( !m_running )
		return;

	assert( m_openSampleIndex == -1 );
	assert( m_openSampleCount == 0 );

	++m_totalProfiledFrames;

	// reset per-frame stats of all samples
	for( uint i = 0; i < MAX_PROFILER_SAMPLES; i++ ) {
		m_samples[ i ].thisFrameTime = 0;
		m_samples[ i ].thisFrameCount = 0;
	}
}

void Profiler::output( ) const {
	// there shouldn't be any open samples
	assert( m_openSampleIndex == -1 );
	assert( m_openSampleCount == 0 );

	uint64 curTime = getTime( );

	if( m_running ) {
		m_endTime = curTime;
	}

	float timestampMS = 1000.0f * float( curTime - m_globalStartTime ) / float( m_perfFreq );
	uint64 profiledTime = m_endTime - m_startTime;
	float profiledTimeMS = 1000.0f * float( m_totalProfiledTime + profiledTime ) / float( m_perfFreq );

	beginOutput( timestampMS, profiledTimeMS );
	output( -1 );
	endOutput( );
}

void Profiler::clear( ) {
	// there shouldn't be any open samples
	assert( m_openSampleIndex == -1 );
	assert( m_openSampleCount == 0 );

	for( uint i = 0; i < MAX_PROFILER_SAMPLES; i++ ) {
		m_samples[ i ].valid = false;
	}

	m_startTime = m_endTime;
	m_totalProfiledTime = 0;
	m_totalProfiledFrames = 0;

	// notify output handlers
	float timestampMS = 1000.0f * float( m_startTime - m_globalStartTime ) / float( m_perfFreq );
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->cleared( timestampMS );
	}
}

void Profiler::output( int parentIndex ) const {
	uint64 totalTime = m_endTime - m_startTime;

	for( uint i = 0; i < MAX_PROFILER_SAMPLES; i++ ) {
		if( m_samples[ i ].valid && m_samples[ i ].parentIndex == parentIndex ) {
			// calculate the time spent on the sample itself (excluding children)
			float sampleTotalTimeMS = 1000.0f * float( m_samples[ i ].totalTime ) / float( m_perfFreq );
			uint64 sampleOwnTime = m_samples[ i ].totalTime - m_samples[ i ].childTime;
			float sampleOwnTimeMS      = 1000.0f * float( sampleOwnTime )               / float( m_perfFreq );
			float sampleMaxCallTimeMS  = 1000.0f * float( m_samples[ i ].maxCallTime )  / float( m_perfFreq );
			float sampleMaxFrameTimeMS = 1000.0f * float( m_samples[ i ].maxFrameTime ) / float( m_perfFreq );

			// output these values
			putSample( m_samples[ i ].name, m_samples[ i ].parentCount, m_samples[ i ].hasUncountedChildren, sampleTotalTimeMS, sampleMaxCallTimeMS, sampleMaxFrameTimeMS, sampleOwnTimeMS, m_samples[ i ].totalCount, m_samples[ i ].maxFrameCount );

			// recurse on children
			output( i );
		}
	}
}

void Profiler::beginOutput( float timestampMS, float profiledTimeMS ) const {
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->beginOutput( timestampMS, profiledTimeMS, m_totalProfiledFrames );
	}
}

void Profiler::putSample( const std::string& name, uint parentCount, bool hasUncountedChildren, float totalTimeMS, float maxCallTimeMS, float maxFrameTimeMS, float ownTimeMS, uint totalCount, uint maxFrameCount ) const {
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->putSample( name, parentCount, hasUncountedChildren, totalTimeMS, maxCallTimeMS, maxFrameTimeMS, ownTimeMS, totalCount, maxFrameCount );
	}
}

void Profiler::endOutput( ) const {
	std::set< IProfilerOutputHandler* >::const_iterator it, itEnd = m_outputHandlers.end( );
	for( it = m_outputHandlers.begin( ); it != itEnd; it++ ) {
		( *it )->endOutput( );
	}
}
